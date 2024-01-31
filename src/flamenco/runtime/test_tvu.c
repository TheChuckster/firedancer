// /home/asiegel/solana/test-ledger

/* This is an attempt to wire together all the components of runtime...

   Start with a non-consensus participating, non-fork tracking tile that can
     1. receive shreds from Repair
     2. put them in the Blockstore
     3. validate and execute them

   ./build/native/gcc/unit-test/test_tvu \
      --rpc-port 8124 \
      --gossip-peer-addr 86.109.3.165:8000 \
      --repair-peer-addr 86.109.3.165:8008 \
      --repair-peer-id F7SW17yGN7UUPQ519nxaDt26UMtvwJPSFVu9kBMBQpW \
      --snapshot snapshot-24* \
      --incremental-snapshot incremental-snapshot-24* \
      --log-level-logfile 0 \
      --log-level-stderr 0

    More sample commands:

    rm -f *.zst ; wget --trust-server-names http://localhost:8899/snapshot.tar.bz2 ; wget --trust-server-names http://localhost:8899/incremental-snapshot.tar.bz2

    build/native/gcc/bin/fd_frank_ledger --cmd ingest --snapshotfile snapshot-24* --incremental incremental-snapshot-24* --pages 100 --backup /data/asiegel/test_backup --slothistory 100

    build/native/gcc/unit-test/test_tvu --load /data/asiegel/test_backup --rpc-port 8123 --page-cnt 100 \
      --gossip-peer-addr :8000 \
      --repair-peer-addr :8008 \
      --repair-peer-id F7SW17yGN7UUPQ519nxaDt26UMtvwJPSFVu9kBMBQp

*/

#define _GNU_SOURCE /* See feature_test_macros(7) */

#define FD_TVU_TILE_SLOT_DELAY 32

#include "../../flamenco/fd_flamenco.h"
#include "fd_tvu.h"
#include <signal.h>

// SIGINT signal handler
volatile int * stopflag;
static void
stop( int sig ) {
  (void)sig;
  *stopflag = 1;
}

int
main( int argc, char ** argv ) {
  signal( SIGINT, stop );
  signal( SIGPIPE, SIG_IGN );

  fd_boot( &argc, &argv );
  fd_flamenco_boot( &argc, &argv );

  fd_runtime_args_t args;
  FD_TEST(fd_tvu_parse_args( &args, argc, argv ) == 0);

  fd_runtime_ctx_t runtime_ctx;
  memset(&runtime_ctx, 0, sizeof(runtime_ctx));

  fd_tvu_repair_ctx_t repair_ctx;
  memset(&repair_ctx, 0, sizeof(repair_ctx));

  fd_tvu_gossip_ctx_t gossip_ctx;
  memset(&gossip_ctx, 0, sizeof(gossip_ctx));

  fd_tvu_main_setup( &runtime_ctx, &repair_ctx, &gossip_ctx, 1, NULL, &args);
  if( runtime_ctx.blowup ) return 1;
  stopflag = &runtime_ctx.stopflag;

  /**********************************************************************/
  /* Tile                                                               */
  /**********************************************************************/

  if( fd_tvu_main( runtime_ctx.gossip,
                   &runtime_ctx.gossip_config,
                   &repair_ctx,
                   &runtime_ctx.repair_config,
                   &runtime_ctx.stopflag,
                   args.repair_peer_id,
                   args.repair_peer_addr,
                   args.tvu_addr,
                   args.tvu_fwd_addr ) ) {
       return 1;
  }

  /***********************************************************************/
  /* Cleanup                                                             */
  /***********************************************************************/

  fd_tvu_main_teardown( &runtime_ctx, &repair_ctx );
  fd_halt();
  return 0;
}