#define OLD_TAR

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../../util/fd_util.h"
#ifdef OLD_TAR
#include "../../util/archive/fd_tar_old.h"
#else
#include "../../util/archive/fd_tar.h"
#endif /* OLD_TAR */
#include "../../util/compress/fd_compress.h"
#include "../../flamenco/fd_flamenco.h"
#include "../../flamenco/nanopb/pb_decode.h"
#include "../../flamenco/runtime/fd_banks_solana.h"
#include "../../flamenco/runtime/fd_hashes.h"
#include "../../funk/fd_funk.h"
#include "../../flamenco/types/fd_types.h"
#include "../../flamenco/runtime/fd_runtime.h"
#include "../../flamenco/runtime/fd_account.h"
#include "../../flamenco/runtime/fd_rocksdb.h"
#include "../../ballet/base58/fd_base58.h"
#include "../../flamenco/types/fd_solana_block.pb.h"
#include "../../flamenco/runtime/context/fd_capture_ctx.h"
#include "../../flamenco/runtime/fd_snapshot_loader.h"
#include "../../flamenco/runtime/fd_blockstore.h"

extern void fd_write_builtin_bogus_account( fd_exec_slot_ctx_t * slot_ctx, uchar const       pubkey[ static 32 ], char const *      data, ulong             sz );

static void usage(char const * progname) {
  fprintf(stderr, "USAGE: %s\n", progname);
  fprintf(stderr, " --cmd ingest --snapshotfile <file>               ingest solana snapshot file\n");
  fprintf(stderr, "              --incremental <file>                also ingest incremental snapshot file\n");
  fprintf(stderr, "              --rocksdb <file>                    also ingest a rocks database file\n");
  fprintf(stderr, "                --txnstatus true                    also ingest transaction status from rocksdb\n");
  fprintf(stderr, "              --genesis <file>                    also ingest a genesis file\n");
  fprintf(stderr, " --wksp <name>                                    workspace name\n");
  fprintf(stderr, " --reset true                                     reset workspace before ingesting\n");
  fprintf(stderr, " --backup <file>                                  make a funky backup file\n");
  fprintf(stderr, " --indexmax <count>                               size of funky account map\n");
  fprintf(stderr, " --txnmax <count>                                 size of funky transaction map\n");
  fprintf(stderr, " --verifyhash <base58hash>                        verify that the accounts hash matches the given one\n");
  fprintf(stderr, " --verifyfunky true                               verify database integrity\n");
  fprintf(stderr, " --verifypoh true                                 verify proof-of-history while importing blocks\n");
  fprintf(stderr, " --loglevel <level>                               Set logging level\n");
  fprintf(stderr, " --network <net>                                  main/dev/testnet\n");
}

void
ingest_rocksdb( fd_exec_slot_ctx_t * slot_ctx,
                char const *      file,
                ulong             end_slot,
                fd_blockstore_t * blockstore ) {
  fd_rocksdb_t rocks_db;
  char *err = fd_rocksdb_init(&rocks_db, file);
  if (err != NULL) {
    FD_LOG_ERR(("fd_rocksdb_init returned %s", err));
  }

  ulong last_slot = fd_rocksdb_last_slot(&rocks_db, &err);
  if (err != NULL) {
    FD_LOG_ERR(("fd_rocksdb_last_slot returned %s", err));
  }
  if (end_slot > last_slot)
    end_slot = last_slot;

  ulong start_slot = slot_ctx->slot_bank.slot;
  if ( last_slot < start_slot ) {
    FD_LOG_ERR(("rocksdb blocks are older than snapshot. first=%lu last=%lu wanted=%lu",
                fd_rocksdb_first_slot(&rocks_db, &err), last_slot, start_slot));
  }

  FD_LOG_NOTICE(("ingesting rocksdb from start=%lu to end=%lu", start_slot, end_slot));

  fd_rocksdb_root_iter_t iter;
  fd_rocksdb_root_iter_new ( &iter );

  fd_slot_meta_t m;
  fd_memset(&m, 0, sizeof(m));

  int ret = fd_rocksdb_root_iter_seek( &iter, &rocks_db, start_slot, &m, slot_ctx->valloc );
  if (ret < 0)
    FD_LOG_ERR(("fd_rocksdb_root_iter_seek returned %d", ret));

  ulong blk_cnt = 0;
  do {
    ulong slot = m.slot;
    if (slot >= end_slot)
      break;

    /* Read and deshred block from RocksDB */

    int err = fd_rocksdb_import_block(&rocks_db, &m, blockstore);
    if( FD_UNLIKELY( err ) ) FD_LOG_ERR(( "fd_rocksdb_get_block failed" ));

    ++blk_cnt;

    fd_bincode_destroy_ctx_t ctx = { .valloc = slot_ctx->valloc };
    fd_slot_meta_destroy(&m, &ctx);

    ret = fd_rocksdb_root_iter_next( &iter, &m, slot_ctx->valloc );
    if (ret < 0)
      FD_LOG_ERR(("fd_rocksdb_root_iter_seek returned %d", ret));
  } while (1);

  fd_rocksdb_root_iter_destroy( &iter );
  fd_rocksdb_destroy(&rocks_db);

  FD_LOG_NOTICE(("ingested %lu blocks", blk_cnt));
}

int
main( int     argc,
      char ** argv ) {

  if( FD_UNLIKELY( argc==1 ) ) {
    usage( argv[0] );
    return 1;
  }

  fd_boot( &argc, &argv );
  fd_flamenco_boot( &argc, &argv );

  char const * wkspname     = fd_env_strip_cmdline_cstr ( &argc, &argv, "--wksp",         NULL, NULL      );
  ulong        pages        = fd_env_strip_cmdline_ulong( &argc, &argv, "--pages",        NULL,         5 );
  char const * reset        = fd_env_strip_cmdline_cstr ( &argc, &argv, "--reset",        NULL, "false"   );
  char const * cmd          = fd_env_strip_cmdline_cstr ( &argc, &argv, "--cmd",          NULL, NULL      );
  ulong        index_max    = fd_env_strip_cmdline_ulong( &argc, &argv, "--indexmax",     NULL, 350000000 );
  ulong        xactions_max = fd_env_strip_cmdline_ulong( &argc, &argv, "--txnmax",       NULL,      1000 );
  char const * verifyfunky  = fd_env_strip_cmdline_cstr ( &argc, &argv, "--verifyfunky",  NULL, "false"   );
  char const * snapshotfile = fd_env_strip_cmdline_cstr ( &argc, &argv, "--snapshotfile", NULL, NULL      );
  char const * incremental  = fd_env_strip_cmdline_cstr ( &argc, &argv, "--incremental",  NULL, NULL      );
  char const * genesis      = fd_env_strip_cmdline_cstr ( &argc, &argv, "--genesis",      NULL, NULL      );
  char const * rocksdb_dir  = fd_env_strip_cmdline_cstr ( &argc, &argv, "--rocksdb",      NULL, NULL      );
  ulong        end_slot     = fd_env_strip_cmdline_ulong( &argc, &argv, "--endslot",      NULL, ULONG_MAX );
  char const * verifyhash   = fd_env_strip_cmdline_cstr ( &argc, &argv, "--verifyhash",   NULL, NULL      );
  char const * backup       = fd_env_strip_cmdline_cstr ( &argc, &argv, "--backup",       NULL, NULL      );
  char const * capture_fpath = fd_env_strip_cmdline_cstr ( &argc, &argv, "--capture",      NULL, NULL      );
#ifdef _ENABLE_RHASH
  char const * rhash        = fd_env_strip_cmdline_cstr ( &argc, &argv, "--rhash",        NULL, "false"   );
#endif

  fd_wksp_t* wksp;
  if (wkspname == NULL) {
    FD_LOG_NOTICE(( "--wksp not specified, using an anonymous local workspace" ));
    wksp = fd_wksp_new_anonymous( FD_SHMEM_GIGANTIC_PAGE_SZ, pages, 0, "wksp", 0UL );
  } else {
    fd_shmem_info_t shmem_info[1];
    if ( FD_UNLIKELY( fd_shmem_info( wkspname, 0UL, shmem_info ) ) )
      FD_LOG_ERR(( "unable to query region \"%s\"\n\tprobably does not exist or bad permissions", wkspname ));
    wksp = fd_wksp_attach(wkspname);
  }
  if (wksp == NULL)
    FD_LOG_ERR(( "failed to attach to workspace %s", wkspname ));

  char hostname[64];
  gethostname(hostname, sizeof(hostname));
  ulong hashseed = fd_hash(0, hostname, strnlen(hostname, sizeof(hostname)));

  if( strcmp(reset, "true") == 0 ) {
    fd_wksp_reset( wksp, (uint)hashseed);
  }

  /* Create scratch region */
  ulong  smax   = 512 /*MiB*/ << 20;
  ulong  sdepth = 128;      /* 128 scratch frames */
  void * smem   = fd_wksp_alloc_laddr( wksp, fd_scratch_smem_align(), fd_scratch_smem_footprint( smax   ), 421UL );
  void * fmem   = fd_wksp_alloc_laddr( wksp, fd_scratch_fmem_align(), fd_scratch_fmem_footprint( sdepth ), 421UL );
  FD_TEST( (!!smem) & (!!fmem) );
  fd_scratch_attach( smem, fmem, smax, sdepth );

  fd_funk_t* funk;

  if( FD_UNLIKELY( !cmd ) ) FD_LOG_ERR(( "no command specified" ));

  void* shmem;
  fd_wksp_tag_query_info_t info;
  ulong tag = FD_FUNK_MAGIC;
  if (fd_wksp_tag_query(wksp, &tag, 1, &info, 1) > 0) {
    shmem = fd_wksp_laddr_fast( wksp, info.gaddr_lo );
    funk = fd_funk_join(shmem);
    if (funk == NULL)
      FD_LOG_ERR(( "failed to join a funky" ));
    if (strcmp(verifyfunky, "true") == 0)
      if (fd_funk_verify(funk))
        FD_LOG_ERR(( "verification failed" ));
  } else {
    shmem = fd_wksp_alloc_laddr( wksp, fd_funk_align(), fd_funk_footprint(), FD_FUNK_MAGIC );
    if (shmem == NULL)
      FD_LOG_ERR(( "failed to allocate a funky" ));
    funk = fd_funk_join(fd_funk_new(shmem, 1, hashseed, xactions_max, index_max));
    if (funk == NULL) {
      fd_wksp_free_laddr(shmem);
      FD_LOG_ERR(( "failed to allocate a funky" ));
    }
  }

  FD_LOG_NOTICE(( "funky at global address 0x%016lx", fd_wksp_gaddr_fast( wksp, shmem ) ));

  fd_blockstore_t* blockstore;

  tag = FD_BLOCKSTORE_MAGIC;
  if (fd_wksp_tag_query(wksp, &tag, 1, &info, 1) > 0) {
    shmem = fd_wksp_laddr_fast( wksp, info.gaddr_lo );
    blockstore = fd_blockstore_join(shmem);
    if (blockstore == NULL)
      FD_LOG_ERR(( "failed to join a blockstore" ));
  } else {
    shmem = fd_wksp_alloc_laddr( wksp, fd_blockstore_align(), fd_blockstore_footprint(), FD_BLOCKSTORE_MAGIC );
    if (shmem == NULL)
      FD_LOG_ERR(( "failed to allocate a blockstore" ));
    ulong tmp_shred_max = 1UL << 20;
    int   lg_slot_max   = 14;
    int   lg_txn_max    = 20;
    blockstore = fd_blockstore_join(fd_blockstore_new(shmem, 1, hashseed, tmp_shred_max, lg_slot_max, lg_txn_max));
    if (blockstore == NULL) {
      fd_wksp_free_laddr(shmem);
      FD_LOG_ERR(( "failed to allocate a blockstore" ));
    }
  }

  FD_LOG_NOTICE(( "blockstore at global address 0x%016lx", fd_wksp_gaddr_fast( wksp, shmem ) ));

  uchar epoch_ctx_mem[FD_EXEC_EPOCH_CTX_FOOTPRINT] __attribute__((aligned(FD_EXEC_EPOCH_CTX_ALIGN)));
  fd_exec_epoch_ctx_t * epoch_ctx = fd_exec_epoch_ctx_join( fd_exec_epoch_ctx_new( epoch_ctx_mem ) );

  uchar slot_ctx_mem[FD_EXEC_SLOT_CTX_FOOTPRINT] __attribute__((aligned(FD_EXEC_SLOT_CTX_ALIGN)));
  fd_exec_slot_ctx_t * slot_ctx = fd_exec_slot_ctx_join( fd_exec_slot_ctx_new( slot_ctx_mem ) );
  slot_ctx->epoch_ctx = epoch_ctx;

  fd_alloc_t * alloc = fd_alloc_join( fd_wksp_laddr_fast( wksp, funk->alloc_gaddr ), 0UL );
  if( FD_UNLIKELY( !alloc ) ) FD_LOG_ERR(( "fd_alloc_join(gaddr=%#lx) failed", funk->alloc_gaddr ));
  /* TODO leave */

  epoch_ctx->valloc = fd_alloc_virtual( alloc );
  slot_ctx->valloc = fd_alloc_virtual( alloc );

  fd_acc_mgr_t mgr[1];
  slot_ctx->acc_mgr = fd_acc_mgr_new( mgr, funk );

  ulong tcnt = fd_tile_cnt();
  uchar tpool_mem[ FD_TPOOL_FOOTPRINT(FD_TILE_MAX) ] __attribute__((aligned(FD_TPOOL_ALIGN)));
  fd_tpool_t * tpool = NULL;
  if ( tcnt > 1) {
    tpool = fd_tpool_init(tpool_mem, tcnt);
    if ( tpool == NULL )
      FD_LOG_ERR(("failed to create thread pool"));
    for ( ulong i = 1; i <= tcnt-1; ++i ) {
      if ( fd_tpool_worker_push( tpool, i, NULL, 0UL ) == NULL )
        FD_LOG_ERR(("failed to launch worker"));
    }
  }

  if (cmd == NULL) {
    // Do nothing

  } else if (strcmp(cmd, "ingest") == 0) {
#ifdef _ENABLE_RHASH
    if ((NULL != rhash) && (strcmp(rhash, "true") == 0)) {
      fd_accounts_init_rhash(slot_ctx);
      fd_accounts_check_rhash(slot_ctx);
    }
#endif

    if( snapshotfile ) {
      const char * snapshotfiles[3];
      snapshotfiles[0] = snapshotfile;
      snapshotfiles[1] = incremental;
      snapshotfiles[2] = NULL;
      fd_snapshot_load(snapshotfiles, slot_ctx);
    }

    if( genesis ) {

      FILE *               capture_file = NULL;
      fd_capture_ctx_t *   capture_ctx  = NULL;
      if( capture_fpath ) {
        capture_file = fopen( capture_fpath, "w+" );
        if( FD_UNLIKELY( !capture_file ) )
          FD_LOG_ERR(( "fopen(%s) failed (%d-%s)", capture_fpath, errno, strerror( errno ) ));

        void * capture_ctx_mem = fd_alloc_malloc( alloc, FD_CAPTURE_CTX_ALIGN, FD_CAPTURE_CTX_FOOTPRINT );
        FD_TEST( capture_ctx_mem );
        capture_ctx = fd_capture_ctx_new( capture_ctx_mem );

        // FD_TEST( fd_solcap_writer_init( capture_ctx->capture, capture_file ) );
      }

      // fd_solcap_writer_set_slot( capture_ctx->capture, 0UL );

      struct stat sbuf;
      if( FD_UNLIKELY( stat( genesis, &sbuf) < 0 ) )
        FD_LOG_ERR(("cannot open %s : %s", genesis, strerror(errno)));
      int fd = open( genesis, O_RDONLY );
      if( FD_UNLIKELY( fd < 0 ) )
        FD_LOG_ERR(("cannot open %s : %s", genesis, strerror(errno)));
      uchar * buf = malloc((ulong) sbuf.st_size);  /* TODO Make this a scratch alloc */
      ssize_t n = read(fd, buf, (ulong) sbuf.st_size);
      close(fd);

      fd_genesis_solana_t genesis_block;
      fd_genesis_solana_new(&genesis_block);
      fd_bincode_decode_ctx_t ctx = {
        .data = buf,
        .dataend = buf + n,
        .valloc  = slot_ctx->valloc
      };
      if( fd_genesis_solana_decode(&genesis_block, &ctx) )
        FD_LOG_ERR(("fd_genesis_solana_decode failed"));

      // The hash is generated from the raw data... don't mess with this..
      fd_hash_t genesis_hash;
      fd_sha256_hash( buf, (ulong)n, genesis_hash.uc );
      FD_LOG_NOTICE(( "Genesis Hash: %32J", &genesis_hash ));

      free(buf);

      fd_runtime_init_bank_from_genesis( slot_ctx, &genesis_block, &genesis_hash );

      fd_runtime_init_program( slot_ctx );

      FD_LOG_DEBUG(( "start genesis accounts - count: %lu", genesis_block.accounts_len));

      for( ulong i=0; i < genesis_block.accounts_len; i++ ) {
        fd_pubkey_account_pair_t * a = &genesis_block.accounts[i];

        FD_BORROWED_ACCOUNT_DECL(rec);

        int err = fd_acc_mgr_modify(
            slot_ctx->acc_mgr,
            slot_ctx->funk_txn,
            &a->key,
            /* do_create */ 1,
            a->account.data_len,
            rec);
        if( FD_UNLIKELY( err ) )
          FD_LOG_ERR(( "fd_acc_mgr_modify failed (%d)", err ));

        rec->meta->dlen            = a->account.data_len;
        rec->meta->info.lamports   = a->account.lamports;
        rec->meta->info.rent_epoch = a->account.rent_epoch;
        rec->meta->info.executable = (char)a->account.executable;
        memcpy( rec->meta->info.owner, a->account.owner.key, 32UL );
        if( a->account.data_len )
          memcpy( rec->data, a->account.data, a->account.data_len );

        err = fd_acc_mgr_commit_raw( slot_ctx->acc_mgr, rec->rec, &a->key, rec->meta, slot_ctx );
        if( FD_UNLIKELY( err ) )
          FD_LOG_ERR(( "fd_acc_mgr_commit_raw failed (%d)", err ));
      }

      FD_LOG_DEBUG(( "end genesis accounts"));

      FD_LOG_DEBUG(( "native instruction processors - count: %lu", genesis_block.native_instruction_processors_len));

      for( ulong i=0; i < genesis_block.native_instruction_processors_len; i++ ) {
        fd_string_pubkey_pair_t * a = &genesis_block.native_instruction_processors[i];
        fd_write_builtin_bogus_account( slot_ctx, a->pubkey.uc, a->string, strlen(a->string) );
      }

      /* sort and update bank hash */
      int result = fd_update_hash_bank( slot_ctx, capture_ctx, &slot_ctx->slot_bank.banks_hash, slot_ctx->signature_cnt );
      if (result != FD_EXECUTOR_INSTR_SUCCESS) {
        return result;
      }

      slot_ctx->slot_bank.slot = 0UL;

      FD_TEST( FD_RUNTIME_EXECUTE_SUCCESS == fd_runtime_save_epoch_bank( slot_ctx ) );

      FD_TEST( FD_RUNTIME_EXECUTE_SUCCESS == fd_runtime_save_slot_bank( slot_ctx ) );

      fd_bincode_destroy_ctx_t ctx2 = { .valloc = slot_ctx->valloc };
      fd_genesis_solana_destroy(&genesis_block, &ctx2);

      if( capture_ctx )  {
        fd_solcap_writer_fini( capture_ctx->capture );
        fclose( capture_file );
      }
    }

    if( rocksdb_dir ) {
      ingest_rocksdb( slot_ctx, rocksdb_dir, end_slot, blockstore );

      fd_hash_t const * known_bank_hash = fd_get_bank_hash( slot_ctx->acc_mgr->funk, slot_ctx->slot_bank.slot );

      if( known_bank_hash ) {
        if( FD_UNLIKELY( 0!=memcmp( slot_ctx->slot_bank.banks_hash.hash, known_bank_hash->hash, 32UL ) ) ) {
          FD_LOG_ERR(( "Bank hash mismatch! slot=%lu expected=%32J, got=%32J",
              slot_ctx->slot_bank.slot,
              known_bank_hash->hash,
              slot_ctx->slot_bank.banks_hash.hash ));
        }
      }
    }

    /* Dump feature activation state */

    for( fd_feature_id_t const * id = fd_feature_iter_init();
                                     !fd_feature_iter_done( id );
                                 id = fd_feature_iter_next( id ) ) {
      ulong activated_at = fd_features_query( &slot_ctx->epoch_ctx->features, id );
      if( activated_at )
        FD_LOG_DEBUG(( "feature %32J activated at slot %lu", id->id.key, activated_at ));
    }
  }

  if (strcmp(verifyfunky, "true") == 0) {
    FD_LOG_NOTICE(("verifying funky"));
    if (fd_funk_verify(funk))
      FD_LOG_ERR(( "verification failed" ));
  }

  if (verifyhash) {
    fd_funk_rec_t * rec_map  = fd_funk_rec_map( funk, wksp );
    ulong num_iter_accounts = fd_funk_rec_map_key_cnt( rec_map );

    FD_LOG_NOTICE(( "verifying hash for %lu accounts", num_iter_accounts ));

    ulong zero_accounts = 0;
    ulong num_pairs = 0;
    fd_pubkey_hash_pair_t * pairs = (fd_pubkey_hash_pair_t *) malloc(num_iter_accounts*sizeof(fd_pubkey_hash_pair_t));
    for( fd_funk_rec_map_iter_t iter = fd_funk_rec_map_iter_init( rec_map );
         !fd_funk_rec_map_iter_done( rec_map, iter );
         iter = fd_funk_rec_map_iter_next( rec_map, iter ) ) {
      fd_funk_rec_t * rec = fd_funk_rec_map_iter_ele( rec_map, iter );
      if ( !fd_acc_mgr_is_key( rec->pair.key ) )
        continue;

      if (num_pairs % 10000000 == 0) {
        FD_LOG_NOTICE(( "read %lu so far", num_pairs ));
      }

      fd_account_meta_t * metadata = (fd_account_meta_t *) fd_funk_val_const( rec, wksp );
      if ((metadata->magic != FD_ACCOUNT_META_MAGIC) || (metadata->hlen != sizeof(fd_account_meta_t))) {
        FD_LOG_ERR(("invalid magic on metadata"));
      }

      if ((metadata->info.lamports == 0) | ((metadata->info.executable & ~1) != 0)) {
        zero_accounts++;
        continue;
      }

      fd_hash_t acc_hash;
      if( fd_hash_account_v0(acc_hash.uc, NULL, metadata, rec->pair.key->uc, fd_account_get_data(metadata), metadata->slot)==NULL )
        FD_LOG_ERR(("error processing account hash"));

      if( memcmp(acc_hash.uc, metadata->hash, 32) != 0 ) {
        FD_LOG_ERR(("account hash mismatch - num_pairs: %lu, slot: %lu, acc: %32J, acc_hash: %32J, snap_hash: %32J", num_pairs, slot_ctx->slot_bank.slot, rec->pair.key->uc, acc_hash.uc, metadata->hash));
      }

      pairs[num_pairs].pubkey = (const fd_pubkey_t *)rec->pair.key->uc;
      pairs[num_pairs].hash = (const fd_hash_t *)metadata->hash;
      num_pairs++;
    }
    FD_LOG_NOTICE(("num_iter_accounts: %ld  zero_accounts: %lu", num_iter_accounts, zero_accounts));
  
    fd_hash_t accounts_hash;
    fd_hash_account_deltas(pairs, num_pairs, &accounts_hash, slot_ctx);

    free(pairs);

    char accounts_hash_58[FD_BASE58_ENCODED_32_SZ];
    fd_base58_encode_32((uchar const *)accounts_hash.hash, NULL, accounts_hash_58);

    FD_LOG_NOTICE(("hash result %s", accounts_hash_58));
    if (strcmp(verifyhash, accounts_hash_58) == 0)
      FD_LOG_NOTICE(("hash verified!"));
    else
      FD_LOG_ERR(("hash does not match!"));
  }

  if ( tpool )
    fd_tpool_fini( tpool );

  fd_scratch_detach( NULL );
  fd_wksp_free_laddr( smem );
  fd_wksp_free_laddr( fmem );

  if (backup) {
    /* Copy the entire workspace into a file in the most naive way */
    FD_LOG_NOTICE(("writing %s", backup));
    unlink(backup);
    int err = fd_wksp_checkpt(wksp, backup, 0666, 0, NULL);
    if (err)
      FD_LOG_ERR(("backup failed: error %d", err));
  }

  fd_exec_slot_ctx_delete( fd_exec_slot_ctx_leave( slot_ctx ) );
  fd_exec_epoch_ctx_delete( fd_exec_epoch_ctx_leave( epoch_ctx ) );
  fd_funk_leave( funk );
  fd_blockstore_leave( blockstore );

  fd_log_flush();
  fd_flamenco_halt();
  fd_halt();
  return 0;
}