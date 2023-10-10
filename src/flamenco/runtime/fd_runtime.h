#ifndef HEADER_fd_src_flamenco_runtime_fd_runtime_h
#define HEADER_fd_src_flamenco_runtime_fd_runtime_h

#include "fd_rocksdb.h"
#include "fd_acc_mgr.h"
#include "fd_executor.h"
#include "../features/fd_features.h"
#include "fd_rent_lists.h"
#include "../../ballet/poh/fd_poh.h"
#include "program/fd_builtin_programs.h"
#include "../leaders/fd_leaders.h"
#include "sysvar/fd_sysvar.h"
#include "context/fd_exec_slot_ctx.h"
#include "context/fd_exec_txn_ctx.h"
#include "fd_instr_info.h"

#define FD_RUNTIME_EXECUTE_SUCCESS                               ( 0 )  /* Slot executed successfully */
#define FD_RUNTIME_EXECUTE_GENERIC_ERR                          ( -1 ) /* The Slot execute returned an error */
#define MAX_PERMITTED_DATA_LENGTH ( 10 * 1024 * 1024 )

#define FD_RUNTIME_TRACE_NONE   (0)
#define FD_RUNTIME_TRACE_SAVE   (1)
#define FD_RUNTIME_TRACE_REPLAY (2)

#define FD_FEATURE_ACTIVE(_slot_ctx, _feature_name)  (_slot_ctx->bank.slot >= _slot_ctx->epoch_ctx->features. _feature_name)

#define FD_ACC_MGR_KEY_TYPE ((uchar)0)
#define FD_BLOCK_KEY_TYPE ((uchar)1)
#define FD_BLOCK_META_KEY_TYPE ((uchar)2)

/* FD_BLOCK_BANKS_TYPE stores fd_firedancer_banks_t bincode encoded */
#define FD_BLOCK_BANKS_TYPE ((uchar)3)

/* FD_BANK_HASH_TYPE stores the bank hash of each slot */
#define FD_BANK_HASH_TYPE ((uchar)4)

/* FD_BLOCK_TXNSTATUS_TYPE stores the transaction metadata for a block */
#define FD_BLOCK_TXNSTATUS_TYPE ((uchar)5)

FD_PROTOTYPES_BEGIN

ulong
fd_runtime_lamports_per_signature( fd_firedancer_banks_t const * bank );

ulong 
fd_runtime_txn_lamports_per_signature( fd_exec_txn_ctx_t * txn_ctx, 
                                       fd_txn_t * txn_descriptor,
                                       fd_rawtxn_b_t const * txn_raw );

void
fd_runtime_init_bank_from_genesis( fd_exec_slot_ctx_t * slot_ctx,
                                   fd_genesis_solana_t * genesis_block,
                                   fd_hash_t const * genesis_hash );

void 
fd_runtime_init_program( fd_exec_slot_ctx_t * slot_ctx );

int
fd_runtime_block_execute( fd_exec_slot_ctx_t * slot_ctx, 
                          fd_slot_meta_t *m, 
                          const void* block,
                          ulong blocklen );

int
fd_runtime_block_verify( fd_exec_slot_ctx_t * slot_ctx,
  fd_slot_meta_t *m, const void* block, ulong blocklen );

int fd_runtime_block_verify_tpool( fd_exec_slot_ctx_t * slot_ctx, 
                                   fd_slot_meta_t *m,
                                  const void* block, ulong blocklen, fd_tpool_t * tpool, ulong max_workers );

int fd_runtime_block_eval( fd_exec_slot_ctx_t *slot_ctx,
                           fd_slot_meta_t * m,
                           const void * block, 
                           ulong blocklen );

ulong
fd_runtime_calculate_fee ( fd_exec_txn_ctx_t * txn_ctx, 
                           fd_txn_t * txn_descriptor,
                           fd_rawtxn_b_t const * txn_raw );

void 
fd_runtime_freeze( fd_exec_slot_ctx_t * slot_ctx );

ulong
fd_runtime_lamports_per_signature_for_blockhash( fd_exec_slot_ctx_t const * slot_ctx, 
                                                 fd_hash_t * blockhash );

fd_funk_rec_key_t 
fd_runtime_block_key( ulong slot );

fd_funk_rec_key_t 
fd_runtime_block_meta_key( ulong slot );

fd_funk_rec_key_t
fd_runtime_banks_key( void );

fd_funk_rec_key_t
fd_runtime_bank_hash_key( ulong slot );

static inline fd_funk_rec_key_t
fd_runtime_block_txnstatus_key( ulong slot ) {
  fd_funk_rec_key_t id = {0};
  id.ul[ 0 ] = slot;
  id.c[ FD_FUNK_REC_KEY_FOOTPRINT - 1 ] = FD_BLOCK_TXNSTATUS_TYPE;
  return id;
}

int
fd_runtime_save_banks( fd_exec_slot_ctx_t * slot_ctx );

int
fd_global_import_solana_manifest( fd_exec_slot_ctx_t * slot_ctx, 
                                  fd_solana_manifest_t * manifest);

/* fd_features_restore loads all known feature accounts from the
   accounts database.  This is used when initializing bank from a
   snapshot. */

void
fd_features_restore( fd_exec_slot_ctx_t * slot_ctx );

static inline ulong 
fd_rent_exempt( fd_firedancer_banks_t const * bank, 
                ulong                         sz ) {
  return (sz + 128) * ((ulong) ((double)bank->rent.lamports_per_uint8_year * bank->rent.exemption_threshold));
}

void
fd_process_new_epoch( fd_exec_slot_ctx_t * slot_ctx,
                      ulong parent_epoch );

void
fd_runtime_update_leaders( fd_exec_slot_ctx_t * slot_ctx, ulong slot);

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_runtime_fd_runtime_h */
