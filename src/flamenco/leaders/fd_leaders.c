#include "fd_leaders.h"
#include "../../ballet/chacha20/fd_chacha20rng.h"

ulong
fd_epoch_leaders_align( void ) {
  return FD_EPOCH_LEADERS_ALIGN;
}

FD_FN_CONST ulong
fd_epoch_leaders_footprint( ulong pub_cnt,
                            ulong sched_cnt ) {
  if( FD_UNLIKELY( ( pub_cnt  ==     0UL )
                 | ( pub_cnt   >UINT_MAX )
                 | ( sched_cnt==     0UL ) ) )
    return 0UL;
  return FD_EPOCH_LEADERS_FOOTPRINT( pub_cnt, sched_cnt );
}

void *
fd_epoch_leaders_new( void * shmem,
                      ulong  pub_cnt,
                      ulong  sched_cnt ) {

  if( FD_UNLIKELY( !shmem ) ) {
    FD_LOG_WARNING(( "NULL shmem" ));
    return NULL;
  }

  ulong laddr = (ulong)shmem;
  if( FD_UNLIKELY( !fd_ulong_is_aligned( laddr, FD_EPOCH_LEADERS_ALIGN ) ) ) {
    FD_LOG_WARNING(( "misaligned shmem" ));
    return NULL;
  }
  laddr += FD_LAYOUT_INIT;

  laddr  = fd_ulong_align_up( laddr, alignof(fd_epoch_leaders_t) );
  fd_epoch_leaders_t * leaders = (fd_epoch_leaders_t *)fd_type_pun( (void *)laddr );
  memset( leaders, 0, sizeof(fd_epoch_leaders_t) );
  laddr += sizeof(fd_epoch_leaders_t);

  laddr  = fd_ulong_align_up( laddr, sizeof(fd_pubkey_t) );
  leaders->pub     = (fd_pubkey_t *)laddr;
  leaders->pub_cnt = pub_cnt;
  laddr += pub_cnt*sizeof(fd_pubkey_t);

  laddr  = fd_ulong_align_up( laddr, alignof(uint) );
  leaders->sched     = (uint *)laddr;
  leaders->sched_cnt = sched_cnt;
  //laddr += sched_cnt*sizeof(uint);

  return (void *)shmem;
}

fd_epoch_leaders_t *
fd_epoch_leaders_join( void * shleaders ) {
  return (fd_epoch_leaders_t *)shleaders;
}

void *
fd_epoch_leaders_leave( fd_epoch_leaders_t * leaders ) {
  return (void *)leaders;
}

void *
fd_epoch_leaders_delete( void * shleaders ) {
  return shleaders;
}

/* fd_epoch_leaders_weighted_index performs binary search to resolve a
   sample uniformly distributed in [0,accum_stake) to a public key
   while preserving stake weight probability distribution. */

static ulong
fd_epoch_leaders_weighted_index( ulong const * scratch,
                                 ulong         stakes_cnt,
                                 ulong         roll ) {
  ulong lo = 0UL;
  ulong hi = stakes_cnt;
  while( lo<=hi ) {
    ulong idx = lo+(hi-lo)/2UL;
    if( scratch[idx]<=roll && roll<scratch[idx+1] )
      return idx;
    if( roll<scratch[idx] )
      hi = idx-1UL;
    else
      lo = idx+1UL;
  }
  __builtin_unreachable();
}

void
fd_epoch_leaders_derive( fd_epoch_leaders_t *      leaders,
                         fd_stake_weight_t const * stakes,
                         ulong                     epoch ) {

  fd_scratch_push();

  ulong pub_cnt   = leaders->pub_cnt;
  ulong sched_cnt = leaders->sched_cnt;

  /* Copy public keys */
  for( ulong i=0UL; i<pub_cnt; i++ )
    memcpy( &leaders->pub[ i ], &stakes[ i ].pub, 32UL );

  /* Create map of cumulative stake index */
  ulong * scratch = fd_scratch_alloc( alignof(ulong), pub_cnt+1UL );
  ulong accum_stake = 0UL;
  for( ulong i=0UL; i<pub_cnt; i++ ) {
    scratch[ i ] = accum_stake;
    accum_stake += stakes[ i ].stake;
  }
  scratch[ pub_cnt ] = accum_stake;

  /* Create and seed ChaCha20Rng */
  fd_chacha20rng_t _rng[1];
  fd_chacha20rng_t * rng = fd_chacha20rng_join( fd_chacha20rng_new( _rng ) );
  uchar key[ 32 ] = {0};
  memcpy( key, &epoch, sizeof(ulong) );
  fd_chacha20rng_init( rng, key );

  /* Sample leader schedule */
  for( ulong i=0UL; i<sched_cnt; i++ ) {
    ulong roll = fd_chacha20rng_ulong_roll( rng, accum_stake );
    ulong idx  = fd_epoch_leaders_weighted_index( scratch, pub_cnt, roll );
    leaders->sched[ i ] = (uint)idx;
  }

  /* Clean up */
  fd_chacha20rng_delete( fd_chacha20rng_leave( rng ) );

  fd_scratch_pop();
}

