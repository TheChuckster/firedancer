#ifndef HEADER_fd_src_flamenco_stakes_fd_stakes_h
#define HEADER_fd_src_flamenco_stakes_fd_stakes_h

#include "../fd_flamenco_base.h"
#include "../types/fd_types.h"

/* fd_stake_weight_t assigns an Ed25519 public key (node identity) a
   stake weight number measured in lamports. */

struct __attribute__((aligned(8UL))) fd_stake_weight {
  fd_pubkey_t pub;
  ulong       stake;
};
typedef struct fd_stake_weight fd_stake_weight_t;

FD_PROTOTYPES_BEGIN

/* fd_stake_weights_by_node converts Stakes (unordered list of (vote
   acc, active stake) tuples) to an ordered list of (stake, node
   identity) sorted by (stake descending, node identity descending).

   weights points to an array suitable to hold ...

     fd_vote_accounts_pair_t_map_size( accs->vote_accounts_pool,
                                       accs->vote_accounts_root )

   ... items.  On return, weights be an ordered list.

   Returns the number of items in weights (which is <= no of vote accs).
   On failure returns ULONG_MAX.  Reasons for failure include not enough
   scratch space available. */

ulong
fd_stake_weights_by_node( fd_vote_accounts_t const * accs,
                          fd_stake_weight_t *        weights );

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_stakes_fd_stakes_h */
