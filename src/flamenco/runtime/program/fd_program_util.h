#ifndef HEADER_fd_src_flamenco_runtime_native_program_util_h
#define HEADER_fd_src_flamenco_runtime_native_program_util_h

#include "../../fd_flamenco_base.h"
#include "../context/fd_exec_instr_ctx.h"
#include "../context/fd_exec_slot_ctx.h"
#include "../fd_executor.h"
#include "../fd_runtime.h"

#define FD_DEBUG_MODE 0

#ifndef FD_DEBUG_MODE
#define FD_DEBUG( ... ) __VA_ARGS__
#else
#define FD_DEBUG( ... )
#endif

#define FD_PROGRAM_OK 0

FD_PROTOTYPES_BEGIN

/**********************************************************************/
/* mod instruction                                                    */
/**********************************************************************/

// https://github.com/firedancer-io/solana/blob/v1.17/sdk/program/src/instruction.rs#L519
static inline int
fd_instr_checked_add( ulong a, ulong b, ulong * out ) {
  bool cf = __builtin_uaddl_overflow( a, b, out );
  return fd_int_if( cf, FD_EXECUTOR_INSTR_ERR_INSUFFICIENT_FUNDS, FD_PROGRAM_OK );
}

// https://github.com/firedancer-io/solana/blob/v1.17/sdk/program/src/instruction.rs#L519
static inline int FD_FN_UNUSED
fd_instr_checked_sub( ulong a, ulong b, ulong * out ) {
  bool cf = __builtin_usubl_overflow( a, b, out );
  return fd_int_if( cf, FD_EXECUTOR_INSTR_ERR_INSUFFICIENT_FUNDS, FD_PROGRAM_OK );
}

/**********************************************************************/
/* impl BorrowedAccount                                               */
/**********************************************************************/

// https://github.com/firedancer-io/solana/blob/da470eef4652b3b22598a1f379cacfe82bd5928d/sdk/src/transaction_context.rs#L841
static inline int
fd_borrowed_account_checked_add_lamports( fd_borrowed_account_t * self, ulong lamports ) {
  // FIXME suppress warning
  ulong temp;
  int   rc = fd_int_if( __builtin_uaddl_overflow( self->meta->info.lamports, lamports, &temp ),
                      FD_EXECUTOR_INSTR_ERR_ARITHMETIC_OVERFLOW,
                      FD_PROGRAM_OK );
  self->meta->info.lamports = temp;
  return rc;
}

// https://github.com/firedancer-io/solana/blob/da470eef4652b3b22598a1f379cacfe82bd5928d/sdk/src/transaction_context.rs#L851
static inline int
fd_borrowed_account_checked_sub_lamports( fd_borrowed_account_t * self, ulong lamports ) {
  // FIXME suppress warning
  ulong temp;
  int   rc = fd_int_if( __builtin_usubl_overflow( self->meta->info.lamports, lamports, &temp ),
                      FD_EXECUTOR_INSTR_ERR_ARITHMETIC_OVERFLOW,
                      FD_PROGRAM_OK );
  self->meta->info.lamports = temp;
  return rc;
}

/**********************************************************************/
/* impl TransactionContext                                            */
/**********************************************************************/

int
fd_txn_ctx_get_key_of_account_at_index( fd_exec_txn_ctx_t const * self,
                                        uchar                     index_in_transaction,
                                        /* out */ fd_pubkey_t *   pubkey ) {
  if ( FD_UNLIKELY( index_in_transaction >= self->accounts_cnt ) ) {
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;
  }
  *pubkey = self->accounts[index_in_transaction];
  return FD_PROGRAM_OK;
}

/**********************************************************************/
/* impl InstructionContext                                            */
/**********************************************************************/

int
fd_instr_ctx_check_number_of_instruction_accounts( fd_instr_info_t const * self,
                                                   uchar                   expected_at_least ) {
  if ( FD_UNLIKELY( self->acct_cnt < expected_at_least ) ) {
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;
  }
  return FD_PROGRAM_OK;
}

int
fd_instr_ctx_get_index_of_instruction_account_in_transaction(
    fd_instr_info_t const * self,
    uchar                   instruction_account_index,
    /* out */ uchar *       index_in_transaction ) {
  if ( FD_UNLIKELY( instruction_account_index >= self->acct_cnt ) ) {
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;
  }
  *index_in_transaction = self->acct_txn_idxs[instruction_account_index];
  return FD_PROGRAM_OK;
}

int
fd_instr_ctx_try_borrow_account( FD_PARAM_UNUSED fd_instr_info_t const * self,
                                 fd_exec_txn_ctx_t const *               transaction_context,
                                 uchar                                   index_in_transaction,
                                 FD_PARAM_UNUSED uchar                   index_in_instruction,
                                 fd_borrowed_account_t *                 out ) {
  int rc;
  if ( FD_UNLIKELY( index_in_transaction >= transaction_context->accounts_cnt ) ) {
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  out->pubkey = &transaction_context->accounts[index_in_transaction];

  // FIXME implement `const` versions for instructions that don't need write
  // https://github.com/firedancer-io/solana/blob/da470eef4652b3b22598a1f379cacfe82bd5928d/sdk/src/transaction_context.rs#L685-L690
  rc = fd_acc_mgr_modify( transaction_context->acc_mgr,
                          transaction_context->funk_txn,
                          out->pubkey,
                          1,
                          0, // FIXME
                          out );
  switch ( rc ) {
  case FD_ACC_MGR_SUCCESS:
    return FD_PROGRAM_OK;
  case FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT:
    // https://github.com/firedancer-io/solana/blob/da470eef4652b3b22598a1f379cacfe82bd5928d/sdk/src/transaction_context.rs#L637
    return FD_EXECUTOR_INSTR_ERR_MISSING_ACC;
  default:
    // https://github.com/firedancer-io/solana/blob/da470eef4652b3b22598a1f379cacfe82bd5928d/sdk/src/transaction_context.rs#L639
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
}

int
fd_instr_ctx_try_borrow_instruction_account( fd_instr_info_t const *   self,
                                             fd_exec_txn_ctx_t const * transaction_context,
                                             uchar                     instruction_account_index,
                                             fd_borrowed_account_t *   out ) {
  int rc;

  uchar index_in_transaction = FD_TXN_ACCT_ADDR_MAX;
  rc                         = fd_instr_ctx_get_index_of_instruction_account_in_transaction(
      self, instruction_account_index, &index_in_transaction );
  if ( FD_UNLIKELY( rc != FD_PROGRAM_OK ) ) return rc;

  rc = fd_instr_ctx_try_borrow_account( self,
                                        transaction_context,
                                        index_in_transaction,
                                        instruction_account_index, // FIXME add to program accounts?
                                        out );
  if ( FD_UNLIKELY( rc != FD_PROGRAM_OK ) ) return rc;

  return FD_PROGRAM_OK;
}

// https://github.com/firedancer-io/solana/blob/v1.17/sdk/src/transaction_context.rs#L718
int
fd_instr_ctx_is_instruction_account_signer( fd_instr_info_t const * self,
                                            uchar                   instruction_account_index,
                                            bool *                  out ) {
  if ( FD_UNLIKELY( instruction_account_index >= self->acct_cnt ) ) {
    return FD_EXECUTOR_INSTR_ERR_MISSING_ACC;
  }
  *out = fd_instr_acc_is_signer_idx( self, instruction_account_index );
  return FD_PROGRAM_OK;
}

// https://github.com/firedancer-io/solana/blob/v1.17/sdk/src/transaction_context.rs#L718
int
fd_instr_ctx_get_signers( fd_instr_info_t const *   self,
                          fd_exec_txn_ctx_t const * transaction_context,
                          fd_pubkey_t const *       signers[static FD_TXN_SIG_MAX] ) {
  // int   rc;
  uchar j = 0;
  for ( uchar i = 0; i < self->acct_cnt && j < FD_TXN_SIG_MAX; i++ ) {
    if ( FD_UNLIKELY( fd_instr_acc_is_signer_idx( self, i ) ) ) {
      signers[j++] = &transaction_context->accounts[self->acct_txn_idxs[i]];
      // FIXME
      // rc =
      //     get_key_of_account_at_index( transaction_context, self->acct_txn_idxs[i], &signers[j++]
      //     );
      // if ( FD_UNLIKELY( rc != FD_PROGRAM_OK ) ) return rc;
    }
  }
  return FD_PROGRAM_OK;
}

static inline bool
fd_instr_ctx_signers_contains( fd_pubkey_t const * signers[FD_TXN_SIG_MAX],
                               fd_pubkey_t const * pubkey ) {
  for ( ulong i = 0; i < FD_TXN_SIG_MAX && signers[i]; i++ ) {
    if ( FD_UNLIKELY( 0 == memcmp( signers[i], pubkey, sizeof( fd_pubkey_t ) ) ) ) return true;
  }
  return false;
}

/**********************************************************************/
/* mod get_sysvar_with_account_check                                  */
/**********************************************************************/

// https://github.com/firedancer-io/solana/blob/v1.17/program-runtime/src/sysvar_cache.rs#L223
#define FD_SYSVAR_CHECK_SYSVAR_ACCOUNT(                                                                  \
    transaction_context, instruction_context, instruction_account_index, fd_sysvar_id )                  \
  do {                                                                                                   \
    int   rc;                                                                                            \
    uchar index_in_transaction = FD_TXN_ACCT_ADDR_MAX;                                                   \
    rc                         = fd_instr_ctx_get_index_of_instruction_account_in_transaction(           \
        instruction_context, instruction_account_index, &index_in_transaction ); \
    if ( FD_UNLIKELY( rc != FD_PROGRAM_OK ) ) return rc;                                                 \
    if ( FD_UNLIKELY( 0 != memcmp( &transaction_context->accounts[index_in_transaction],                 \
                                   fd_sysvar_id.key,                                                     \
                                   sizeof( fd_pubkey_t ) ) ) ) {                                         \
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;                                                          \
    }                                                                                                    \
  } while ( 0 )

#define FD_SYSVAR_CHECKED_READ( invFD_PROGRAM_OKe_context,                                         \
                                instruction_context,                                               \
                                instruction_account_index,                                         \
                                fd_sysvar_id,                                                      \
                                fd_sysvar_read,                                                    \
                                out )                                                              \
  do {                                                                                             \
    FD_SYSVAR_CHECK_SYSVAR_ACCOUNT( invFD_PROGRAM_OKe_context->txn_ctx,                            \
                                    instruction_context,                                           \
                                    instruction_account_index,                                     \
                                    fd_sysvar_id );                                                \
    int rc = fd_sysvar_read( invFD_PROGRAM_OKe_context->slot_ctx, out );                           \
    if ( FD_UNLIKELY( rc != FD_ACC_MGR_SUCCESS ) )                                                 \
      return FD_EXECUTOR_INSTR_ERR_UNSUPPORTED_SYSVAR;                                             \
    return FD_PROGRAM_OK;                                                                          \
  } while ( 0 )

// https://github.com/firedancer-io/solana/blob/debug-master/program-runtime/src/sysvar_cache.rs#L236
static int
fd_sysvar_clock_checked_read( fd_exec_instr_ctx_t const *       invFD_PROGRAM_OKe_context,
                              fd_instr_info_t const *           instruction_context,
                              uchar                             instruction_account_index,
                              /* out */ fd_sol_sysvar_clock_t * clock ) {
  FD_SYSVAR_CHECKED_READ( invFD_PROGRAM_OKe_context,
                          instruction_context,
                          instruction_account_index,
                          fd_sysvar_clock_id,
                          fd_sysvar_clock_read,
                          clock );
}

// https://github.com/firedancer-io/solana/blob/debug-master/program-runtime/src/sysvar_cache.rs#L249
static int
fd_sysvar_rent_checked_read( fd_exec_instr_ctx_t const * invFD_PROGRAM_OKe_context,
                             fd_instr_info_t const *     instruction_context,
                             uchar                       instruction_account_index,
                             /* out */ fd_rent_t *       rent ) {
  FD_SYSVAR_CHECKED_READ( invFD_PROGRAM_OKe_context,
                          instruction_context,
                          instruction_account_index,
                          fd_sysvar_rent_id,
                          fd_sysvar_rent_read,
                          rent );
}

// https://github.com/firedancer-io/solana/blob/debug-master/program-runtime/src/sysvar_cache.rs#L289
static int
fd_sysvar_stake_history_checked_read( fd_exec_instr_ctx_t const *    invFD_PROGRAM_OKe_context,
                                      fd_instr_info_t const *        instruction_context,
                                      uchar                          instruction_account_index,
                                      /* out */ fd_stake_history_t * stake_history ) {
  FD_SYSVAR_CHECKED_READ( invFD_PROGRAM_OKe_context,
                          instruction_context,
                          instruction_account_index,
                          fd_sysvar_stake_history_id,
                          fd_sysvar_stake_history_read,
                          stake_history );
}

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_runtime_native_program_util_h */
