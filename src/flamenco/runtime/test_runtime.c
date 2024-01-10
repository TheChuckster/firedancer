/****

build/native/gcc/bin/fd_frank_ledger --rocksdb $LEDGER/rocksdb --genesis $LEDGER/genesis.bin --cmd ingest --indexmax 10000 --txnmax 100 --backup test_ledger_backup

build/native/gcc/unit-test/test_runtime --load test_ledger_backup --cmd replay --end-slot 25 --confirm_hash AsHedZaZkabNtB8XBiKWQkKwaeLy2y4Hrqm6MkQALT5h --confirm_parent CvgPeR54qpVRZGBuiQztGXecxSXREPfTF8wALujK4WdE --confirm_account_delta 7PL6JZgcNy5vkPSc6JsMHET9dvpvsFMWR734VtCG29xN  --confirm_signature 2  --confirm_last_block G4YL2SieHDGNZGjiwBsJESK7jMDfazg33ievuCwbkjrv --validate true

build/native/gcc/bin/fd_shmem_cfg reset

build/native/gcc/bin/fd_wksp_ctl new giant_wksp 200 gigantic 32-63 0666

build/native/gcc/bin/fd_frank_ledger --wksp giant_wksp --reset true --cmd ingest --snapshotfile /home/jsiegel/mainnet-ledger/snapshot-179244883-2DyMb1qN8JuTijCjsW8w4G2tg1hWuAw2AopH7Bj9Qstu.tar.zst --incremental /home/jsiegel/mainnet-ledger/incremental-snapshot-179244883-179248368-6TprbHABozQQLjjc1HBeQ2p4AigMC7rhHJS2Q5WLcbyw.tar.zst --rocksdb /home/jsiegel/mainnet-ledger/rocksdb --endslot 179249378 --backup /home/asiegel/mainnet_backup

build/native/gcc/unit-test/test_runtime --wksp giant_wksp --reset true --load /home/asiegel/mainnet_backup --cmd replay --index-max 350000000

build/native/gcc/unit-test/test_runtime --wksp giant_wksp --gaddr 0xc7ce180 --cmd replay
  NOTE: gaddr argument may be different

build/native/gcc/bin/fd_frank_ledger --wksp giant_wksp --reset true --cmd ingest --snapshotfile /data/jsiegel/mainnet-ledger/snapshot-179244883-2DyMb1qN8JuTijCjsW8w4G2tg1hWuAw2AopH7Bj9Qstu.tar.zst --incremental /data/jsiegel/mainnet-ledger/incremental-snapshot-179244883-179248368-6TprbHABozQQLjjc1HBeQ2p4AigMC7rhHJS2Q5WLcbyw.tar.zst --rocksdb /data/jsiegel/mainnet-ledger/rocksdb --endslot 179248378 --backup /data/jsiegel/mainnet_backup

build/native/gcc/unit-test/test_runtime --wksp giant_wksp --gaddr 0x000000000c7ce180 --cmd replay

/data/jsiegel/mainnet-ledger/snapshot-179244883-2DyMb1qN8JuTijCjsW8w4G2tg1hWuAw2AopH7Bj9Qstu.tar.zst
/data/jsiegel/mainnet-ledger/incremental-snapshot-179244883-179248368-6TprbHABozQQLjjc1HBeQ2p4AigMC7rhHJS2Q5WLcbyw.tar.zst

build/native/gcc/unit-test/test_runtime --wksp giant_wksp --gaddr 0xc7ce180 --cmd verifyonly

build/native/gcc/unit-test/test_runtime --wksp giant_wksp --gaddr 0xc7ce180 --cmd verifyonly --tile-cpus 32-100

build/native/gcc/bin/fd_frank_ledger --wksp giant_wksp --reset true --cmd ingest --snapshotfile /data/jsiegel/mainnet-ledger/snapshot-179244882-2DyMb1qN8JuTijCjsW8w4G2tg1hWuAw2AopH7Bj9Qstu.tar.zst --rocksdb /data/jsiegel/mainnet-ledger/rocksdb --endslot 179244982 --backup /data/asiegel/mainnet_backup

build/native/gcc/unit-test/test_runtime --wksp giant_wksp --cmd replay --load /data/asiegel/mainnet_backup

****/

#include "../fd_flamenco.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <fcntl.h>
#include "fd_rocksdb.h"
#include "fd_banks_solana.h"
#include "fd_hashes.h"
#include "fd_account.h"
#include "fd_executor.h"
#include "../../flamenco/types/fd_types.h"
#include "../../funk/fd_funk.h"
#include "../../util/alloc/fd_alloc.h"
#include "../../ballet/base58/fd_base58.h"
#include "../../ballet/poh/fd_poh.h"
#include "../../ballet/sha256/fd_sha256.h"
#include "sysvar/fd_sysvar_clock.h"
#include "sysvar/fd_sysvar.h"
#include "fd_runtime.h"
#include "fd_replay.h"
#include "sysvar/fd_sysvar_epoch_schedule.h"
#include "program/fd_stake_program.h"
#include "../stakes/fd_stakes.h"
#include "context/fd_capture_ctx.h"
#include "../../ballet/base64/fd_base64.h"
#include "fd_blockstore.h"

#include <dirent.h>
#include <signal.h>

static void
usage( char const * progname ) {
  fprintf( stderr, "USAGE: %s\n", progname );
  fprintf( stderr,
      " --wksp        <name>       workspace name\n"
      " --load        <file>       load funky backup file\n"
      " --end-slot    <num>        stop iterating at block...\n"
      " --cmd         <operation>  What operation should we test\n"
      " --index-max   <bool>       How big should the index table be?\n"
      " --validate    <bool>       Validate the funk db\n"
      " --reset       <bool>       Reset the workspace\n"
      " --capture     <file>       Write bank preimage to capture file\n"
      " --abort-on-mismatch {0,1}  If 1, stop on bank hash mismatch\n",
      " --loglevel    <level>      Set logging level\n",
      " --cap         <file>       Slot capitalization file\n",
      " --trace       <dir>        Export traces to given directory\n",
      " --retrace     <bool>       Immediately replay captured traces\n" );
}

int
main( int     argc,
      char ** argv ) {
  fd_boot         ( &argc, &argv );
  fd_flamenco_boot( &argc, &argv );

  fd_replay_state_t state;
  fd_memset(&state, 0, sizeof(state));

  state.argc = argc;
  state.argv = argv;

  state.name                = fd_env_strip_cmdline_cstr ( &argc, &argv, "--wksp",         NULL, NULL );
  state.end_slot            = fd_env_strip_cmdline_ulong( &argc, &argv, "--end-slot",     NULL, ULONG_MAX);
  state.cmd                 = fd_env_strip_cmdline_cstr ( &argc, &argv, "--cmd",          NULL, NULL);
  state.reset               = fd_env_strip_cmdline_cstr ( &argc, &argv, "--reset",        NULL, NULL);
  state.load                = fd_env_strip_cmdline_cstr ( &argc, &argv, "--load",         NULL, NULL);
  state.capitalization_file = fd_env_strip_cmdline_cstr ( &argc, &argv, "--cap",          NULL, NULL);

  state.pages               = fd_env_strip_cmdline_ulong ( &argc, &argv, "--pages",      NULL, 5);

  char const * index_max_opt           = fd_env_strip_cmdline_cstr ( &argc, &argv, "--index-max", NULL, NULL );
  char const * allocator               = fd_env_strip_cmdline_cstr ( &argc, &argv, "--allocator", NULL, "wksp" );
  char const * validate_db             = fd_env_strip_cmdline_cstr ( &argc, &argv, "--validate",  NULL, NULL );
  char const * capture_fpath           = fd_env_strip_cmdline_cstr ( &argc, &argv, "--capture",   NULL, NULL );
  char const * trace_fpath             = fd_env_strip_cmdline_cstr ( &argc, &argv, "--trace",     NULL, NULL );
  int          retrace                 = fd_env_strip_cmdline_int  ( &argc, &argv, "--retrace",   NULL, 0    );

  char const * confirm_hash            = fd_env_strip_cmdline_cstr ( &argc, &argv, "--confirm_hash",          NULL, NULL);
  char const * confirm_parent          = fd_env_strip_cmdline_cstr ( &argc, &argv, "--confirm_parent",        NULL, NULL);
  char const * confirm_account_delta   = fd_env_strip_cmdline_cstr ( &argc, &argv, "--confirm_account_delta", NULL, NULL);
  char const * confirm_signature       = fd_env_strip_cmdline_cstr ( &argc, &argv, "--confirm_signature",     NULL, NULL);
  char const * confirm_last_block      = fd_env_strip_cmdline_cstr ( &argc, &argv, "--confirm_last_block",    NULL, NULL);

  if (state.cmd == NULL) {
    usage(argv[0]);
    return 1;
  }

  state.map = capitalization_map_join(capitalization_map_new(state.capitalization_map_mem));

  state.abort_on_mismatch = (uchar)fd_env_strip_cmdline_int( &argc, &argv, "--abort-on-mismatch", NULL, 0 );

  if (NULL != state.capitalization_file) {
    FILE *fp = fopen(state.capitalization_file, "r");
    if (NULL == fp) {
      perror(state.capitalization_file);
      return -1;
    }
    ulong slot = 0;
    ulong cap = 0;
    while (fscanf(fp, "%ld,%ld", &slot, &cap) == 2) {
      slot_capitalization_t *c = capitalization_map_insert(state.map, slot);
      c->capitalization = cap;
    }
    fclose(fp);
  }

  char hostname[64];
  gethostname(hostname, sizeof(hostname));
  ulong hashseed = fd_hash(0, hostname, strnlen(hostname, sizeof(hostname)));

  fd_wksp_t *local_wksp = fd_wksp_new_anonymous( FD_SHMEM_GIGANTIC_PAGE_SZ, state.pages, 0, "local_wksp", 0UL );
  if ( FD_UNLIKELY( !local_wksp ) )
    FD_LOG_ERR(( "Unable to create local wksp" ));

  fd_wksp_t *funk_wksp;
  if ( state.name ) {
    FD_LOG_NOTICE(( "Attaching to --wksp %s", state.name ));
    funk_wksp = fd_wksp_attach( state.name );
    if ( FD_UNLIKELY( !funk_wksp ) )
      FD_LOG_ERR(( "Unable to attach to wksp" ));
    if ( state.reset && strcmp(state.reset, "true") == 0 ) {
      fd_wksp_reset( funk_wksp, (uint)hashseed);
    }
  } else {
    funk_wksp = local_wksp;
  }

  if (NULL != state.load) {
    FD_LOG_NOTICE(("loading %s", state.load));
    int err = fd_wksp_restore(funk_wksp, state.load, (uint)hashseed);
    if (err)
      FD_LOG_ERR(("restore failed: error %d", err));
  }

  void* shmem;
  fd_wksp_tag_query_info_t info;
  fd_funk_t * funk;
  ulong tag = FD_FUNK_MAGIC;
  if (fd_wksp_tag_query(funk_wksp, &tag, 1, &info, 1) > 0) {
    shmem = fd_wksp_laddr_fast( funk_wksp, info.gaddr_lo );
    funk = fd_funk_join(shmem);
    if (funk == NULL)
      FD_LOG_ERR(( "failed to join a funky" ));
  } else {
    shmem = fd_wksp_alloc_laddr( funk_wksp, fd_funk_align(), fd_funk_footprint(), FD_FUNK_MAGIC );
    if (shmem == NULL)
      FD_LOG_ERR(( "failed to allocate a funky" ));
    ulong index_max = 1000000;    // Maximum size (count) of master index
    if (index_max_opt)
      index_max = (ulong) atoi((char *) index_max_opt);
    ulong xactions_max = 100;     // Maximum size (count) of transaction index
    FD_LOG_NOTICE(("creating new funk db, index_max=%lu xactions_max=%lu", index_max, xactions_max));
    funk = fd_funk_join(fd_funk_new(shmem, 1, hashseed, xactions_max, index_max));
    if (funk == NULL) {
      fd_wksp_free_laddr(shmem);
      FD_LOG_ERR(( "failed to allocate a funky" ));
    }
  }

  fd_blockstore_t * blockstore;
  tag = FD_BLOCKSTORE_MAGIC;
  if (fd_wksp_tag_query(funk_wksp, &tag, 1, &info, 1) > 0) {
    FD_LOG_WARNING(("Wksp blockstore"));
    shmem = fd_wksp_laddr_fast( funk_wksp, info.gaddr_lo );
    blockstore = fd_blockstore_join(shmem);
    if (blockstore == NULL)
      FD_LOG_ERR(( "failed to join a blockstore" ));
  } else {
    FD_LOG_WARNING(("Alloc blockstore"));
    shmem = fd_wksp_alloc_laddr( funk_wksp, fd_blockstore_align(), fd_blockstore_footprint(), FD_BLOCKSTORE_MAGIC );
    if (shmem == NULL)
      FD_LOG_ERR(( "failed to allocate a blockstore" ));
    ulong tmp_shred_max = 1UL << 23;
    int   lg_txn_max    = 20;
    ulong slot_history_max = 6000;//FD_DEFAULT_SLOT_HISTORY_MAX;
    blockstore = fd_blockstore_join(fd_blockstore_new(shmem, 1, hashseed, tmp_shred_max, lg_txn_max, slot_history_max));
    if (blockstore == NULL) {
      fd_wksp_free_laddr(shmem);
      FD_LOG_ERR(( "failed to allocate a blockstore" ));
    }
  }

  uchar epoch_ctx_mem[FD_EXEC_EPOCH_CTX_FOOTPRINT] __attribute__((aligned(FD_EXEC_EPOCH_CTX_ALIGN)));
  state.epoch_ctx = fd_exec_epoch_ctx_join( fd_exec_epoch_ctx_new( epoch_ctx_mem ) );

  uchar slot_ctx_mem[FD_EXEC_SLOT_CTX_FOOTPRINT] __attribute__((aligned(FD_EXEC_SLOT_CTX_ALIGN)));
  state.slot_ctx = fd_exec_slot_ctx_join( fd_exec_slot_ctx_new( slot_ctx_mem ) );
  state.slot_ctx->epoch_ctx = state.epoch_ctx;

  fd_acc_mgr_t _acc_mgr[1];
  state.slot_ctx->acc_mgr = fd_acc_mgr_new( _acc_mgr, funk, blockstore );

  if ((validate_db != NULL) && (strcmp(validate_db, "true") == 0)) {
    FD_LOG_WARNING(("starting validate"));
    if ( fd_funk_verify(funk) != FD_FUNK_SUCCESS )
      FD_LOG_ERR(("valdation failed"));
    FD_LOG_INFO(("finishing validate"));
  }
  ulong r = fd_funk_txn_cancel_all(state.slot_ctx->acc_mgr->funk, 1);
  FD_LOG_INFO(( "Cancelled old transactions %lu", r ));
  void * alloc_shmem = fd_wksp_alloc_laddr( local_wksp, fd_alloc_align(), fd_alloc_footprint(), 3UL );
  if( FD_UNLIKELY( !alloc_shmem ) ) {
    FD_LOG_ERR(( "fd_alloc too large for workspace" ));
  }
  void * alloc_shalloc = fd_alloc_new( alloc_shmem, 3UL );
  if( FD_UNLIKELY( !alloc_shalloc ) ) {
    FD_LOG_ERR(( "fd_allow_new failed" ));
  }
  fd_alloc_t * alloc = fd_alloc_join( alloc_shalloc, 3UL );
  if( FD_UNLIKELY( !alloc ) ) {
    FD_LOG_ERR(( "fd_alloc_join failed" ));
  }

  state.local_wksp = local_wksp;
  if( strcmp( allocator, "libc" ) == 0 ) {
    state.slot_ctx->valloc = fd_libc_alloc_virtual();
    state.epoch_ctx->valloc = fd_libc_alloc_virtual();
  } else if ( strcmp( allocator, "wksp" ) == 0 ) {
    state.slot_ctx->valloc = fd_alloc_virtual( alloc );
    state.epoch_ctx->valloc = fd_alloc_virtual( alloc );
  } else {
    FD_LOG_ERR(( "unknown allocator specified" ));
  }

  if( capture_fpath ) {
    state.capture_file = fopen( capture_fpath, "w+" );
    if( FD_UNLIKELY( !state.capture_file ) )
      FD_LOG_ERR(( "fopen(%s) failed (%d-%s)", capture_fpath, errno, fd_io_strerror( errno ) ));


    // FD_TEST( fd_solcap_writer_init( state.capture_ctx->capture, state.capture_file ) );
  }

  if( trace_fpath ) {
    FD_LOG_NOTICE(( "Exporting traces to %s", trace_fpath ));

    if( FD_UNLIKELY( 0!=mkdir( trace_fpath, 0777 ) && errno!=EEXIST ) )
      FD_LOG_ERR(( "mkdir(%s) failed (%d-%s)", trace_fpath, errno, fd_io_strerror( errno ) ));

    int fd = open( trace_fpath, O_DIRECTORY );
    if( FD_UNLIKELY( fd<=0 ) )  /* technically 0 is valid, but it serves as a sentinel here */
      FD_LOG_ERR(( "open(%s) failed (%d-%s)", trace_fpath, errno, fd_io_strerror( errno ) ));

    // state.capture_ctx->trace_mode |= FD_RUNTIME_TRACE_SAVE;
    // state.capture_ctx->trace_dirfd = fd;
  }

  if( retrace ) {
    FD_LOG_NOTICE(( "Retrace mode enabled" ));

    state.capture_ctx->trace_mode |= FD_RUNTIME_TRACE_REPLAY;
  }

  fd_runtime_recover_banks( state.slot_ctx, 0 );

  ulong tcnt = fd_tile_cnt();
  uchar tpool_mem[ FD_TPOOL_FOOTPRINT(FD_TILE_MAX) ] __attribute__((aligned(FD_TPOOL_ALIGN)));
  fd_tpool_t * tpool = NULL;
  if ( tcnt > 1) {
    tpool = fd_tpool_init(tpool_mem, tcnt);
    if ( tpool == NULL )
      FD_LOG_ERR(("failed to create thread pool"));
    for ( ulong i = 1; i < tcnt; ++i ) {
      ulong  smax = 512 /*MiB*/ << 20;
      // ulong  smax = 2048UL /*GiB*/ << 20UL;
      void * smem = fd_wksp_alloc_laddr( state.local_wksp, fd_scratch_smem_align(), smax, 1UL );
      if( FD_UNLIKELY( !smem ) ) FD_LOG_ERR(( "Failed to alloc scratch mem" ));
      if ( fd_tpool_worker_push( tpool, i, smem, smax ) == NULL )
        FD_LOG_ERR(("failed to launch worker"));
    }
  }

  if (strcmp(state.cmd, "replay") == 0) {
    state.tpool = tpool;
    state.max_workers = tcnt;

    int err = fd_replay(&state);
    if( err!=0 ) return err;

    if (NULL != confirm_hash) {
      uchar h[32];
      fd_base58_decode_32( confirm_hash,  h);
      FD_TEST(memcmp(h, &state.slot_ctx->slot_bank.banks_hash, sizeof(h)) == 0);
    }

    if (NULL != confirm_parent) {
      uchar h[32];
      fd_base58_decode_32( confirm_parent,  h);
      FD_TEST(memcmp(h, state.slot_ctx->prev_banks_hash.uc, sizeof(h)) == 0);
    }

    if (NULL != confirm_account_delta) {
      uchar h[32];
      fd_base58_decode_32( confirm_account_delta,  h);
      FD_TEST(memcmp(h, state.slot_ctx->account_delta_hash.uc, sizeof(h)) == 0);
    }

    if (NULL != confirm_signature)
      FD_TEST((ulong) atoi(confirm_signature) == state.slot_ctx->signature_cnt);

    if (NULL != confirm_last_block) {
      uchar h[32];
      fd_base58_decode_32( confirm_last_block,  h);
      FD_TEST(memcmp(h, &state.slot_ctx->slot_bank.poh, sizeof(h)) == 0);
    }

  }

  // fd_alloc_free( alloc, fd_solcap_writer_delete( fd_solcap_writer_fini( state.capture_ctx->capture ) ) );
  if( state.capture_file  ) fclose( state.capture_file );
  // if( state.capture_ctx->trace_dirfd>0 ) close( state.capture_ctx->trace_dirfd );

  fd_valloc_free(state.slot_ctx->valloc, state.epoch_ctx->leaders);
  fd_exec_slot_ctx_delete(fd_exec_slot_ctx_leave(state.slot_ctx));
  fd_exec_epoch_ctx_delete(fd_exec_epoch_ctx_leave(state.epoch_ctx));

  // FD_TEST(fd_alloc_is_empty(alloc));
  fd_wksp_free_laddr( fd_alloc_delete( fd_alloc_leave( alloc ) ) );

  fd_wksp_delete_anonymous( state.local_wksp );
  if( state.name )
    fd_wksp_detach( funk_wksp );

  FD_LOG_NOTICE(( "pass" ));

  if( tpool != NULL ) {
    fd_tpool_fini( tpool );
  }
  fd_halt();

  return 0;
}
