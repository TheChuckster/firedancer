#ifndef HEADER_fd_src_flamenco_stakes_fd_stakes_h
#define HEADER_fd_src_flamenco_stakes_fd_stakes_h

#include "../fd_flamenco_base.h"
#include "../types/fd_types.h"
#include "../runtime/sysvar/fd_sysvar.h"



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
#define STAKE_ACCOUNT_SIZE ( 200 )



ulong
fd_stake_weights_by_node( fd_vote_accounts_t const * accs,
                          fd_stake_weight_t *        weights );

FD_PROTOTYPES_END

void fd_stakes_init( fd_global_ctx_t * global, fd_stakes_t* stakes );

void activate_epoch( fd_global_ctx_t * global, ulong next_epoch );

fd_stake_history_entry_t stake_and_activating( fd_delegation_t const * delegation, ulong target_epoch, fd_stake_history_t * stake_history );

fd_stake_history_entry_t stake_activating_and_deactivating( fd_delegation_t const * delegation, ulong target_epoch, fd_stake_history_t * stake_history );

int write_stake_state(
    fd_global_ctx_t* global,
    fd_pubkey_t* stake_acc,
    fd_stake_state_t* stake_state,
    ushort is_new_account
);

#endif /* HEADER_fd_src_flamenco_stakes_fd_stakes_h */
