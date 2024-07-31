#include "sys/random.h"
#include "fd_pending_slots.h"
#include "../../util/fd_util.h"
#include "../../flamenco/fd_flamenco.h"

#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

void
random_shuffle( ulong* array, ulong len ) {
    for (ulong i = 0; i < len; i++) {
      ulong idx_to_swap;
      FD_TEST( sizeof(ulong) == getrandom( &idx_to_swap, sizeof(ulong), 0 ) );
      idx_to_swap %= len;

      ulong tmp = array[i];
      array[i] = array[ idx_to_swap ];
      array[ idx_to_swap ] = tmp;
    }
}

int
main( int argc, char ** argv ) {
  fd_boot( &argc, &argv );

  FD_LOG_NOTICE(("New fd_pending_slots_t"));
  uchar __attribute__((aligned((alignof( fd_pending_slots_t ))))) buf[ fd_pending_slots_footprint() ];
  fd_pending_slots_t * pending_slots = (void*)buf;
  fd_pending_slots_new( pending_slots, 1 );

  FD_LOG_NOTICE(("Insert %lu elements into pending_slots", FD_BLOCK_MAX));
  ulong slots[FD_BLOCK_MAX];
  for (ulong i = 0; i < FD_BLOCK_MAX; i++) slots[i] = i;
  random_shuffle( slots, FD_BLOCK_MAX );

  long now = fd_log_wallclock();
  for (ulong i = 0; i < FD_BLOCK_MAX; i++) {
    long when = now + (long)slots[i];
    fd_pending_slots_add( pending_slots, slots[i], when );
  }

  FD_LOG_NOTICE(("Iterate pending_slots and check the insertion"));
  ulong iterated = 0;
  fd_pending_slots_treap_fwd_iter_t iter = fd_pending_slots_treap_fwd_iter_init( pending_slots->treap, pending_slots->pool );
  while ( !fd_pending_slots_treap_fwd_iter_done( iter ) ) {
    iterated += 1;
    fd_pending_slots_treap_ele_t * ele = fd_pending_slots_treap_fwd_iter_ele( iter, pending_slots->pool );
    FD_TEST( ele->time == now + (long)ele->slot );
    iter = fd_pending_slots_treap_fwd_iter_next( iter, pending_slots->pool );
  }
  FD_TEST( iterated == FD_BLOCK_MAX );

  FD_LOG_NOTICE(("Remove %lu slots with set_lo_wmark()", FD_BLOCK_MAX / 2));
  ulong lo_wmark = FD_BLOCK_MAX / 2 - 1;
  fd_pending_slots_set_lo_wmark( pending_slots, lo_wmark );
  /* All slots with slot# <= lo_wmark should be removed */

  FD_LOG_NOTICE(("Iterate pending_slots and check the remove"));
  iterated = 0;
  iter = fd_pending_slots_treap_fwd_iter_init( pending_slots->treap, pending_slots->pool );
  while ( !fd_pending_slots_treap_fwd_iter_done( iter ) ) {
    iterated += 1;
    fd_pending_slots_treap_ele_t * ele = fd_pending_slots_treap_fwd_iter_ele( iter, pending_slots->pool );
    FD_TEST( ele->slot > lo_wmark );
    iter = fd_pending_slots_treap_fwd_iter_next( iter, pending_slots->pool );
  }
  FD_TEST( iterated == FD_BLOCK_MAX / 2 );

  FD_LOG_NOTICE(("Insert duplicate slots with different timestamps"));
  for (ulong i = lo_wmark + 1; i < FD_BLOCK_MAX; i++) {
    long when = now + (long)i - (long)lo_wmark;
    /* This `when` is lower than what's previously inserted */
    fd_pending_slots_add( pending_slots, i, when );
  }

  FD_LOG_NOTICE(("Iterate pending_slots and check duplicate insertion works"));
  iterated = 0;
  iter = fd_pending_slots_treap_fwd_iter_init( pending_slots->treap, pending_slots->pool );
  while ( !fd_pending_slots_treap_fwd_iter_done( iter ) ) {
    iterated += 1;
    fd_pending_slots_treap_ele_t * ele = fd_pending_slots_treap_fwd_iter_ele( iter, pending_slots->pool );
    FD_TEST( ele->time == now + (long)ele->slot - (long)lo_wmark );
    iter = fd_pending_slots_treap_fwd_iter_next( iter, pending_slots->pool );
  }
  FD_TEST( iterated == FD_BLOCK_MAX / 2 );

  FD_LOG_NOTICE(("Insert %lu new elements to pending_slots", FD_BLOCK_MAX / 2));
  for (ulong i = 0; i < FD_BLOCK_MAX / 2; i++) {
    fd_pending_slots_add( pending_slots, FD_BLOCK_MAX + i, 0 );
  }

  FD_LOG_NOTICE(("Insert another new element which should trigger an error"));
  fd_pending_slots_add( pending_slots, FD_BLOCK_MAX * 2, 0 );

  return 0;
}
