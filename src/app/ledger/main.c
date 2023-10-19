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
#include "../../util/archive/fd_tar.h"
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

struct SnapshotParser {
  struct fd_tar_stream  tarreader_;
  char*                 tmpstart_;
  char*                 tmpcur_;
  char*                 tmpend_;

  fd_exec_slot_ctx_t *  slot_ctx_;

  fd_solana_manifest_t* manifest_;
};

void SnapshotParser_init(struct SnapshotParser* self, fd_exec_slot_ctx_t * slot_ctx) {
  fd_tar_stream_init( &self->tarreader_, slot_ctx->valloc );
  size_t tmpsize = 1<<30;
  self->tmpstart_ = self->tmpcur_ = (char*)malloc(tmpsize);
  self->tmpend_ = self->tmpstart_ + tmpsize;

  self->slot_ctx_ = slot_ctx;

  self->manifest_ = NULL;
}

void SnapshotParser_destroy(struct SnapshotParser* self) {
  if (self->manifest_) {
    fd_exec_slot_ctx_t * slot_ctx = self->slot_ctx_;
    fd_bincode_destroy_ctx_t ctx = { .valloc = slot_ctx->valloc };
    fd_solana_manifest_destroy(self->manifest_, &ctx);
    fd_valloc_free( slot_ctx->valloc, self->manifest_ );
    self->manifest_ = NULL;
  }

  fd_tar_stream_delete(&self->tarreader_);
  free(self->tmpstart_);
}

/* why is this creatively named "parse" if it actually loads the
   snapshot into the database? */

void SnapshotParser_parsefd_solana_accounts(struct SnapshotParser* self, char const * name, const void* data, size_t datalen) {
  ulong id, slot;
  if (sscanf(name, "accounts/%lu.%lu", &slot, &id) != 2)
    return;

  fd_slot_account_pair_t_mapnode_t key1;
  key1.elem.slot = slot;
  fd_slot_account_pair_t_mapnode_t* node1 = fd_slot_account_pair_t_map_find(
    self->manifest_->accounts_db.storages_pool, self->manifest_->accounts_db.storages_root, &key1);
  if (node1 == NULL)
    return;

  fd_serializable_account_storage_entry_t_mapnode_t key2;
  key2.elem.id = id;
  fd_serializable_account_storage_entry_t_mapnode_t* node2 = fd_serializable_account_storage_entry_t_map_find(
    node1->elem.accounts_pool, node1->elem.accounts_root, &key2);
  if (node2 == NULL)
    return;

  if (node2->elem.accounts_current_len < datalen)
    datalen = node2->elem.accounts_current_len;

  fd_acc_mgr_t *  acc_mgr = self->slot_ctx_->acc_mgr;
  fd_funk_txn_t * txn     = self->slot_ctx_->funk_txn;

  while (datalen) {
    size_t roundedlen = (sizeof(fd_solana_account_hdr_t)+7UL)&~7UL;
    if (roundedlen > datalen)
      return;

    fd_solana_account_hdr_t const * hdr = (fd_solana_account_hdr_t const *)data;
    uchar const * acc_data = (uchar const *)hdr + sizeof(fd_solana_account_hdr_t);

    fd_pubkey_t const * acc_key = (fd_pubkey_t const *)&hdr->meta.pubkey;

    do {
      /* Check existing account */
      FD_BORROWED_ACCOUNT_DECL(rec);

      int read_result = FD_ACC_MGR_SUCCESS;
      fd_account_meta_t const * acc_meta = fd_acc_mgr_view_raw( acc_mgr, txn, acc_key, &rec->const_rec, &read_result);
      
      /* Skip if we previously inserted a newer version */
      if( read_result == FD_ACC_MGR_SUCCESS ) {
        if( acc_meta->slot > slot ) break;
      } else if( FD_UNLIKELY( read_result != FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT ) ) {
        FD_LOG_ERR(( "database error while loading snapshot: %d", read_result ));
      }

      if( FD_UNLIKELY( hdr->meta.data_len > MAX_ACC_SIZE ) )
        FD_LOG_ERR(("account too large: %lu bytes", hdr->meta.data_len));

      /* Write account */
      int write_result = fd_acc_mgr_modify(acc_mgr, txn, acc_key, /* do_create */ 1, hdr->meta.data_len, rec);
      if( FD_UNLIKELY( write_result != FD_ACC_MGR_SUCCESS ) )
        FD_LOG_ERR(("writing account failed"));

      rec->meta->dlen = hdr->meta.data_len;
      rec->meta->slot = slot;
      memcpy( &rec->meta->hash, hdr->hash.value, 32UL );
      memcpy( &rec->meta->info, &hdr->info, sizeof(fd_solana_account_meta_t) );
      if( hdr->meta.data_len )
        memcpy( rec->data, acc_data, hdr->meta.data_len );

      /* TODO Check if calculated hash fails to match account hash from snapshot. */
    } while (0);

    roundedlen = (sizeof(fd_solana_account_hdr_t)+hdr->meta.data_len+7UL)&~7UL;
    if (roundedlen > datalen)
      return;
    data = (char const *)data + roundedlen;
    datalen -= roundedlen;
  }
}

void SnapshotParser_parseSnapshots(struct SnapshotParser* self, const void* data, size_t datalen) {
  fd_exec_slot_ctx_t * slot_ctx = self->slot_ctx_;

  self->manifest_ = fd_valloc_malloc( slot_ctx->valloc, FD_SOLANA_MANIFEST_ALIGN, FD_SOLANA_MANIFEST_FOOTPRINT );
  fd_bincode_decode_ctx_t ctx;
  ctx.data = data;
  ctx.dataend = (char const *)data + datalen;
  ctx.valloc  = slot_ctx->valloc;
  if ( fd_solana_manifest_decode(self->manifest_, &ctx) )
    FD_LOG_ERR(("fd_solana_manifest_decode failed"));

  if ( fd_global_import_solana_manifest(slot_ctx, self->manifest_) )
    FD_LOG_ERR(("fd_global_import_solana_manifest failed"));
}

void SnapshotParser_tarEntry(void* arg, char const * name, const void* data, size_t datalen) {
  if (datalen == 0)
    return;
  if (strncmp(name, "accounts/", sizeof("accounts/")-1) == 0)
    SnapshotParser_parsefd_solana_accounts((struct SnapshotParser*)arg, name, data, datalen);
  if (strncmp(name, "snapshots/", sizeof("snapshots/")-1) == 0 &&
      strcmp(name, "snapshots/status_cache") != 0)
    SnapshotParser_parseSnapshots((struct SnapshotParser*)arg, data, datalen);
}

// Return non-zero on end of tarball
int
SnapshotParser_moreData( void *        arg,
                         uchar const * data,
                         ulong         datalen ) {
  struct SnapshotParser* self = (struct SnapshotParser*)arg;
  return fd_tar_stream_moreData(&self->tarreader_, data, datalen, SnapshotParser_tarEntry, self);
}

#define VECT_NAME vec_fd_txnstatusidx
#define VECT_ELEMENT fd_txnstatusidx_t
#include "../../flamenco/runtime/fd_vector.h"

void
ingest_txnstatus( fd_exec_slot_ctx_t * slot_ctx,
                  fd_rocksdb_t *    rocks_db,
                  fd_slot_meta_t *  m,
                  void const *      block,
                  ulong             blocklen ) {

  vec_fd_txnstatusidx_t vec_idx;
  vec_fd_txnstatusidx_new(&vec_idx);
  ulong datamax = 1UL<<20;
  uchar* data = (uchar*)malloc(datamax);
  ulong datalen = 0;

  /* Loop across batches */
  ulong blockoff = 0;
  while (blockoff < blocklen) {
    if ( blockoff + sizeof(ulong) > blocklen )
      FD_LOG_ERR(("premature end of block"));
    ulong mcount = *(const ulong *)((const uchar *)block + blockoff);
    blockoff += sizeof(ulong);

    /* Loop across microblocks */
    for (ulong mblk = 0; mblk < mcount; ++mblk) {
      if ( blockoff + sizeof(fd_microblock_hdr_t) > blocklen )
        FD_LOG_ERR(("premature end of block"));
      fd_microblock_hdr_t * hdr = (fd_microblock_hdr_t *)((const uchar *)block + blockoff);
      blockoff += sizeof(fd_microblock_hdr_t);

      /* Loop across transactions */
      for ( ulong txn_idx = 0; txn_idx < hdr->txn_cnt; txn_idx++ ) {
        fd_txn_xray_result_t xray;
        const uchar* raw = (const uchar *)block + blockoff;
        ulong pay_sz = fd_txn_xray(raw, blocklen - blockoff, &xray);
        if ( pay_sz == 0UL )
          FD_LOG_ERR(("failed to parse transaction %lu in microblock %lu in slot %lu", txn_idx, mblk, m->slot));

        if ( xray.signature_cnt ) {
          fd_ed25519_sig_t const * sigs = (fd_ed25519_sig_t const *)((ulong)raw + (ulong)xray.signature_off);
          ulong status_sz;
          void * status = fd_rocksdb_get_txn_status_raw( rocks_db, m->slot, sigs, &status_sz );
          if ( status ) {

#if 0
            fd_solblock_TransactionStatusMeta txn_status = {0};
            pb_istream_t stream = pb_istream_from_buffer( status, status_sz );
            if( FD_UNLIKELY( !pb_decode( &stream, fd_solblock_TransactionStatusMeta_fields, &txn_status ) ) ) {
              FD_LOG_ERR(( "failed to decode txn status for slot %lu signature %64J: %s", m->slot, sigs, PB_GET_ERROR( &stream ) ));
            }
            pb_release( fd_solblock_TransactionStatusMeta_fields, &txn_status );
#endif

            for ( ulong i = 0; i < xray.signature_cnt; ++i) {
              fd_txnstatusidx_t idx;
              fd_memcpy(idx.sig, sigs + i, sizeof(fd_ed25519_sig_t));
              idx.offset = datalen;
              idx.status_sz = status_sz;
              vec_fd_txnstatusidx_push(&vec_idx, idx);
            }

            while (datalen + status_sz > datamax)
              data = (uchar*)realloc(data, (datamax += 1UL<<20));
            fd_memcpy(data + datalen, status, status_sz);
            datalen += status_sz;

            free(status);
          }
        }

        blockoff += pay_sz;
      }
    }
  }

  if (blockoff != blocklen)
    FD_LOG_ERR(("garbage at end of block"));

  FD_LOG_NOTICE(("slot %lu txn status: %lu offsets, %lu bytes of raw data", m->slot, vec_idx.cnt, datalen));

  ulong totsize = sizeof(ulong) + vec_idx.cnt*sizeof(fd_txnstatusidx_t) + datalen;
  fd_funk_rec_key_t key = fd_runtime_block_txnstatus_key(m->slot);
  int ret;
  fd_funk_rec_t * rec = fd_funk_rec_modify( slot_ctx->acc_mgr->funk, fd_funk_rec_insert( slot_ctx->acc_mgr->funk, NULL, &key, &ret ) );
  if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_rec_modify failed with code %d", ret ));
  rec = fd_funk_val_truncate( rec, totsize, (fd_alloc_t *)slot_ctx->valloc.self, slot_ctx->funk_wksp, &ret );
  if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_val_truncate failed with code %d", ret ));
  uchar * val = (uchar*) fd_funk_val( rec, slot_ctx->funk_wksp );
  *(ulong*)val = vec_idx.cnt;
  val += sizeof(ulong);
  fd_memcpy(val, vec_idx.elems, vec_idx.cnt*sizeof(fd_txnstatusidx_t));
  val += vec_idx.cnt*sizeof(fd_txnstatusidx_t);
  fd_memcpy(val, data, datalen);

  vec_fd_txnstatusidx_destroy(&vec_idx);
  free(data);
}

void
ingest_rocksdb( fd_exec_slot_ctx_t * slot_ctx,
                char const *      file,
                ulong             end_slot,
                char const *      verifypoh,
                char const *      txnstatus,
                fd_tpool_t *      tpool,
                ulong             max_workers ) {

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

  ulong start_slot = slot_ctx->bank.slot;
  if ( last_slot < start_slot ) {
    FD_LOG_ERR(("rocksdb blocks are older than snapshot. first=%lu last=%lu wanted=%lu",
                fd_rocksdb_first_slot(&rocks_db, &err), last_slot, start_slot));
  }

  FD_LOG_NOTICE(("ingesting rocksdb from start=%lu to end=%lu", start_slot, end_slot));

  fd_hash_t oldhash = slot_ctx->bank.poh;

  /* Write database-wide slot meta */

  fd_slot_meta_meta_t mm;
  mm.start_slot = start_slot;
  mm.end_slot = end_slot;
  fd_funk_rec_key_t key = fd_runtime_block_meta_key(ULONG_MAX);
  int ret;
  fd_funk_rec_t * rec = fd_funk_rec_modify( slot_ctx->acc_mgr->funk, fd_funk_rec_insert( slot_ctx->acc_mgr->funk, NULL, &key, &ret ) );
  if (rec == NULL)
    FD_LOG_ERR(("funky insert failed with code %d", ret));
  ulong sz = fd_slot_meta_meta_size(&mm);
  rec = fd_funk_val_truncate( rec, sz, (fd_alloc_t *)slot_ctx->valloc.self, slot_ctx->funk_wksp, &ret );
  if (rec == NULL)
    FD_LOG_ERR(("funky insert failed with code %d", ret));
  void * val = fd_funk_val( rec, slot_ctx->funk_wksp );
  fd_bincode_encode_ctx_t ctx;
  ctx.data = val;
  ctx.dataend = (uchar *)val + sz;
  if ( fd_slot_meta_meta_encode( &mm, &ctx ) )
    FD_LOG_ERR(("fd_slot_meta_meta_encode failed"));

  fd_rocksdb_root_iter_t iter;
  fd_rocksdb_root_iter_new ( &iter );

  fd_slot_meta_t m;
  fd_memset(&m, 0, sizeof(m));

  ret = fd_rocksdb_root_iter_seek( &iter, &rocks_db, start_slot, &m, slot_ctx->valloc );
  if (ret < 0)
    FD_LOG_ERR(("fd_rocksdb_root_iter_seek returned %d", ret));

  ulong blk_cnt = 0;
  do {
    ulong slot = m.slot;
    if (slot >= end_slot)
      break;

    /* Insert block metadata */

    key = fd_runtime_block_meta_key(slot);
    rec = fd_funk_rec_modify( slot_ctx->acc_mgr->funk, fd_funk_rec_insert( slot_ctx->acc_mgr->funk, NULL, &key, &ret ) );
    if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_rec_modify failed with code (%d-%s)", ret, fd_funk_strerror( ret ) ));
    sz  = fd_slot_meta_size(&m);
    rec = fd_funk_val_truncate( rec, sz, (fd_alloc_t *)slot_ctx->valloc.self, slot_ctx->funk_wksp, &ret );
    if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_val_truncate failed with code (%d-%s)", ret, fd_funk_strerror( ret ) ));
    val = fd_funk_val( rec, slot_ctx->funk_wksp );
    fd_bincode_encode_ctx_t ctx2;
    ctx2.data = val;
    ctx2.dataend = (uchar *)val + sz;
    FD_TEST( fd_slot_meta_encode( &m, &ctx2 ) == FD_BINCODE_SUCCESS );

    /* Read and deshred block from RocksDB */

    ulong block_sz;
    void* block = fd_rocksdb_get_block(&rocks_db, &m, slot_ctx->valloc, &block_sz);
    if( FD_UNLIKELY( !block ) ) FD_LOG_ERR(( "fd_rocksdb_get_block failed" ));

    /* Insert block to funky */

    key = fd_runtime_block_key(slot);
    rec = fd_funk_rec_modify( slot_ctx->acc_mgr->funk, fd_funk_rec_insert( slot_ctx->acc_mgr->funk, NULL, &key, &ret ) );
    if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_rec_modify failed with code %d", ret ));
    /* TODO messy valloc => alloc upcast */
    rec = fd_funk_val_truncate( rec, block_sz, slot_ctx->valloc.self, slot_ctx->funk_wksp, &ret );
    if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_val_truncate failed with code %d", ret ));
    fd_memcpy( fd_funk_val( rec, slot_ctx->funk_wksp ), block, block_sz );

    /* Read bank hash from RocksDB */

    fd_hash_t hash;
    if( FD_UNLIKELY( !fd_rocksdb_get_bank_hash( &rocks_db, m.slot, hash.hash ) ) ) {
      FD_LOG_WARNING(( "fd_rocksdb_get_bank_hash failed for slot %lu", m.slot ));
    } else {
      /* Insert bank hash to funky */
      key = fd_runtime_bank_hash_key( slot );
      rec = fd_funk_rec_modify( slot_ctx->acc_mgr->funk, fd_funk_rec_insert( slot_ctx->acc_mgr->funk, NULL, &key, &ret ) );
      if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_rec_modify failed with code %d", ret ));
      sz  = sizeof(fd_hash_t);
      rec = fd_funk_val_truncate( rec, sz, (fd_alloc_t *)slot_ctx->valloc.self, slot_ctx->funk_wksp, &ret );
      if( FD_UNLIKELY( !rec ) ) FD_LOG_ERR(( "fd_funk_val_truncate failed with code %d", ret ));
      memcpy( fd_funk_val( rec, slot_ctx->funk_wksp ), hash.hash, sizeof(fd_hash_t) );
      FD_LOG_DEBUG(( "slot=%lu bank_hash=%32J", slot, hash.hash ));
    }

    if ( strcmp(txnstatus, "true") == 0 )
      ingest_txnstatus( slot_ctx, &rocks_db, &m, block, block_sz );

    // FD_LOG_NOTICE(("slot %lu: block size %lu", slot, block_sz));
    ++blk_cnt;

    if ( strcmp(verifypoh, "true") == 0 ) {
      if ( tpool )
        fd_runtime_block_verify_tpool( slot_ctx, &m, block, block_sz, tpool, max_workers );
      else
        fd_runtime_block_verify( slot_ctx, &m, block, block_sz );
    }

    fd_valloc_free( slot_ctx->valloc, block );
    fd_bincode_destroy_ctx_t ctx = { .valloc = slot_ctx->valloc };
    fd_slot_meta_destroy(&m, &ctx);

    ret = fd_rocksdb_root_iter_next( &iter, &m, slot_ctx->valloc );
    if (ret < 0)
      FD_LOG_ERR(("fd_rocksdb_root_iter_seek returned %d", ret));
  } while (1);

  fd_rocksdb_root_iter_destroy( &iter );
  fd_rocksdb_destroy(&rocks_db);

  /* Verify messes with the poh */
  slot_ctx->bank.poh = oldhash;

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
  char const * verifypoh    = fd_env_strip_cmdline_cstr ( &argc, &argv, "--verifypoh",    NULL, "false"   );
  char const * txnstatus    = fd_env_strip_cmdline_cstr ( &argc, &argv, "--txnstatus",    NULL, "false"   );
  char const * verifyhash   = fd_env_strip_cmdline_cstr ( &argc, &argv, "--verifyhash",   NULL, NULL      );
  char const * backup       = fd_env_strip_cmdline_cstr ( &argc, &argv, "--backup",       NULL, NULL      );
  char const * capture_fpath = fd_env_strip_cmdline_cstr ( &argc, &argv, "--capture",      NULL, NULL      );

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
  ulong  smax   = 1<<25UL;  /* 32 MiB scratch memory */
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

  uchar epoch_ctx_mem[FD_EXEC_EPOCH_CTX_FOOTPRINT] __attribute__((aligned(FD_EXEC_EPOCH_CTX_ALIGN)));
  fd_exec_epoch_ctx_t * epoch_ctx = fd_exec_epoch_ctx_join( fd_exec_epoch_ctx_new( epoch_ctx_mem ) );

  uchar slot_ctx_mem[FD_EXEC_SLOT_CTX_FOOTPRINT] __attribute__((aligned(FD_EXEC_SLOT_CTX_ALIGN)));
  fd_exec_slot_ctx_t * slot_ctx = fd_exec_slot_ctx_join( fd_exec_slot_ctx_new( slot_ctx_mem ) );
  slot_ctx->epoch_ctx = epoch_ctx;

  fd_alloc_t * alloc = fd_alloc_join( fd_wksp_laddr_fast( wksp, funk->alloc_gaddr ), 0UL );
  if( FD_UNLIKELY( !alloc ) ) FD_LOG_ERR(( "fd_alloc_join(gaddr=%#lx) failed", funk->alloc_gaddr ));
  /* TODO leave */

  slot_ctx->funk_wksp = wksp;
  slot_ctx->local_wksp = NULL;
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
    uchar snapshot_used = 0;

    if( snapshotfile ) {
      struct SnapshotParser parser;
      SnapshotParser_init(&parser, slot_ctx);

      FD_LOG_NOTICE(( "reading %s", snapshotfile ));
      int fd = open( snapshotfile, O_RDONLY );
      if( FD_UNLIKELY( fd<0 ) )
        FD_LOG_ERR(( "open(%s) failed (%d-%s)", snapshotfile, errno, fd_io_strerror( errno ) ));

      int err = 0;
      if( 0==strcmp( snapshotfile + strlen(snapshotfile) - 4, ".zst" ) )
        err = fd_decompress_zstd( fd, SnapshotParser_moreData, &parser );
      else if( 0==strcmp( snapshotfile + strlen(snapshotfile) - 4, ".bz2" ) )
        err = fd_decompress_bz2( fd, SnapshotParser_moreData, &parser );
      else
        FD_LOG_ERR(( "unknown snapshot compression suffix" ));

      if( err ) FD_LOG_ERR(( "failed to load snapshot (%d-%s)", err, fd_io_strerror( err ) ));

      err = close(fd);
      if( FD_UNLIKELY( err ) )
        FD_LOG_ERR(( "close(%s) failed (%d-%s)", snapshotfile, errno, fd_io_strerror( errno ) ));

      SnapshotParser_destroy(&parser);
      snapshot_used = 1;
    }

    if( incremental ) {
      struct SnapshotParser parser;
      SnapshotParser_init(&parser, slot_ctx);

      FD_LOG_NOTICE(( "reading %s", incremental ));
      int fd = open( incremental, O_RDONLY );
      if( FD_UNLIKELY( fd<0 ) )
        FD_LOG_ERR(( "open(%s) failed (%d-%s)", incremental, errno, fd_io_strerror( errno ) ));

      int err = 0;
      if( 0==strcmp( incremental + strlen(incremental) - 4, ".zst" ) )
        err = fd_decompress_zstd( fd, SnapshotParser_moreData, &parser );
      else if( 0==strcmp( incremental + strlen(incremental) - 4, ".bz2" ) )
        err = fd_decompress_bz2( fd, SnapshotParser_moreData, &parser );
      else
        FD_LOG_ERR(( "unknown snapshot compression suffix" ));

      if( err ) FD_LOG_ERR(( "failed to load snapshot (%d-%s)", err, fd_io_strerror( err ) ));

      err = close(fd);
      if( FD_UNLIKELY( err ) )
        FD_LOG_ERR(( "close(%s) failed (%d-%s)", incremental, errno, fd_io_strerror( errno ) ));

      SnapshotParser_destroy(&parser);
      snapshot_used = 1;
    }

    if (snapshot_used) {
      FD_BORROWED_ACCOUNT_DECL(block_hashes_rec);
      int err = fd_acc_mgr_view(slot_ctx->acc_mgr, slot_ctx->funk_txn, &fd_sysvar_recent_block_hashes_id, block_hashes_rec);

      if( err != FD_ACC_MGR_SUCCESS )
        FD_LOG_ERR(( "missing recent block hashes account" ));

      fd_bincode_decode_ctx_t ctx = {
        .data       = block_hashes_rec->const_data,
        .dataend    = block_hashes_rec->const_data + block_hashes_rec->const_meta->dlen,
        .valloc     = slot_ctx->valloc
      };

      fd_recent_block_hashes_decode( &slot_ctx->bank.recent_block_hashes, &ctx );

      fd_runtime_save_banks( slot_ctx );
    }

    if( genesis ) {

      FILE *               capture_file = NULL;
      fd_solcap_writer_t * capture      = NULL;
      if( capture_fpath ) {
        capture_file = fopen( capture_fpath, "w+" );
        if( FD_UNLIKELY( !capture_file ) )
          FD_LOG_ERR(( "fopen(%s) failed (%d-%s)", capture_fpath, errno, strerror( errno ) ));

        void * capture_writer_mem = fd_alloc_malloc( alloc, fd_solcap_writer_align(), fd_solcap_writer_footprint() );
        FD_TEST( capture_writer_mem );
        capture = fd_solcap_writer_new( capture_writer_mem );

        FD_TEST( fd_solcap_writer_init( capture, capture_file ) );
        slot_ctx->capture = capture;
      }

      fd_solcap_writer_set_slot( capture, 0UL );

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
      FD_LOG_NOTICE(( "Genesis Hash: %32J", genesis_hash ));

      free(buf);

      fd_runtime_init_bank_from_genesis( slot_ctx, &genesis_block, &genesis_hash );

      fd_runtime_init_program( slot_ctx );

      FD_LOG_DEBUG(( "start genesis accounts"));

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

        err = fd_acc_mgr_commit_raw( slot_ctx->acc_mgr, rec->rec, &a->key, rec->meta, 0UL );
        if( FD_UNLIKELY( err ) )
          FD_LOG_ERR(( "fd_acc_mgr_commit_raw failed (%d)", err ));
      }

      FD_LOG_DEBUG(( "end genesis accounts"));

      for( ulong i=0; i < genesis_block.native_instruction_processors_len; i++ ) {
        fd_string_pubkey_pair_t * a = &genesis_block.native_instruction_processors[i];
        fd_write_builtin_bogus_account( slot_ctx, a->pubkey.uc, a->string, strlen(a->string) );
      }

      /* sort and update bank hash */
      int result = fd_update_hash_bank( slot_ctx, &slot_ctx->bank.banks_hash, slot_ctx->signature_cnt );
      if (result != FD_EXECUTOR_INSTR_SUCCESS) {
        return result;
      }

      slot_ctx->bank.slot = 0UL;

      FD_TEST( fd_runtime_save_banks( slot_ctx )==FD_RUNTIME_EXECUTE_SUCCESS );

      fd_bincode_destroy_ctx_t ctx2 = { .valloc = slot_ctx->valloc };
      fd_genesis_solana_destroy(&genesis_block, &ctx2);

      if( capture )  {
        fd_solcap_writer_fini( capture );
        fclose( capture_file );
      }
    }

    if( rocksdb_dir ) {
      ingest_rocksdb( slot_ctx, rocksdb_dir, end_slot, verifypoh, txnstatus, tpool, tcnt-1 );

      fd_hash_t const * known_bank_hash = fd_get_bank_hash( slot_ctx->acc_mgr->funk, slot_ctx->bank.slot );

      if( known_bank_hash ) {
        if( FD_UNLIKELY( 0!=memcmp( slot_ctx->bank.banks_hash.hash, known_bank_hash->hash, 32UL ) ) ) {
          FD_LOG_ERR(( "Bank hash mismatch! slot=%lu expected=%32J, got=%32J",
              slot_ctx->bank.slot,
              known_bank_hash->hash,
              slot_ctx->bank.banks_hash.hash ));
        }
      }
    }

    /* Dump feature activation state */

    for( fd_feature_id_t const * id = fd_feature_iter_init();
                                     !fd_feature_iter_done( id );
                                 id = fd_feature_iter_next( id ) ) {
      ulong activated_at = *fd_features_ptr_const( &slot_ctx->epoch_ctx->features, id );
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
      if( fd_hash_account_v0(acc_hash.uc, metadata, rec->pair.key->uc, fd_account_get_data(metadata), metadata->slot)==NULL )
        FD_LOG_ERR(("error processing account hash"));

      if( memcmp(acc_hash.uc, metadata->hash, 32) != 0 ) {
        FD_LOG_ERR(("account hash mismatch - num_pairs: %lu, slot: %lu, acc: %32J, acc_hash: %32J, snap_hash: %32J", num_pairs, slot_ctx->bank.slot, rec->pair.key->uc, acc_hash.uc, metadata->hash));
      }

      fd_memcpy(pairs[num_pairs].pubkey.key, rec->pair.key, 32);
      fd_memcpy(pairs[num_pairs].hash.hash, metadata->hash, 32);
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

  fd_scratch_detach( NULL );
  fd_wksp_free_laddr( smem );
  fd_wksp_free_laddr( fmem );

  fd_log_flush();
  fd_flamenco_halt();
  fd_halt();
  return 0;
}
