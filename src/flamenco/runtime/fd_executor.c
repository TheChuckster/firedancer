#include "fd_executor.h"
#include "fd_acc_mgr.h"
#include "fd_hashes.h"
#include "fd_runtime.h"

#include "../../util/rng/fd_rng.h"
#include "../nanopb/pb_encode.h"
#include "../trace/fd_txntrace.h"
#include "program/fd_bpf_deprecated_loader_program.h"
#include "program/fd_bpf_loader_program.h"
#include "program/fd_bpf_loader_v4_program.h"
#include "program/fd_bpf_upgradeable_loader_program.h"
#include "program/fd_compute_budget_program.h"
#include "program/fd_config_program.h"
#include "program/fd_ed25519_program.h"
#include "program/fd_secp256k1_program.h"
#include "program/fd_stake_program.h"
#include "program/fd_system_program.h"
#include "program/fd_vote_program.h"

#include "../../ballet/base58/fd_base58.h"

#include <errno.h>
#include <stdio.h>   /* snprintf(3) */
#include <fcntl.h>   /* openat(2) */
#include <unistd.h>  /* write(3) */
#include <time.h>

/* Look up a native program given it's pubkey key */
execute_instruction_func_t
fd_executor_lookup_native_program( fd_pubkey_t const * pubkey ) {
  /* TODO: replace with proper lookup table */
  if ( !memcmp( pubkey, fd_solana_vote_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_vote_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_system_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_system_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_config_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_config_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_stake_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_stake_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_ed25519_sig_verify_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_ed25519_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_keccak_secp_256k_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_secp256k1_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_bpf_loader_upgradeable_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_bpf_upgradeable_loader_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_bpf_loader_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_bpf_loader_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_bpf_loader_deprecated_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_bpf_deprecated_loader_program_execute_instruction;
  } else if ( !memcmp( pubkey, fd_solana_compute_budget_program_id.key, sizeof( fd_pubkey_t ) ) ) {
    return fd_executor_compute_budget_program_execute_instruction_nop;
  } else if( !memcmp( pubkey, fd_solana_bpf_loader_v4_program_id.key, sizeof(fd_pubkey_t) ) ) {
    return fd_executor_bpf_loader_v4_program_execute_instruction;
  } else {
    return NULL; /* FIXME */
  }
}

int
fd_executor_lookup_program( fd_exec_slot_ctx_t * slot_ctx,
                            fd_pubkey_t const * pubkey ) {
  if( fd_executor_bpf_upgradeable_loader_program_is_executable_program_account( slot_ctx, pubkey )==0 ) {
    return 0;
  }

  return -1;
}

// TODO: handle error codes
void
fd_executor_setup_accessed_accounts_for_txn( fd_exec_txn_ctx_t * txn_ctx, fd_rawtxn_b_t const * txn_raw, uint * use_sysvar_instructions ) {
  fd_pubkey_t *tx_accs   = (fd_pubkey_t *)((uchar *)txn_raw->raw + txn_ctx->txn_descriptor->acct_addr_off);

  for( ulong i = 0; i < txn_ctx->txn_descriptor->acct_addr_cnt; i++ ) {
    txn_ctx->accounts[i] = tx_accs[i];
    if ( FD_UNLIKELY(
        *use_sysvar_instructions
        || memcmp(&txn_ctx->accounts[i], fd_sysvar_instructions_id.key, sizeof(fd_pubkey_t))==0
      ) ) {
      *use_sysvar_instructions = 1;
    }
  }
  txn_ctx->accounts_cnt += (uchar) txn_ctx->txn_descriptor->acct_addr_cnt;

  if( txn_ctx->txn_descriptor->transaction_version == FD_TXN_V0 ) {
    fd_txn_acct_addr_lut_t * addr_luts = fd_txn_get_address_tables( txn_ctx->txn_descriptor );
    for (ulong i = 0; i < txn_ctx->txn_descriptor->addr_table_lookup_cnt; i++) {
      fd_txn_acct_addr_lut_t * addr_lut = &addr_luts[i];
      fd_pubkey_t const * addr_lut_acc = (fd_pubkey_t *)((uchar *)txn_raw->raw + addr_lut->addr_off);

      FD_BORROWED_ACCOUNT_DECL(addr_lut_rec);
      int err = fd_acc_mgr_view(txn_ctx->acc_mgr, txn_ctx->funk_txn, (fd_pubkey_t *) addr_lut_acc, addr_lut_rec);
      if( FD_UNLIKELY( err != FD_ACC_MGR_SUCCESS ) ) {
        // TODO: return txn err code
        FD_LOG_ERR(( "addr lut not found" ));
      }
      FD_LOG_WARNING(( "LUT ACC: idx: %lu, acc: %32J, meta.dlen; %lu", i, addr_lut_acc, addr_lut_rec->const_meta->dlen ));

      fd_address_lookup_table_state_t addr_lookup_table_state;
      fd_bincode_decode_ctx_t decode_ctx = {
        .data = addr_lut_rec->const_data,
        .dataend = &addr_lut_rec->const_data[56], // TODO macro const.
        .valloc  = txn_ctx->valloc,
      };
      if (fd_address_lookup_table_state_decode( &addr_lookup_table_state, &decode_ctx )) {
        FD_LOG_ERR(("fd_address_lookup_table_state_decode failed"));
      }
      if (addr_lookup_table_state.discriminant != fd_address_lookup_table_state_enum_lookup_table) {
        FD_LOG_ERR(("addr lut is uninit"));
      }

      fd_pubkey_t * lookup_addrs = (fd_pubkey_t *)&addr_lut_rec->const_data[56];
      uchar * writable_lut_idxs = (uchar *)txn_raw->raw + addr_lut->writable_off;
      for (ulong j = 0; j < addr_lut->writable_cnt; j++) {
        FD_LOG_WARNING(( "LUT ACC WRITABLE: idx: %3lu, acc: %32J, lut_idx: %3lu, acct_idx: %3lu, %32J", i, addr_lut_acc, j, writable_lut_idxs[j], &lookup_addrs[writable_lut_idxs[j]] ));
        txn_ctx->accounts[txn_ctx->accounts_cnt++] = lookup_addrs[writable_lut_idxs[j]];
        if ( FD_UNLIKELY(
            *use_sysvar_instructions
            || memcmp( &lookup_addrs[writable_lut_idxs[j]], fd_sysvar_instructions_id.key, sizeof(fd_pubkey_t))==0
          ) ) {
          *use_sysvar_instructions = 1;
        }
      }
    }

    for (ulong i = 0; i < txn_ctx->txn_descriptor->addr_table_lookup_cnt; i++) {
      fd_txn_acct_addr_lut_t * addr_lut = &addr_luts[i];
      fd_pubkey_t const * addr_lut_acc = (fd_pubkey_t *)((uchar *)txn_raw->raw + addr_lut->addr_off);

      FD_BORROWED_ACCOUNT_DECL(addr_lut_rec);
      int err = fd_acc_mgr_view(txn_ctx->acc_mgr, txn_ctx->funk_txn, (fd_pubkey_t *) addr_lut_acc, addr_lut_rec);
      if( FD_UNLIKELY( err != FD_ACC_MGR_SUCCESS ) ) {
        // TODO: return txn err code
        FD_LOG_ERR(( "addr lut not found" ));
      }

      fd_address_lookup_table_state_t addr_lookup_table_state;
      fd_bincode_decode_ctx_t decode_ctx = {
        .data = addr_lut_rec->const_data,
        .dataend = &addr_lut_rec->const_data[56], // TODO macro const.
        .valloc  = txn_ctx->valloc,
      };
      if (fd_address_lookup_table_state_decode( &addr_lookup_table_state, &decode_ctx )) {
        FD_LOG_ERR(("fd_address_lookup_table_state_decode failed"));
      }
      if (addr_lookup_table_state.discriminant != fd_address_lookup_table_state_enum_lookup_table) {
        FD_LOG_ERR(("addr lut is uninit"));
      }

      fd_pubkey_t * lookup_addrs = (fd_pubkey_t *)&addr_lut_rec->const_data[56];
      uchar * readonly_lut_idxs = (uchar *)txn_raw->raw + addr_lut->readonly_off;
      for (ulong j = 0; j < addr_lut->readonly_cnt; j++) {
        FD_LOG_WARNING(( "LUT ACC READONLY: idx: %3lu, acc: %32J, lut_idx: %3lu, acct_idx: %3lu, %32J", i, addr_lut_acc, j, readonly_lut_idxs[j], &lookup_addrs[readonly_lut_idxs[j]] ));
        txn_ctx->accounts[txn_ctx->accounts_cnt++] = lookup_addrs[readonly_lut_idxs[j]];
        if ( FD_UNLIKELY(
            *use_sysvar_instructions
            || memcmp( &lookup_addrs[readonly_lut_idxs[j]], fd_sysvar_instructions_id.key, sizeof(fd_pubkey_t))==0
          ) ) {
          *use_sysvar_instructions = 1;
        }
      }
    }
  }
}

/* todo rent exempt check */
static void
fd_set_exempt_rent_epoch_max( fd_exec_slot_ctx_t * slot_ctx,
                              void const *         addr ) {

  FD_BORROWED_ACCOUNT_DECL(rec);

  int err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, (fd_pubkey_t const *)addr, rec);
  if( FD_UNLIKELY( err==FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT ) )
    return;
  FD_TEST( err==FD_ACC_MGR_SUCCESS );

  if( rec->const_meta->info.lamports < fd_rent_exempt_minimum_balance( slot_ctx, rec->const_meta->dlen ) ) return;
  if( rec->const_meta->info.rent_epoch == ULONG_MAX ) return;

  err = fd_acc_mgr_modify( slot_ctx->acc_mgr, slot_ctx->funk_txn, (fd_pubkey_t const *)addr, 0, 0, rec);
  FD_TEST( err==FD_ACC_MGR_SUCCESS );

  rec->meta->info.rent_epoch = ULONG_MAX;
}

static int
fd_executor_collect_fee( fd_exec_slot_ctx_t * slot_ctx,
                         fd_pubkey_t const *  account,
                         ulong                fee ) {

  FD_BORROWED_ACCOUNT_DECL(rec);

  int err = fd_acc_mgr_modify( slot_ctx->acc_mgr, slot_ctx->funk_txn, account, 0, 0UL, rec);
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(( "fd_acc_mgr_modify(%32J) failed (%d-%s)", account->uc, err, fd_acc_mgr_strerror( err ) ));
    // TODO: The fee payer does not seem to exist?!  what now?
    return -1;
  }

  if (fee > rec->meta->info.lamports) {
    // TODO: Not enough lamports to pay for this txn...
    //
    // (Should this be lamps + whatever is required to keep the payer rent exempt?)
    FD_LOG_WARNING(( "Not enough lamps" ));
    return -1;
  }

  FD_LOG_DEBUG(( "fd_execute_txn: global->collected: %ld->%ld (%ld)", slot_ctx->bank.collected_fees, slot_ctx->bank.collected_fees + fee, fee));
  FD_LOG_DEBUG(( "calling set_lamports to charge the fee %lu", fee));

  // TODO: I BELIEVE we charge for the fee BEFORE we create the funk_txn fork
  // since we collect reguardless of the success of the txn execution...
  rec->meta->info.lamports -= fee;
  slot_ctx->bank.collected_fees += fee;

  /* todo rent exempt check */
  if( FD_FEATURE_ACTIVE( slot_ctx, set_exempt_rent_epoch_max ) )
    rec->meta->info.rent_epoch = ULONG_MAX;
  return 0;
}

int
fd_execute_instr( fd_instr_info_t * instr, fd_exec_txn_ctx_t * txn_ctx ) {
  fd_pubkey_t const * txn_accs = txn_ctx->accounts;

  fd_exec_instr_ctx_t * ctx = &txn_ctx->instr_stack[txn_ctx->instr_stack_sz++];
  ctx->instr = instr;
  ctx->txn_ctx = txn_ctx;
  ctx->epoch_ctx = txn_ctx->epoch_ctx;
  ctx->slot_ctx = txn_ctx->slot_ctx;
  ctx->valloc = txn_ctx->valloc;
  ctx->acc_mgr = txn_ctx->acc_mgr;
  ctx->funk_txn = txn_ctx->funk_txn;

  // defense in depth
  if (instr->program_id >= txn_ctx->txn_descriptor->acct_addr_cnt + txn_ctx->txn_descriptor->addr_table_adtl_cnt) {
    FD_LOG_WARNING(( "INVALID PROGRAM ID, RUNTIME BUG!!!" ));
    int exec_result = FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;
    txn_ctx->instr_stack_sz--;

    //FD_LOG_WARNING(( "instruction executed unsuccessfully: error code %d", exec_result ));
    return exec_result;
  }

  /* TODO: allow instructions to be failed, and the transaction to be reverted */
  fd_pubkey_t const * program_id_acc = &txn_accs[instr->program_id];

  execute_instruction_func_t exec_instr_func = fd_executor_lookup_native_program( program_id_acc );

  int exec_result = FD_EXECUTOR_INSTR_SUCCESS;
  if (exec_instr_func != NULL) {
    exec_result = exec_instr_func( *ctx );

  } else {
    if (fd_executor_lookup_program( ctx->slot_ctx, program_id_acc ) == 0 ) {
      FD_LOG_NOTICE(( "found BPF upgradeable executable program account - program id: %32J", program_id_acc ));

      exec_result = fd_executor_bpf_upgradeable_loader_program_execute_program_instruction(*ctx);

    } else if ( fd_executor_bpf_loader_program_is_executable_program_account( ctx->slot_ctx, program_id_acc ) == 0 ) {
      FD_LOG_NOTICE(( "found BPF v2 executable program account - program id: %32J", program_id_acc ));

      exec_result = fd_executor_bpf_loader_program_execute_program_instruction(*ctx);

    } else {
      FD_LOG_WARNING(( "did not find native or BPF executable program account - program id: %32J", program_id_acc ));

      exec_result = FD_EXECUTOR_INSTR_ERR_GENERIC_ERR;
    }
  }

//  if ( FD_UNLIKELY( exec_result != FD_EXECUTOR_INSTR_SUCCESS ) ) {
//    FD_LOG_WARNING(( "instruction executed unsuccessfully: error code %d, custom err: %d, program id: %32J", exec_result, txn_ctx->custom_err, program_id_acc ));
//  }

  txn_ctx->instr_stack_sz--;

  /* TODO: sanity before/after checks: total lamports unchanged etc */
  return exec_result;
}

/* fd_executor_dump_txntrace creates a new file in the trace dir
   containing the given binary blob. */

static void
fd_executor_dump_txntrace( fd_exec_slot_ctx_t *         slot_ctx,
                           fd_ed25519_sig_t const *     sig,
                           fd_soltrace_TxnTrace const * trace ) {

  if( FD_UNLIKELY( !fd_scratch_push_is_safe() ) ) return;
  FD_SCRATCH_SCOPED_FRAME;
  if( FD_UNLIKELY( !fd_scratch_prepare_is_safe( 1UL ) ) ) return;

  /* Serialize to open-ended scratch frame */
  ulong   data_bufsz = fd_scratch_free();
  uchar * data       = fd_scratch_prepare( 1UL );
  pb_ostream_t ostream = pb_ostream_from_buffer( data, data_bufsz );
  if( FD_UNLIKELY( !pb_encode( &ostream, fd_soltrace_TxnTrace_fields, trace ) ) ) {
    FD_LOG_ERR(( "pb_encode of trace %p failed (%lu bufsz, %lu written): %s",
                     (void *)trace, data_bufsz, ostream.bytes_written, PB_GET_ERROR( &ostream ) ));
    fd_scratch_cancel();
    return;
  }
  ulong data_sz = ostream.bytes_written;
  fd_scratch_publish( data+data_sz );

  /* Formulate file name */
  char filename[ 128UL ];
  /* 118 (20+1+88+9) chars + null terminator */
  snprintf( filename, sizeof(filename), "%lu-%64J.txntrace",
            slot_ctx->bank.slot, sig );

  /* Create file */
  int dump_fd = openat( slot_ctx->trace_dirfd, filename, O_WRONLY|O_CREAT|O_TRUNC, 0666 );
  if( FD_UNLIKELY( dump_fd<0 ) ) {
    FD_LOG_WARNING(( "openat(%d, %s) failed (%d-%s)",
                     slot_ctx->trace_dirfd, filename, errno, fd_io_strerror( errno ) ));
    return;
  }

  /* Write file */
  long nbytes = write( dump_fd, data, data_sz );
  if( FD_UNLIKELY( nbytes!=(long)data_sz ) ) {
    FD_LOG_WARNING(( "write to %s failed (%d-%s)",
                     filename, errno, fd_io_strerror( errno ) ));
    close( dump_fd );
    unlinkat( slot_ctx->trace_dirfd, filename, 0 );
    return;
  }
  close( dump_fd );
}

void
fd_executor_setup_borrowed_accounts_for_txn( fd_exec_txn_ctx_t * txn_ctx ) {
  for( ulong i = 0; i < txn_ctx->accounts_cnt; i++ ) {
    fd_pubkey_t * acc = &txn_ctx->accounts[i];
    fd_borrowed_account_t * borrowed_account = fd_borrowed_account_init( &txn_ctx->borrowed_accounts[i] );
    if( fd_txn_account_is_writable_idx( txn_ctx->txn_descriptor, (int)i ) ) {
      int err = fd_acc_mgr_modify( txn_ctx->acc_mgr, txn_ctx->funk_txn, acc, 1, 0UL, borrowed_account);

      if( FD_UNLIKELY( err ) ) {
      //   FD_LOG_WARNING(( "fd_acc_mgr_modify(%32J) failed (%d-%s)", acc->uc, err, fd_acc_mgr_strerror( err ) ));
      }
    } else {
      int err = fd_acc_mgr_view( txn_ctx->acc_mgr, txn_ctx->funk_txn, acc, borrowed_account );

      if( FD_UNLIKELY( err ) ) {
      //   FD_LOG_WARNING(( "fd_acc_mgr_view(%32J) failed (%d-%s)", acc->uc, err, fd_acc_mgr_strerror( err ) ));
      }
    }
  }
}

static void
fd_executor_retrace( fd_exec_slot_ctx_t *          slot_ctx,
                     fd_soltrace_TxnInput const * trace_pre,
                     fd_soltrace_TxnDiff  const * trace_post0 ) {

  FD_SCRATCH_SCOPED_FRAME;

  fd_soltrace_TxnDiff  _trace_post1[1];
  fd_soltrace_TxnDiff * trace_post1 =
      fd_txntrace_replay( _trace_post1, trace_pre, slot_ctx->local_wksp );

  if( FD_UNLIKELY( !trace_post1 ) )
    FD_LOG_ERR(( "fd_txntrace_replay failed" ));

  if( FD_UNLIKELY( !fd_txntrace_diff( trace_post0, trace_post1 ) ) )
    FD_LOG_ERR(( "fd_txntrace_replay returned incorrect trace:\n%s",
                 fd_txntrace_diff_cstr() ));
}

int
fd_execute_txn( fd_exec_slot_ctx_t *  slot_ctx,
                fd_txn_t *            txn_descriptor,
                fd_rawtxn_b_t const * txn_raw ) {
  FD_SCRATCH_SCOPED_FRAME;

  fd_pubkey_t * tx_accs   = (fd_pubkey_t *)((uchar *)txn_raw->raw + txn_descriptor->acct_addr_off);

  /* Trace transaction input */
  fd_soltrace_TxnInput  _trace_pre[1];
  fd_soltrace_TxnInput * trace_pre = NULL;
  if( FD_UNLIKELY( slot_ctx->trace_mode ) ) {
    trace_pre = fd_txntrace_capture_pre( _trace_pre, slot_ctx, txn_descriptor, txn_raw->raw );
    if( FD_UNLIKELY( !trace_pre ) )
      FD_LOG_WARNING(( "fd_txntrace_capture_pre failed (out of scratch memory?)" ));
  }

  fd_transaction_return_data_t return_data = {0};
  return_data.data = (uchar*)fd_valloc_malloc(slot_ctx->valloc, 1, 1024);

  fd_exec_txn_ctx_t txn_ctx = {
    .epoch_ctx          = slot_ctx->epoch_ctx,
    .slot_ctx           = slot_ctx,
    .funk_txn           = slot_ctx->funk_txn,
    .acc_mgr            = slot_ctx->acc_mgr,
    .valloc             = slot_ctx->valloc,
    .compute_unit_limit = 200000,
    .compute_unit_price = 0,
    .compute_meter      = 200000,
    .prioritization_fee_type = FD_COMPUTE_BUDGET_PRIORITIZATION_FEE_TYPE_DEPRECATED,
    .txn_descriptor     = txn_descriptor,
    ._txn_raw           = txn_raw,
    .custom_err         = UINT_MAX,
    .instr_stack_sz     = 0,
    .accounts_cnt       = 0,
    .return_data        = return_data,
  };

  uint use_sysvar_instructions = 0;
  fd_executor_setup_accessed_accounts_for_txn( &txn_ctx, txn_raw, &use_sysvar_instructions );
  int compute_budget_status = fd_executor_compute_budget_program_execute_instructions( &txn_ctx, txn_raw );
  (void)compute_budget_status;

  ulong fee = fd_runtime_calculate_fee( &txn_ctx, txn_descriptor, txn_raw, FD_FEATURE_ACTIVE( slot_ctx, remove_congestion_multiplier_from_fee_calculation ), FD_FEATURE_ACTIVE( slot_ctx, include_loaded_accounts_data_size_in_fee_calculation ));
  if( fd_executor_collect_fee( slot_ctx, &tx_accs[0], fee ) ) {
    return -1;
  }

  // TODO: we are just assuming the fee payer is account 0... FIX this..

  /// Returns true if the account at the specified index is not invoked as a
  /// program or, if invoked, is passed to a program.
  //    pub fn is_non_loader_key(&self, key_index: usize) -> bool {
  //      !self.is_invoked(key_index) || self.is_key_passed_to_program(key_index)
  //    }

  //    let fee_payer = (0..message.account_keys().len()).find_map(|i| {
  //            if let Some((k, a)) = &accounts.get(i) {
  //                if message.is_non_loader_key(i) {
  //                    return Some((k, a));
  //                }
  //            }
  //      }

  /* TODO: track compute budget used within execution */
  /* TODO: store stack of instructions to detect reentrancy */

  /* TODO: execute within a transaction context, which can be reverted */

  fd_funk_txn_t* parent_txn = slot_ctx->funk_txn;
  fd_funk_txn_xid_t xid;
  xid.ul[0] = fd_rng_ulong( slot_ctx->rng );
  xid.ul[1] = fd_rng_ulong( slot_ctx->rng );
  xid.ul[2] = fd_rng_ulong( slot_ctx->rng );
  xid.ul[3] = fd_rng_ulong( slot_ctx->rng );
  fd_funk_txn_t * txn = fd_funk_txn_prepare( slot_ctx->acc_mgr->funk, parent_txn, &xid, 1 );
  // TODO: bad for multi-threading...
  txn_ctx.funk_txn = txn;

  fd_executor_setup_borrowed_accounts_for_txn( &txn_ctx );
  /* Update rent exempt on writable accounts if feature activated
    TODO this should probably not run on executable accounts
        Also iterate over LUT accounts */
  if( FD_FEATURE_ACTIVE( slot_ctx, set_exempt_rent_epoch_max ) ) {
    fd_txn_acct_iter_t ctrl;
    for( ulong i=fd_txn_acct_iter_init( txn_descriptor, FD_TXN_ACCT_CAT_WRITABLE, &ctrl );
          i<fd_txn_acct_iter_end(); i=fd_txn_acct_iter_next( i, &ctrl ) ) {
      if( i==0 ) continue;
      fd_set_exempt_rent_epoch_max( slot_ctx, &tx_accs[i] );
    }
  }

  fd_instr_info_t instrs[txn_descriptor->instr_cnt];
  for ( ushort i = 0; i < txn_descriptor->instr_cnt; ++i ) {
    fd_txn_instr_t *  txn_instr = &txn_descriptor->instr[i];
    fd_convert_txn_instr_to_instr( txn_descriptor, txn_raw, txn_instr, txn_ctx.accounts, txn_ctx.borrowed_accounts, &instrs[i] );
  }

  int ret = 0L;
  if ( FD_UNLIKELY( use_sysvar_instructions ) ) {
    fd_sysvar_instructions_serialize_account( &txn_ctx, instrs, txn_descriptor->instr_cnt );
    if( ret != FD_ACC_MGR_SUCCESS ) {
      FD_LOG_WARNING(( "SYSVAR INSTRS FAILED TO SERIALIZE!" ));
      fd_funk_txn_cancel(txn_ctx.acc_mgr->funk, txn, 0);
      txn_ctx.funk_txn = parent_txn;
      return -1;
    }
  }
  uint unknown_accounts[128];
  for( ulong i = 0; i < txn_ctx.accounts_cnt; i++ ) {
    unknown_accounts[i] = 0;
    if (fd_txn_is_writable(txn_ctx.txn_descriptor, (int)i)) {
      FD_BORROWED_ACCOUNT_DECL(writable_new);
      int err = fd_acc_mgr_view(txn_ctx.acc_mgr, txn_ctx.funk_txn, &txn_ctx.accounts[i], writable_new);
      if( FD_UNLIKELY( err != FD_ACC_MGR_SUCCESS ) ) {
        unknown_accounts[i] = 1;
      }
    }
  }
  for ( ushort i = 0; i < txn_descriptor->instr_cnt; ++i ) {
    if ( FD_UNLIKELY( use_sysvar_instructions ) ) {
      ret = fd_sysvar_instructions_update_current_instr_idx( &txn_ctx, i );
      if( ret != FD_ACC_MGR_SUCCESS ) {
        FD_LOG_WARNING(( "SYSVAR INSTRS FAILED TO UPDATE CURRENT INSTR IDX!" ));
        fd_funk_txn_cancel(txn_ctx.acc_mgr->funk, txn, 0);
        txn_ctx.funk_txn = parent_txn;
        return -1;
      }
    }

    int exec_result = fd_execute_instr( &instrs[i], &txn_ctx );
    if( exec_result != FD_EXECUTOR_INSTR_SUCCESS ) {
      FD_LOG_DEBUG(( "fd_execute_instr failed (%d)", exec_result ));
      fd_funk_txn_cancel(txn_ctx.acc_mgr->funk, txn, 0);
      txn_ctx.funk_txn = parent_txn;
      return -1;
    }

    /* TODO: sanity before/after checks: total lamports unchanged etc */
  }

  for( ulong i = 0; i < txn_ctx.accounts_cnt; i++ ) {
    if (unknown_accounts[i]) {
      FD_BORROWED_ACCOUNT_DECL(writable_new);
      int err = fd_acc_mgr_modify(txn_ctx.acc_mgr, txn_ctx.funk_txn, &txn_ctx.accounts[i], 1, 0, writable_new);
      if( FD_UNLIKELY( err != FD_ACC_MGR_SUCCESS ) ) {
        FD_LOG_ERR(( "account mgr modify failed for %32J", txn_ctx.accounts[i].uc ));
      }
      writable_new->meta->slot = txn_ctx.slot_ctx->bank.slot;
      memset(writable_new->meta->hash, 0xFF, sizeof(fd_hash_t));
    }
  }

  fd_valloc_free(txn_ctx.valloc, txn_ctx.return_data.data);

  if ( FD_UNLIKELY( use_sysvar_instructions ) ) {
    ret = fd_sysvar_instructions_cleanup_account( &txn_ctx );
    if( ret != FD_ACC_MGR_SUCCESS ) {
      FD_LOG_WARNING(( "SYSVAR INSTRS FAILED TO CLEANUP!" ));
      return -1;
    }
  }

  /* Export trace to Protobuf file */
  if( FD_UNLIKELY( trace_pre ) ) {
    fd_soltrace_TxnDiff  _trace_post[1];
    fd_soltrace_TxnDiff * trace_post = fd_txntrace_capture_post( _trace_post, slot_ctx, trace_pre );
    if( trace_post ) {
      if( slot_ctx->trace_mode & FD_RUNTIME_TRACE_SAVE ) {
        fd_soltrace_TxnTrace trace = {
          .input = trace_pre,
          .diff  = trace_post
        };
        fd_ed25519_sig_t const * sig0 = &fd_txn_get_signatures( txn_descriptor, txn_raw->raw )[0];
        fd_executor_dump_txntrace( slot_ctx, sig0, &trace );
      }
      if( slot_ctx->trace_mode & FD_RUNTIME_TRACE_REPLAY ) {
        fd_executor_retrace( slot_ctx, trace_pre, trace_post );
      }
    }
  }

  fd_funk_txn_merge(slot_ctx->acc_mgr->funk, txn, 0);
  slot_ctx->funk_txn = parent_txn;
  return 0;
}
