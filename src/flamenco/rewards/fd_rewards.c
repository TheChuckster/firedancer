#include "fd_rewards.h"
#include "fd_rewards_types.h"
#include <math.h>

#include "../runtime/fd_executor_err.h"
#include "../runtime/fd_system_ids.h"
#include "../runtime/context/fd_exec_epoch_ctx.h"
#include "../runtime/context/fd_exec_slot_ctx.h"
#include "../../ballet/siphash13/fd_siphash13.h"
#include "../runtime/program/fd_program_util.h"

#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

/* https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/sdk/program/src/native_token.rs#L6 */
#define LAMPORTS_PER_SOL   ( 1000000000UL )
#define FD_REWARDS_SUCCESS ( 0UL )

/* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/sdk/src/inflation.rs#L85 */
static double
total( fd_inflation_t const * inflation, double year ) {
    if ( FD_UNLIKELY( year == 0.0 ) ) {
        FD_LOG_ERR(( "inflation year 0" ));
    }
    double tapered = inflation->initial * pow((1.0 - inflation->taper), year);
    return (tapered > inflation->terminal) ? tapered : inflation->terminal;
}

/* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/sdk/src/inflation.rs#L102 */
static double
foundation( fd_inflation_t const * inflation, double year ) {
    return (year < inflation->foundation_term) ? inflation->foundation * total(inflation, year) : 0.0;
}

/* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/sdk/src/inflation.rs#L97 */
static double
validator( fd_inflation_t const * inflation, double year) {
    /* https://github.com/firedancer-io/solana/blob/dab3da8e7b667d7527565bddbdbecf7ec1fb868e/sdk/src/inflation.rs#L96-L99 */
    FD_LOG_DEBUG(("Validator Rate: %.16f %.16f %.16f %.16f %.16f", year, total( inflation, year ), foundation( inflation, year ), inflation->taper, inflation->initial));
    return total( inflation, year ) - foundation( inflation, year );
}

/* Calculates the starting slot for inflation from the activation slot. The activation slot is the earliest
    activation slot of the following features:
    - devnet_and_testnet 
    - full_inflation_enable, if full_inflation_vote has been activated

    https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank.rs#L2095 */
static FD_FN_CONST ulong
get_inflation_start_slot( fd_exec_slot_ctx_t * slot_ctx ) {
    ulong devnet_and_testnet = FD_FEATURE_ACTIVE(slot_ctx, devnet_and_testnet) ? slot_ctx->epoch_ctx->features.devnet_and_testnet : ULONG_MAX;

    ulong enable = ULONG_MAX;
    if ( FD_FEATURE_ACTIVE( slot_ctx, full_inflation_vote ) && FD_FEATURE_ACTIVE(slot_ctx, full_inflation_enable ) ) {
        enable = slot_ctx->epoch_ctx->features.full_inflation_enable;
    }

    ulong min_slot = fd_ulong_min( enable, devnet_and_testnet );
    if ( min_slot == ULONG_MAX ) {
        if ( FD_FEATURE_ACTIVE( slot_ctx, pico_inflation ) ) {
            min_slot = slot_ctx->epoch_ctx->features.pico_inflation;
        } else {
            min_slot = 0;
        }
    }
    return min_slot;
}

/* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank.rs#L2110 */
static ulong
get_inflation_num_slots( fd_exec_slot_ctx_t * slot_ctx,
                         fd_epoch_schedule_t const * epoch_schedule,
                         ulong slot ) {
    ulong inflation_activation_slot = get_inflation_start_slot( slot_ctx );
    ulong inflation_start_slot = fd_epoch_slot0(
        epoch_schedule,
        fd_ulong_sat_sub(
            fd_slot_to_epoch( epoch_schedule, inflation_activation_slot, NULL ),
            1 )
        );

    ulong epoch = fd_slot_to_epoch(epoch_schedule, slot, NULL);

    return fd_epoch_slot0(epoch_schedule, epoch) - inflation_start_slot;
}

/* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank.rs#L2121 */
static double
slot_in_year_for_inflation( fd_exec_slot_ctx_t * slot_ctx ) {
    fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );
    ulong num_slots = get_inflation_num_slots( slot_ctx, &epoch_bank->epoch_schedule, slot_ctx->slot_bank.slot );
    return (double)num_slots / (double)epoch_bank->slots_per_year;
}

/* For a given stake and vote_state, calculate how many points were earned (credits * stake) and new value
   for credits_observed were the points paid
    
    https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/programs/stake/src/points.rs#L109 */
static void
calculate_stake_points_and_credits (
  fd_stake_history_t const *     stake_history,
  fd_stake_t *                   stake,
  fd_vote_state_versioned_t *    vote_state_versioned,
  fd_calculated_stake_points_t * result
) {

    ulong credits_in_stake = stake->credits_observed;
    
    fd_vote_epoch_credits_t * epoch_credits;
    switch (vote_state_versioned->discriminant) {
        case fd_vote_state_versioned_enum_current:
            epoch_credits = vote_state_versioned->inner.current.epoch_credits;
            break;
        case fd_vote_state_versioned_enum_v0_23_5:
            epoch_credits = vote_state_versioned->inner.v0_23_5.epoch_credits;
            break;
        case fd_vote_state_versioned_enum_v1_14_11:
            epoch_credits = vote_state_versioned->inner.v1_14_11.epoch_credits;
            break;
        default:
            FD_LOG_ERR(( "invalid vote account, should never happen" ));
    }
    ulong credits_in_vote = 0UL;
    if ( FD_LIKELY( !deq_fd_vote_epoch_credits_t_empty( epoch_credits ) ) ) {
        credits_in_vote = deq_fd_vote_epoch_credits_t_peek_tail_const( epoch_credits )->credits;
    }

    /* If the Vote account has less credits observed than the Stake account,
       something is wrong and we need to force an update.
       
       https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/programs/stake/src/points.rs#L142 */
    if ( FD_UNLIKELY( credits_in_vote < credits_in_stake ) ) {
        result->points = 0;
        result->new_credits_observed = credits_in_vote;
        result->force_credits_update_with_skipped_reward = 1;
        return;
    }

    /* If the Vote account has the same amount of credits observed as the Stake account,
       then the Vote account hasn't earnt any credits and so there is nothing to update.
       
       https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/programs/stake/src/points.rs#L148 */
    if ( FD_UNLIKELY( credits_in_vote == credits_in_stake ) ) {
        result->points = 0;
        result->new_credits_observed = credits_in_vote;
        result->force_credits_update_with_skipped_reward = 0;
        return;
    }

    /* Calculate the points for each epoch credit */
    uint128 points = 0;
    ulong new_credits_observed = credits_in_stake;
    for ( deq_fd_vote_epoch_credits_t_iter_t iter = deq_fd_vote_epoch_credits_t_iter_init( epoch_credits );
          !deq_fd_vote_epoch_credits_t_iter_done( epoch_credits, iter );
          iter = deq_fd_vote_epoch_credits_t_iter_next( epoch_credits, iter ) ) {

        fd_vote_epoch_credits_t * ele = deq_fd_vote_epoch_credits_t_iter_ele( epoch_credits, iter );
        ulong final_epoch_credits = ele->credits;
        ulong initial_epoch_credits = ele->prev_credits;
        uint128 earned_credits = 0;
        if ( FD_LIKELY( credits_in_stake < initial_epoch_credits ) ) {
        earned_credits = (uint128)(final_epoch_credits - initial_epoch_credits);
        } else if ( FD_UNLIKELY( credits_in_stake < final_epoch_credits ) ) {
        earned_credits = (uint128)(final_epoch_credits - new_credits_observed);
        }

        new_credits_observed = fd_ulong_max( new_credits_observed, final_epoch_credits );

        ulong stake_amount = fd_stake_activating_and_deactivating( &stake->delegation, ele->epoch, stake_history, NULL ).effective;

        points += (uint128)stake_amount * earned_credits;
    }

    result->points = points;
    result->new_credits_observed = new_credits_observed;
    result->force_credits_update_with_skipped_reward = 0;
}

/* https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/programs/stake/src/rewards.rs#L127 */
static int
calculate_stake_rewards(
  fd_stake_history_t const *      stake_history,
  fd_stake_state_v2_t *           stake_state,
  fd_vote_state_versioned_t *     vote_state_versioned,
  ulong                           rewarded_epoch,
  fd_point_value_t *              point_value,
  fd_calculated_stake_rewards_t * result
) {
    fd_calculated_stake_points_t stake_points_result = {0};
    calculate_stake_points_and_credits( stake_history, &stake_state->inner.stake.stake, vote_state_versioned, &stake_points_result);

    // Drive credits_observed forward unconditionally when rewards are disabled
    // or when this is the stake's activation epoch
    stake_points_result.force_credits_update_with_skipped_reward |= (point_value->rewards == 0);
    stake_points_result.force_credits_update_with_skipped_reward |= (stake_state->inner.stake.stake.delegation.activation_epoch == rewarded_epoch);

    if (stake_points_result.force_credits_update_with_skipped_reward) {
        result->staker_rewards = 0;
        result->voter_rewards = 0;
        result->new_credits_observed = stake_points_result.new_credits_observed;
        return 0;
    }
    if ( stake_points_result.points == 0 || point_value->points == 0 ) {
        return 1;
    }

    /* FIXME: need to error out if the conversion from uint128 to u64 fails, also use 128 checked mul and div */
    ulong rewards = (ulong)(stake_points_result.points * (uint128)(point_value->rewards) / (uint128) point_value->points);
    if (rewards == 0) {
        return 1;
    }

    fd_commission_split_t split_result;
    fd_vote_commission_split( vote_state_versioned, rewards, &split_result );
    if (split_result.is_split && (split_result.voter_portion == 0 || split_result.staker_portion == 0)) {
        return 1;
    }

    result->staker_rewards = split_result.staker_portion;
    result->voter_rewards = split_result.voter_portion;
    result->new_credits_observed = stake_points_result.new_credits_observed;
    return 0;
}

/* https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/programs/stake/src/rewards.rs#L33 */
static int
redeem_rewards( fd_stake_history_t const *      stake_history,
                fd_stake_state_v2_t *           stake_state,
                fd_vote_state_versioned_t *     vote_state_versioned,
                ulong                           rewarded_epoch,
                fd_point_value_t *              point_value,
                fd_calculated_stake_rewards_t * calculated_stake_rewards) {

    int rc = calculate_stake_rewards( stake_history, stake_state, vote_state_versioned, rewarded_epoch, point_value, calculated_stake_rewards );
    if ( FD_UNLIKELY( rc != 0 ) ) {
        return rc;
    }

    return FD_EXECUTOR_INSTR_SUCCESS;
}

/* https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/programs/stake/src/points.rs#L70 */
int
calculate_points(
    fd_stake_state_v2_t *       stake_state,
    fd_vote_state_versioned_t * vote_state_versioned,
    fd_stake_history_t const *  stake_history,
    uint128 *                   result
) {
    if ( FD_UNLIKELY( !fd_stake_state_v2_is_stake( stake_state ) ) ) {
        return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_DATA;
    }

    fd_calculated_stake_points_t stake_point_result;
    calculate_stake_points_and_credits( stake_history, &stake_state->inner.stake.stake, vote_state_versioned, &stake_point_result );
    *result = stake_point_result.points;

    return FD_EXECUTOR_INSTR_SUCCESS;
}

/* Returns the length of the given epoch in slots

   https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/sdk/program/src/epoch_schedule.rs#L103 */
static ulong
get_slots_in_epoch(
    ulong epoch,
    fd_epoch_bank_t const * epoch_bank
) {
    return (epoch < epoch_bank->epoch_schedule.first_normal_epoch) ?
        1UL << fd_ulong_sat_add(epoch, FD_EPOCH_LEN_MIN_TRAILING_ZERO) :
        epoch_bank->epoch_schedule.slots_per_epoch;
}

/* https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/runtime/src/bank.rs#L2082 */
static double
epoch_duration_in_years(
    fd_epoch_bank_t const * epoch_bank,
    ulong prev_epoch
) {
    ulong slots_in_epoch = get_slots_in_epoch( prev_epoch, epoch_bank );
    return (double)slots_in_epoch / (double) epoch_bank->slots_per_year;
}

/* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank.rs#L2128 */
static void
calculate_previous_epoch_inflation_rewards(
    fd_exec_slot_ctx_t * slot_ctx,
    ulong prev_epoch_capitalization,
    ulong prev_epoch,
    fd_prev_epoch_inflation_rewards_t * rewards
) {
    double slot_in_year = slot_in_year_for_inflation( slot_ctx );

    fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );
    rewards->validator_rate = validator( &epoch_bank->inflation, slot_in_year );
    rewards->foundation_rate = foundation( &epoch_bank->inflation, slot_in_year );
    rewards->prev_epoch_duration_in_years = epoch_duration_in_years(epoch_bank, prev_epoch);
    rewards->validator_rewards = (ulong)(rewards->validator_rate * (double)prev_epoch_capitalization * rewards->prev_epoch_duration_in_years);
    FD_LOG_DEBUG(("Rewards %lu, Rate %.16f, Duration %.18f Capitalization %lu Slot in year %.16f", rewards->validator_rewards, rewards->validator_rate, rewards->prev_epoch_duration_in_years, prev_epoch_capitalization, slot_in_year));
}


/* Sum the lamports of the vote accounts and the delegated stake

   https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/stakes.rs#L360 */
static ulong
vote_balance_and_staked( fd_exec_slot_ctx_t * slot_ctx, fd_stakes_t const * stakes) {
    ulong result = 0;
    for( fd_vote_accounts_pair_t_mapnode_t const * n = fd_vote_accounts_pair_t_map_minimum_const( stakes->vote_accounts.vote_accounts_pool, stakes->vote_accounts.vote_accounts_root );
         n;
         n = fd_vote_accounts_pair_t_map_successor_const( stakes->vote_accounts.vote_accounts_pool, n ) ) {
        result += n->elem.value.lamports;
    }

    for( fd_vote_accounts_pair_t_mapnode_t const * n = fd_vote_accounts_pair_t_map_minimum_const( slot_ctx->slot_bank.vote_account_keys.vote_accounts_pool, slot_ctx->slot_bank.vote_account_keys.vote_accounts_root );
         n;
         n = fd_vote_accounts_pair_t_map_successor_const( slot_ctx->slot_bank.vote_account_keys.vote_accounts_pool, n ) ) {
        result += n->elem.value.lamports;
    }

    for( fd_delegation_pair_t_mapnode_t const * n = fd_delegation_pair_t_map_minimum_const( stakes->stake_delegations_pool, stakes->stake_delegations_root );
         n;
         n = fd_delegation_pair_t_map_successor_const( stakes->stake_delegations_pool, n ) ) {
        fd_pubkey_t const * stake_acc = &n->elem.account;
        FD_BORROWED_ACCOUNT_DECL(stake_acc_rec);
        if (fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, stake_acc, stake_acc_rec ) != FD_ACC_MGR_SUCCESS  ) {
            continue;
        }

        fd_stake_state_v2_t stake_state;
        if (fd_stake_get_state( stake_acc_rec, &slot_ctx->valloc, &stake_state) != 0) {
            continue;
        }
        if ( FD_UNLIKELY( !fd_stake_state_v2_is_stake( &stake_state ) ) ) {
            continue;
        }

        result += stake_state.inner.stake.stake.delegation.stake;
    }

    for( fd_stake_accounts_pair_t_mapnode_t const * n = fd_stake_accounts_pair_t_map_minimum_const( slot_ctx->slot_bank.stake_account_keys.stake_accounts_pool, slot_ctx->slot_bank.stake_account_keys.stake_accounts_root );
         n;
         n = fd_stake_accounts_pair_t_map_successor_const( slot_ctx->slot_bank.stake_account_keys.stake_accounts_pool, n ) ) {
        fd_pubkey_t const * stake_acc = &n->elem.key;
        FD_BORROWED_ACCOUNT_DECL(stake_acc_rec);
        if ( fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, stake_acc, stake_acc_rec ) != FD_ACC_MGR_SUCCESS ) {
            continue;
        }
        fd_stake_state_v2_t stake_state;
        if ( fd_stake_get_state( stake_acc_rec, &slot_ctx->valloc, &stake_state) != 0 ) {
            continue;
        }
        if ( FD_UNLIKELY( !fd_stake_state_v2_is_stake( &stake_state ) ) ) {
            continue;
        }

        result += stake_state.inner.stake.stake.delegation.stake;
    }

    return result;
}

/* https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/programs/stake/src/lib.rs#L29 */
static ulong
get_minimum_stake_delegation( fd_exec_slot_ctx_t * slot_ctx ) {
    if ( !FD_FEATURE_ACTIVE( slot_ctx, stake_minimum_delegation_for_rewards ) ) {
        return ULONG_MAX;
    }

    if ( !FD_FEATURE_ACTIVE( slot_ctx, stake_raise_minimum_delegation_to_1_sol ) ) {
        return LAMPORTS_PER_SOL;
    }

    return 1;
}

/* Calculates epoch reward points from stake/vote accounts. 

    https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L472 */
static void
calculate_reward_points_partitioned(
    fd_exec_slot_ctx_t *       slot_ctx,
    fd_stake_history_t const * stake_history,
    ulong                      rewards,
    fd_point_value_t *         result
) {
    /* There is a cache of vote account keys stored in the slot context */
    /* TODO: check this cache is correct */

    uint128 points = 0;
    fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );

    ulong minimum_stake_delegation = get_minimum_stake_delegation( slot_ctx );

    /* Calculate the points for each stake delegation */
    for( fd_delegation_pair_t_mapnode_t const * n = fd_delegation_pair_t_map_minimum_const( epoch_bank->stakes.stake_delegations_pool, epoch_bank->stakes.stake_delegations_root );
         n;
         n = fd_delegation_pair_t_map_successor_const( epoch_bank->stakes.stake_delegations_pool, n )
    ) {
        FD_SCRATCH_SCOPE_BEGIN {
            fd_valloc_t valloc = fd_scratch_virtual();

            /* Fetch the stake account */
            FD_BORROWED_ACCOUNT_DECL(stake_acc_rec);
            fd_pubkey_t const * stake_acc = &n->elem.account;
            int err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, stake_acc, stake_acc_rec);
            if ( err != FD_ACC_MGR_SUCCESS && err != FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT ) {
                FD_LOG_ERR(( "failed to read stake account from funk" ));
                continue;
            }
            if ( err == FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT ) {
                FD_LOG_DEBUG(( "stake account not found %32J", stake_acc->uc ));
                continue;
            }
            if ( stake_acc_rec->const_meta->info.lamports == 0 ) {
                FD_LOG_DEBUG(( "stake acc with zero lamports %32J", stake_acc->uc));
                continue;
            }

            /* Check the minimum stake delegation */
            fd_stake_state_v2_t stake_state[1] = {0};
            err = fd_stake_get_state( stake_acc_rec, &valloc, stake_state );
            if ( err != 0 ) {
                FD_LOG_DEBUG(( "get stake state failed" ));
                continue;
            }
            if ( FD_UNLIKELY( stake_state->inner.stake.stake.delegation.stake < minimum_stake_delegation ) ) {
                continue;
            }

            /* Check that the vote account is present in our cache */
            fd_vote_accounts_pair_t_mapnode_t key;
            fd_pubkey_t const * voter_acc = &n->elem.delegation.voter_pubkey;
            fd_memcpy( &key.elem.key, voter_acc, sizeof(fd_pubkey_t) );
            fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank( 
                slot_ctx->epoch_ctx );
            if ( FD_UNLIKELY( fd_vote_accounts_pair_t_map_find( 
                epoch_bank->stakes.vote_accounts.vote_accounts_pool,
                epoch_bank->stakes.vote_accounts.vote_accounts_root,
                &key ) == NULL ) ) {
                FD_LOG_DEBUG(( "vote account missing from cache" ));
                continue;
            }

            /* Check that the vote account is valid and has the correct owner */
            FD_BORROWED_ACCOUNT_DECL(voter_acc_rec);
            err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, voter_acc, voter_acc_rec );
            if ( FD_UNLIKELY( err ) ) {
                FD_LOG_DEBUG(( "failed to read vote account from funk" ));
                continue;
            }
            if( FD_UNLIKELY( memcmp( &voter_acc_rec->const_meta->info.owner, fd_solana_vote_program_id.key, sizeof(fd_pubkey_t) ) != 0 ) ) {
                FD_LOG_DEBUG(( "vote account has wrong owner" ));
                continue;
            }
            fd_bincode_decode_ctx_t decode = {
                .data    = voter_acc_rec->const_data,
                .dataend = voter_acc_rec->const_data + voter_acc_rec->const_meta->dlen,
                .valloc  = valloc,
            };
            fd_vote_state_versioned_t vote_state[1] = {0};
            if( FD_UNLIKELY( 0!=fd_vote_state_versioned_decode( vote_state, &decode ) ) ) {
                FD_LOG_DEBUG(( "vote_state_versioned_decode failed" ));
                continue;
            }

            uint128 account_points;
            err = calculate_points( stake_state, vote_state, stake_history, &account_points );
            if ( FD_UNLIKELY( err ) ) {
                FD_LOG_DEBUG(( "failed to calculate points" ));
                continue;
            }

            points += account_points;
        } FD_SCRATCH_SCOPE_END;
    }

    /* FIXME: do we need to also calculate points for each stake account in the slot_bank.stake_account_keys.stake_accounts_pool */

    if (points > 0) {
        result->points = points;
        result->rewards = rewards;
    } else {
        result = NULL;
    }
}

/* Calculate the partitioned stake rewards for a single stake/vote account pair, updates result with these. */
static void
calculate_stake_vote_rewards_account(
    fd_exec_slot_ctx_t *                        slot_ctx,
    fd_stake_history_t const *                  stake_history,
    ulong                                       rewarded_epoch,
    fd_point_value_t *                          point_value,
    fd_pubkey_t const *                         stake_acc,
    fd_acc_lamports_t *                         total_stake_rewards,
    fd_calculate_stake_vote_rewards_result_t *  result
) {
    fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );
    ulong minimum_stake_delegation = get_minimum_stake_delegation( slot_ctx );

    FD_BORROWED_ACCOUNT_DECL( stake_acc_rec );
    if( fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, stake_acc, stake_acc_rec) != 0 ) {
        FD_LOG_ERR(( "Stake acc not found %32J", stake_acc->uc ));
    }

    fd_stake_state_v2_t stake_state[1] = {0};
    if ( fd_stake_get_state( stake_acc_rec, &slot_ctx->valloc, stake_state ) != 0 ) {
        FD_LOG_ERR(( "Failed to read stake state from stake account %32J", stake_acc ));
    }
    if ( !fd_stake_state_v2_is_stake( stake_state ) ) {
        FD_LOG_DEBUG(( "stake account does not have active delegation" ));
        return;
    }
    fd_pubkey_t const * voter_acc = &stake_state->inner.stake.stake.delegation.voter_pubkey;

    if ( FD_FEATURE_ACTIVE(slot_ctx, stake_minimum_delegation_for_rewards )) {
      if ( stake_state->inner.stake.stake.delegation.stake < minimum_stake_delegation ) {
        return;
      }
    }

    fd_vote_accounts_pair_t_mapnode_t key;
    fd_memcpy( &key.elem.key, voter_acc, sizeof(fd_pubkey_t) );
    if ( fd_vote_accounts_pair_t_map_find( epoch_bank->stakes.vote_accounts.vote_accounts_pool, epoch_bank->stakes.vote_accounts.vote_accounts_root, &key ) == NULL
        && fd_vote_accounts_pair_t_map_find( slot_ctx->slot_bank.vote_account_keys.vote_accounts_pool, slot_ctx->slot_bank.vote_account_keys.vote_accounts_root, &key ) == NULL) {
      return;
    }

    FD_BORROWED_ACCOUNT_DECL( voter_acc_rec );
    int read_err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, voter_acc, voter_acc_rec );
    if( read_err!=0 || memcmp( &voter_acc_rec->const_meta->info.owner, fd_solana_vote_program_id.key, sizeof(fd_pubkey_t) ) != 0 ) {
      return;
    }

    fd_valloc_t valloc = fd_scratch_virtual();
    fd_bincode_decode_ctx_t decode = {
        .data    = voter_acc_rec->const_data,
        .dataend = voter_acc_rec->const_data + voter_acc_rec->const_meta->dlen,
        .valloc  = valloc,
    };
    fd_vote_state_versioned_t vote_state_versioned[1] = {0};
    if( fd_vote_state_versioned_decode( vote_state_versioned, &decode ) != 0 ) {
      return;
    }

    /* Note, this doesn't actually redeem any rewards.. this is a misnomer. */
    fd_calculated_stake_rewards_t calculated_stake_rewards[1] = {0};
    int err = redeem_rewards( stake_history, stake_state, vote_state_versioned, rewarded_epoch, point_value, calculated_stake_rewards );
    if ( err != 0) {
        FD_LOG_DEBUG(( "redeem_rewards failed for %32J with error %d", stake_acc->key, err ));
        return;
    }

    /* Fetch the comission for the vote account */
    ulong * commission = fd_valloc_malloc( slot_ctx->valloc, 1UL, 1UL );
    switch (vote_state_versioned->discriminant) {
        case fd_vote_state_versioned_enum_current:
            *commission = (uchar)vote_state_versioned->inner.current.commission;
            break;
        case fd_vote_state_versioned_enum_v0_23_5:
            *commission = (uchar)vote_state_versioned->inner.v0_23_5.commission;
            break;
        case fd_vote_state_versioned_enum_v1_14_11:
            *commission = (uchar)vote_state_versioned->inner.v1_14_11.commission;
            break;
        default:
            FD_LOG_DEBUG(( "unsupported vote account" ));
            return;
    }

    /* Update the vote reward in the map */
    fd_vote_reward_t_mapnode_t vote_map_key[1];
    fd_memcpy( &vote_map_key->elem.pubkey, voter_acc, sizeof(fd_pubkey_t) );
    fd_vote_reward_t_mapnode_t * vote_reward_node = fd_vote_reward_t_map_find( result->vote_reward_map_pool, result->vote_reward_map_root, vote_map_key );
    if ( vote_reward_node == NULL ) {
        vote_reward_node = fd_vote_reward_t_map_acquire( result->vote_reward_map_pool );
        fd_memcpy( &vote_reward_node->elem.pubkey, voter_acc, sizeof(fd_pubkey_t) );
        vote_reward_node->elem.commission = (uchar)*commission;
        vote_reward_node->elem.vote_rewards = calculated_stake_rewards->voter_rewards;
        vote_reward_node->elem.needs_store = 1;
        fd_vote_reward_t_map_insert( result->vote_reward_map_pool, &result->vote_reward_map_root, vote_reward_node );
    } else {
        vote_reward_node->elem.needs_store = 1;
        vote_reward_node->elem.vote_rewards = fd_ulong_sat_add(
            vote_reward_node->elem.vote_rewards, calculated_stake_rewards->voter_rewards
        );
    }

    /* Track the total stake rewards */
    *total_stake_rewards += calculated_stake_rewards->staker_rewards;

    /* Add the partitioned stake reward to the partitioned stake reward  */
    fd_partitioned_stake_reward_t partitioned_stake_reward;
    fd_memcpy( &partitioned_stake_reward.stake_pubkey, stake_acc, sizeof(fd_pubkey_t) );
    partitioned_stake_reward.stake_reward_info = (fd_reward_info_t) {
        .reward_type = { .discriminant = fd_reward_type_enum_staking },
        .commission = commission,
        .lamports = calculated_stake_rewards->staker_rewards,
        .post_balance = stake_acc_rec->const_meta->info.lamports + calculated_stake_rewards->staker_rewards,
    };
    /* TODO: avoid this copy by keeping a reference around? */
    fd_memcpy( &partitioned_stake_reward.stake, &stake_state->inner.stake.stake, FD_STAKE_FOOTPRINT );

    /* TODO: use zero-copy API to push to queue */
    deq_fd_partitioned_stake_reward_t_push_tail( result->stake_reward_calculation.stake_reward_deq, partitioned_stake_reward );
}

/* Calculates epoch rewards for stake/vote accounts.
   Returns vote rewards, stake rewards, and the sum of all stake rewards in lamports.

   https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L334 */
static void
calculate_stake_vote_rewards(
    fd_exec_slot_ctx_t *                       slot_ctx,
    fd_stake_history_t const *                 stake_history,
    ulong                                      rewarded_epoch,
    fd_point_value_t *                         point_value,
    fd_calculate_stake_vote_rewards_result_t * result
) {
    fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );
    /* FIXME: correct value for max */
    /* FIXME: need to clean these up */
    result->stake_reward_calculation.stake_reward_deq = deq_fd_partitioned_stake_reward_t_alloc( slot_ctx->valloc, fd_ulong_pow2( 24 ) );
    result->vote_reward_map_pool = fd_vote_reward_t_map_alloc( slot_ctx->valloc, fd_ulong_pow2( 24 ) ); /* 2^24 slots */
    result->vote_reward_map_root = NULL;

    /* Loop over all the delegations
    
        https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L367  */
    for( fd_delegation_pair_t_mapnode_t const * n = fd_delegation_pair_t_map_minimum_const(
        epoch_bank->stakes.stake_delegations_pool, epoch_bank->stakes.stake_delegations_root );
         n;
         n = fd_delegation_pair_t_map_successor_const( epoch_bank->stakes.stake_delegations_pool, n )
    ) {
        fd_pubkey_t const * stake_acc = &n->elem.account;

        calculate_stake_vote_rewards_account(
            slot_ctx,
            stake_history,
            rewarded_epoch,
            point_value,
            stake_acc,
            &result->stake_reward_calculation.total_stake_rewards_lamports,
            result );
    }

    /* Loop over all the stake accounts in the slot bank pool */
    for ( fd_stake_accounts_pair_t_mapnode_t const * n = fd_stake_accounts_pair_t_map_minimum_const( slot_ctx->slot_bank.stake_account_keys.stake_accounts_pool, slot_ctx->slot_bank.stake_account_keys.stake_accounts_root );
         n;
         n = fd_stake_accounts_pair_t_map_successor_const( slot_ctx->slot_bank.stake_account_keys.stake_accounts_pool, n) ) {

        fd_pubkey_t const * stake_acc = &n->elem.key;
        calculate_stake_vote_rewards_account(
            slot_ctx,
            stake_history,
            rewarded_epoch,
            point_value,
            stake_acc,
            &result->stake_reward_calculation.total_stake_rewards_lamports,
            result );
    }
}

/* Calculate epoch reward and return vote and stake rewards.

   https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L273 */
static void
calculate_validator_rewards(
    fd_exec_slot_ctx_t * slot_ctx,
    ulong rewarded_epoch,
    ulong rewards,
    fd_calculate_validator_rewards_result_t * result
) {
    /* https://github.com/firedancer-io/solana/blob/dab3da8e7b667d7527565bddbdbecf7ec1fb868e/runtime/src/bank.rs#L2759-L2786 */
    fd_stake_history_t const * stake_history = fd_sysvar_cache_stake_history( slot_ctx->sysvar_cache );
    if( FD_UNLIKELY( !stake_history ) ) {
        FD_LOG_ERR(( "StakeHistory sysvar is missing from sysvar cache" ));
    }

    /* Calculate the epoch reward points from stake/vote accounts */
    fd_point_value_t point_value_result[1] = {0};
    calculate_reward_points_partitioned( slot_ctx, stake_history, rewards, point_value_result );

    /* Calculate the stake and vote rewards for each account */
    calculate_stake_vote_rewards(
        slot_ctx,
        stake_history,
        rewarded_epoch,
        point_value_result,
        &result->calculate_stake_vote_rewards_result );
}

/* Calculate the number of blocks required to distribute rewards to all stake accounts.

    https://github.com/anza-xyz/agave/blob/9a7bf72940f4b3cd7fc94f54e005868ce707d53d/runtime/src/bank/partitioned_epoch_rewards/mod.rs#L214
 */
static ulong
get_reward_distribution_num_blocks(
    fd_epoch_schedule_t const * epoch_schedule,
    ulong slot,
    ulong total_stake_accounts
) {
    /* https://github.com/firedancer-io/solana/blob/dab3da8e7b667d7527565bddbdbecf7ec1fb868e/runtime/src/bank.rs#L1250-L1267 */
    if ( epoch_schedule->warmup &&
         fd_slot_to_epoch( epoch_schedule, slot, NULL ) < epoch_schedule->first_normal_epoch ) {
        return 1UL;
    }

    ulong num_chunks = total_stake_accounts / (ulong)STAKE_ACCOUNT_STORES_PER_BLOCK + (total_stake_accounts % STAKE_ACCOUNT_STORES_PER_BLOCK != 0);
    num_chunks = fd_ulong_max(num_chunks, 1);
    num_chunks = fd_ulong_min(
        fd_ulong_max(
            epoch_schedule->slots_per_epoch / (ulong)MAX_FACTOR_OF_REWARD_BLOCKS_IN_EPOCH,
            1),
        1);
    return num_chunks;
}

static void
hash_rewards_into_partitions(
    fd_exec_slot_ctx_t * slot_ctx,
    fd_partitioned_stake_reward_t * stake_reward_deq,
    ulong num_partitions,
    fd_hash_t * parent_blockhash,
    fd_partitioned_rewards_calculation_t * result
) {
    /* https://github.com/firedancer-io/solana/blob/dab3da8e7b667d7527565bddbdbecf7ec1fb868e/runtime/src/epoch_rewards_hasher.rs#L43C31-L61 */
    fd_siphash13_t  _sip[1] = {0};
    fd_siphash13_t * hasher = fd_siphash13_init( _sip, 0UL, 0UL );

    hasher = fd_siphash13_append( hasher, parent_blockhash->hash, sizeof(fd_hash_t) );

    result->stake_rewards_by_partition.stake_rewards_by_partition_len = num_partitions;
    result->stake_rewards_by_partition.stake_rewards_by_partition = 
        fd_valloc_malloc( 
            slot_ctx->valloc,
            FD_PARTITIONED_STAKE_REWARDS_ALIGN,
            FD_PARTITIONED_STAKE_REWARDS_FOOTPRINT * num_partitions );

    /* FIXME: remove this large allocation */
    ulong max_partition_size = deq_fd_partitioned_stake_reward_t_cnt( stake_reward_deq );
    for ( ulong i = 0; i < num_partitions; ++i ) {
        result->stake_rewards_by_partition.stake_rewards_by_partition[ i ].partition_len = 0UL;
        result->stake_rewards_by_partition.stake_rewards_by_partition[ i ].partition =
            fd_valloc_malloc( 
                slot_ctx->valloc,
                FD_PARTITIONED_STAKE_REWARD_ALIGN,
                FD_PARTITIONED_STAKE_REWARD_FOOTPRINT * max_partition_size );
    }
    for (
        deq_fd_partitioned_stake_reward_t_iter_t iter = deq_fd_partitioned_stake_reward_t_iter_init(
            stake_reward_deq );
        !deq_fd_partitioned_stake_reward_t_iter_done( stake_reward_deq, iter );
        iter = deq_fd_partitioned_stake_reward_t_iter_next( stake_reward_deq, iter)
    ) {
        fd_partitioned_stake_reward_t * ele = deq_fd_partitioned_stake_reward_t_iter_ele( stake_reward_deq, iter );
        /* hash_address_to_partition: find partition index (0..partitions) by hashing `address` with the `hasher` */
        fd_siphash13_append( hasher, (const uchar *) ele->stake_pubkey.key, sizeof(fd_pubkey_t) );
        ulong hash64 = fd_siphash13_fini( hasher );
        /* hash_to_partition */
        /* FIXME: should be saturating add */
        ulong partition_index = (ulong)(
            (uint128) num_partitions *
            (uint128) hash64 /
            ((uint128)ULONG_MAX + 1)
        );

        /* FIXME: avoid this copy */
        result->stake_rewards_by_partition.stake_rewards_by_partition[ partition_index ].partition[ result->stake_rewards_by_partition.stake_rewards_by_partition[ num_partitions ].partition_len++ ] = *ele;
    }
}

/* Calculate rewards from previous epoch to prepare for partitioned distribution.

   https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L214 */
static void
calculate_rewards_for_partitioning(
    fd_exec_slot_ctx_t * slot_ctx,
    ulong prev_epoch,
    fd_hash_t * parent_blockhash,
    fd_partitioned_rewards_calculation_t * result
) {
    /* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L227 */
    fd_prev_epoch_inflation_rewards_t rewards;
    calculate_previous_epoch_inflation_rewards( slot_ctx, slot_ctx->slot_bank.capitalization, prev_epoch, &rewards );

    fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );
    fd_slot_bank_t const * slot_bank = &slot_ctx->slot_bank;
    ulong old_vote_balance_and_staked = vote_balance_and_staked( slot_ctx, &epoch_bank->stakes );

    fd_calculate_validator_rewards_result_t validator_result[1] = {0};
    calculate_validator_rewards( slot_ctx, prev_epoch, rewards.validator_rewards, validator_result );

    /* TODO: pass data structures along */
    ulong num_partitions = get_reward_distribution_num_blocks( 
        &epoch_bank->epoch_schedule,
        slot_bank->slot,
        deq_fd_partitioned_stake_reward_t_cnt( validator_result->calculate_stake_vote_rewards_result.stake_reward_calculation.stake_reward_deq ) );

    hash_rewards_into_partitions(
        slot_ctx,
        validator_result->calculate_stake_vote_rewards_result.stake_reward_calculation.stake_reward_deq,
        num_partitions,
        parent_blockhash,
        result );

    /* TODO: free stake deque */

    result->vote_reward_map_pool = validator_result->calculate_stake_vote_rewards_result.vote_reward_map_pool;
    result->vote_reward_map_root = validator_result->calculate_stake_vote_rewards_result.vote_reward_map_root;
    result->old_vote_balance_and_staked = old_vote_balance_and_staked;
    result->validator_rewards = rewards.validator_rewards;
    result->validator_rate = rewards.validator_rate;
    result->foundation_rate = rewards.foundation_rate;
    result->prev_epoch_duration_in_years = rewards.prev_epoch_duration_in_years;
    result->capitalization = slot_bank->capitalization;
    result->total_points = validator_result->total_points;
}

/* Calculate rewards from previous epoch and distribute vote rewards 
   
   https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L97 */
static void
calculate_rewards_and_distribute_vote_rewards(
    fd_exec_slot_ctx_t * slot_ctx,
    ulong prev_epoch,
    fd_hash_t * parent_blockhash,
    fd_calculate_rewards_and_distribute_vote_rewards_result_t * result
) {
    /* https://github.com/firedancer-io/solana/blob/dab3da8e7b667d7527565bddbdbecf7ec1fb868e/runtime/src/bank.rs#L2406-L2492 */
    fd_partitioned_rewards_calculation_t rewards_calc_result[1] = {0};
    calculate_rewards_for_partitioning( slot_ctx, prev_epoch, parent_blockhash, rewards_calc_result );

    /* Iterate over all the vote reward nodes, deleting as we go */
    fd_vote_reward_t_mapnode_t* next_vote_reward_node;
    for ( fd_vote_reward_t_mapnode_t* vote_reward_node = fd_vote_reward_t_map_minimum(
            rewards_calc_result->vote_reward_map_pool,
            rewards_calc_result->vote_reward_map_root);
            vote_reward_node;
            vote_reward_node = next_vote_reward_node ) {

        fd_pubkey_t const * vote_pubkey = &vote_reward_node->elem.pubkey;
        FD_BORROWED_ACCOUNT_DECL( vote_rec );
        FD_TEST( fd_acc_mgr_modify( slot_ctx->acc_mgr, slot_ctx->funk_txn, vote_pubkey, 1, 0UL, vote_rec ) == FD_ACC_MGR_SUCCESS );

        FD_TEST( fd_borrowed_account_checked_add_lamports( vote_rec, vote_reward_node->elem.vote_rewards ) == 0 );

        next_vote_reward_node = fd_vote_reward_t_map_successor( rewards_calc_result->vote_reward_map_pool, vote_reward_node );
      
        /* TODO: delete vote reward node */
    }

    // This is for vote rewards only.
    fd_epoch_bank_t * epoch_bank = fd_exec_epoch_ctx_epoch_bank( slot_ctx->epoch_ctx );

    /* TODO: make sure this works for the vote accounts */
    ulong new_vote_balance_and_staked = vote_balance_and_staked( slot_ctx, &epoch_bank->stakes );
    ulong validator_rewards_paid = fd_ulong_sat_sub( new_vote_balance_and_staked, rewards_calc_result->old_vote_balance_and_staked );

    // verify that we didn't pay any more than we expected to
    result->total_rewards = fd_ulong_sat_add( validator_rewards_paid,  rewards_calc_result->stake_rewards_by_partition.total_stake_rewards_lamports );
    FD_TEST( rewards_calc_result->validator_rewards >= result->total_rewards );

    slot_ctx->slot_bank.capitalization += validator_rewards_paid;

    result->distributed_rewards = validator_rewards_paid;

    result->stake_rewards_by_partition = rewards_calc_result->stake_rewards_by_partition;
    result->total_points = rewards_calc_result->total_points;
}

/* Distributes a single partitioned reward to a single stake account */
static int
distribute_epoch_reward_to_stake_acc( 
    fd_exec_slot_ctx_t * slot_ctx,
    fd_pubkey_t * stake_pubkey,
    ulong reward_lamports
 ) {

    FD_BORROWED_ACCOUNT_DECL( stake_acc_rec );
    FD_TEST( fd_acc_mgr_modify( slot_ctx->acc_mgr, slot_ctx->funk_txn, stake_pubkey, 0, 0UL, stake_acc_rec ) == FD_ACC_MGR_SUCCESS );

    fd_stake_state_v2_t stake_state[1] = {0};
    if ( fd_stake_get_state(stake_acc_rec, &slot_ctx->valloc, stake_state) != 0 ) {
        FD_LOG_DEBUG(( "failed to read stake state for %32J", stake_pubkey ));
        return 1;
    }

    if ( !fd_stake_state_v2_is_stake( stake_state ) ) {
        /* FIXME: can this happen? what if the stake account changes? */
        FD_LOG_ERR(( "non-stake stake account, this should never happen" ));
    }

    if( fd_borrowed_account_checked_add_lamports( stake_acc_rec, reward_lamports ) ) {
        FD_LOG_DEBUG(( "failed to add lamports to stake account" ));
        return 1;
    }

    stake_state->inner.stake.stake.delegation.stake = fd_ulong_sat_add(
        stake_state->inner.stake.stake.delegation.stake,
        reward_lamports
    );

    return 0;
}

/*  Process reward credits for a partition of rewards.
    Store the rewards to AccountsDB, update reward history record and total capitalization
    
    https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/runtime/src/bank/partitioned_epoch_rewards/distribution.rs#L88 */
static void
distribute_epoch_rewards_in_partition(
    fd_partitioned_stake_rewards_t * all_stake_rewards,
    ulong all_stake_rewards_len,
    ulong partition_index,
    fd_exec_slot_ctx_t * slot_ctx
) {

    FD_TEST( partition_index < all_stake_rewards_len ); 
    fd_partitioned_stake_rewards_t * this_partition_stake_rewards = &all_stake_rewards[ partition_index ];

    ulong lamports_distributed = 0UL;
    ulong lamports_burned = 0UL;
    
    for ( ulong i = 0; i < this_partition_stake_rewards->partition_len; i++ ) {
        fd_partitioned_stake_reward_t * partitioned_reward = &this_partition_stake_rewards->partition[ i ];
        ulong reward_amount = partitioned_reward->stake_reward_info.lamports;

        if ( distribute_epoch_reward_to_stake_acc( slot_ctx, &partitioned_reward->stake_pubkey, reward_amount ) == 0 ) {
            lamports_distributed += reward_amount;
        } else {
            lamports_burned += reward_amount;
        }

        FD_BORROWED_ACCOUNT_DECL( stake_acc_rec );
        FD_TEST( fd_acc_mgr_modify( slot_ctx->acc_mgr, slot_ctx->funk_txn, &partitioned_reward->stake_pubkey, 0, 0UL, stake_acc_rec ) == FD_ACC_MGR_SUCCESS );

        FD_TEST( fd_borrowed_account_checked_add_lamports( stake_acc_rec, partitioned_reward->stake_reward_info.lamports ) == 0 );
    }

    FD_LOG_DEBUG(( "lamports burned: %lu, lamports distributed: %lu", lamports_burned, lamports_distributed ));

}

/* Process reward distribution for the block if it is inside reward interval.

   https://github.com/anza-xyz/agave/blob/cbc8320d35358da14d79ebcada4dfb6756ffac79/runtime/src/bank/partitioned_epoch_rewards/distribution.rs#L42 */
void
fd_distribute_partitioned_epoch_rewards(
    fd_exec_slot_ctx_t * slot_ctx
) {
    if ( !fd_epoch_reward_status_is_Active( &slot_ctx->epoch_reward_status ) ) {
        return;
    }
    fd_start_block_height_and_rewards_t * status = &slot_ctx->epoch_reward_status.inner.Active;

    fd_slot_bank_t * slot_bank = &slot_ctx->slot_bank;
    ulong height = slot_bank->block_height;
    fd_epoch_bank_t const * epoch_bank = fd_exec_epoch_ctx_epoch_bank_const( slot_ctx->epoch_ctx );

    ulong distribution_starting_block_height = status->distribution_starting_block_height;
    ulong distribution_end_exclusive = distribution_starting_block_height + status->stake_rewards_by_partition_len;

    /* FIXME: track current epoch in epoch ctx? */
    ulong epoch = fd_slot_to_epoch( &epoch_bank->epoch_schedule, slot_bank->slot, NULL );
    FD_TEST( get_slots_in_epoch( epoch, epoch_bank ) > status->stake_rewards_by_partition_len );

    if ( ( height >= distribution_starting_block_height ) && ( height < distribution_end_exclusive ) ) {
        ulong partition_index = height - distribution_starting_block_height;
        distribute_epoch_rewards_in_partition(
            status->stake_rewards_by_partition,
            status->stake_rewards_by_partition_len,
            partition_index,
            slot_ctx
        );
    }

    /* If we have finished distributing rewards, set the status to inactive */
    if ( fd_ulong_sat_add( height, 1UL ) >= distribution_end_exclusive ) {
        slot_ctx->epoch_reward_status.discriminant = fd_epoch_reward_status_enum_Inactive;
        fd_sysvar_epoch_rewards_set_inactive( slot_ctx );
    }
}

/* Non-partitioned epoch rewards entry-point. This uses the same logic as the partitioned epoch rewards code, 
   but distributes the rewards in one go.  */
void
fd_update_rewards(
    fd_exec_slot_ctx_t * slot_ctx,
    fd_hash_t * blockhash,
    ulong parent_epoch
) {

    /* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L55 */
    fd_calculate_rewards_and_distribute_vote_rewards_result_t rewards_result[1] = {0};
    calculate_rewards_and_distribute_vote_rewards(
        slot_ctx,
        parent_epoch,
        blockhash,
        rewards_result
    );

    /* Distribute all of the partitioned epoch rewards in one go */
    for ( ulong i = 0UL; i < rewards_result->stake_rewards_by_partition.stake_rewards_by_partition_len; i++ ) {
        distribute_epoch_rewards_in_partition(
            rewards_result->stake_rewards_by_partition.stake_rewards_by_partition,
            rewards_result->stake_rewards_by_partition.stake_rewards_by_partition_len,
            i,
            slot_ctx
        );
    }
}

/* Partitioned epoch rewards entry-point.

   https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L41
*/
void
fd_begin_partitioned_rewards(
    fd_exec_slot_ctx_t * slot_ctx,
    fd_hash_t * blockhash,
    ulong parent_epoch
) {
    /* https://github.com/anza-xyz/agave/blob/7117ed9653ce19e8b2dea108eff1f3eb6a3378a7/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L55 */
    fd_calculate_rewards_and_distribute_vote_rewards_result_t rewards_result[1] = {0};
    calculate_rewards_and_distribute_vote_rewards(
        slot_ctx,
        parent_epoch,
        blockhash,
        rewards_result
    );

    /* https://github.com/anza-xyz/agave/blob/9a7bf72940f4b3cd7fc94f54e005868ce707d53d/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L62 */
    ulong distribution_starting_block_height = slot_ctx->slot_bank.block_height + REWARD_CALCULATION_NUM_BLOCKS;
    
    /* Set the epoch reward status to be active */
    slot_ctx->epoch_reward_status.discriminant = fd_epoch_reward_status_enum_Active;
    slot_ctx->epoch_reward_status.inner.Active.distribution_starting_block_height = distribution_starting_block_height;
    slot_ctx->epoch_reward_status.inner.Active.stake_rewards_by_partition_len = rewards_result->stake_rewards_by_partition.stake_rewards_by_partition_len;
    slot_ctx->epoch_reward_status.inner.Active.stake_rewards_by_partition = rewards_result->stake_rewards_by_partition.stake_rewards_by_partition;

    /* Initialise the epoch rewards sysvar
     
        https://github.com/anza-xyz/agave/blob/9a7bf72940f4b3cd7fc94f54e005868ce707d53d/runtime/src/bank/partitioned_epoch_rewards/calculation.rs#L78 */
    fd_sysvar_epoch_rewards_init( 
        slot_ctx,
        rewards_result->total_rewards,
        rewards_result->distributed_rewards,
        distribution_starting_block_height,
        rewards_result->stake_rewards_by_partition.stake_rewards_by_partition_len,
        rewards_result->total_points,
        blockhash
     );
}
