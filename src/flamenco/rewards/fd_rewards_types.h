#ifndef HEADER_fd_src_flamenco_runtime_program_fd_rewards_types_h
#define HEADER_fd_src_flamenco_runtime_program_fd_rewards_types_h

#include "../fd_flamenco_base.h"
#include "../types/fd_types.h"

#define VECT_NAME fd_stake_rewards
#define VECT_ELEMENT fd_stake_reward_t*
#include "../runtime/fd_vector.h"
#undef VECT_NAME
#undef VECT_ELEMENT

#define VECT_NAME fd_stake_rewards_vector
#define VECT_ELEMENT fd_stake_rewards_t
#include "../runtime/fd_vector.h"
#undef VECT_NAME
#undef VECT_ELEMENT

/* Number of blocks for reward calculation and storing vote accounts.
   Distributing rewards to stake accounts begins AFTER this many blocks.
   
   https://github.com/anza-xyz/agave/blob/9a7bf72940f4b3cd7fc94f54e005868ce707d53d/runtime/src/bank/partitioned_epoch_rewards/mod.rs#L27 */
#define REWARD_CALCULATION_NUM_BLOCKS ( 1UL )

/* stake accounts to store in one block during partitioned reward interval Target to store 64 rewards per entry/tick in a block. A block has a minimum of 64 entries/tick. This gives 4096 total rewards to store in one block. This constant affects consensus. */
#define STAKE_ACCOUNT_STORES_PER_BLOCK          ( 4096UL )
#define TEST_ENABLE_PARTITIONED_REWARDS         ( 0UL )
#define TEST_COMPARE_PARTITIONED_EPOCH_REWARDS  ( 0UL )
#define MAX_FACTOR_OF_REWARD_BLOCKS_IN_EPOCH    ( 10UL ) 

#endif /* HEADER_fd_src_flamenco_runtime_program_fd_rewards_types_h */
