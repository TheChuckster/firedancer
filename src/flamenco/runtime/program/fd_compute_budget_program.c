#include "fd_compute_budget_program.h"

static inline int
is_compute_budget_instruction( transaction_ctx_t * ctx, fd_txn_instr_t * instr ) {
  fd_pubkey_t * txn_accs = (fd_pubkey_t *)((uchar *)ctx->txn_raw->raw + ctx->txn_descriptor->acct_addr_off);
  fd_pubkey_t * program_pubkey = &txn_accs[instr->program_id];
  return !memcmp(program_pubkey, ctx->global->solana_compute_budget_program, sizeof(fd_pubkey_t));
}

// No-op as compute budget instructions are processed prior.
int fd_executor_compute_budget_program_execute_instruction_nop( FD_FN_UNUSED instruction_ctx_t ctx ) {
  return FD_EXECUTOR_INSTR_SUCCESS;
}

int fd_executor_compute_budget_program_execute_instructions( transaction_ctx_t * ctx ) {
  uint has_compute_units_limit_update = 0;
  uint has_compute_units_price_update = 0;
  uint has_requested_heap_size = 0;

  uint updated_compute_unit_limit = 0;
  ulong updated_compute_unit_price = 0;
  uint updated_requested_heap_size = 0;
  ulong request_heap_frame_instr_idx = 0;

  uint prioritization_fee_type = FD_COMPUTE_BUDGET_PRIORITIZATION_FEE_TYPE_COMPUTE_UNIT_PRICE;

  for( ulong i = 0; i < ctx->txn_descriptor->instr_cnt; i++ ) {
    fd_txn_instr_t * instr =  &ctx->txn_descriptor->instr[i];
    
    if( !is_compute_budget_instruction(ctx, instr) ) { /* FIXME: is a compute budget instr */
      continue;
    }
    /* Deserialize the ComputeBudgetInstruction enum */
    uchar *      data             = (uchar *)ctx->txn_raw->raw + instr->data_off;

    fd_compute_budget_program_instruction_t instruction;
    fd_bincode_decode_ctx_t decode_ctx = {
      .data = data,
      .dataend = &data[instr->data_sz],
      .allocf = ctx->global->allocf,
      .allocf_arg = ctx->global->allocf_arg
    };

    int ret = fd_compute_budget_program_instruction_decode( &instruction, &decode_ctx );
    if ( ret ) {
      FD_LOG_WARNING(("fd_compute_budget_program_instruction_decode failed"));
      FD_LOG_HEXDUMP_WARNING(("cbi data", data, instr->data_sz));
      return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_DATA;
    }

    switch (instruction.discriminant) {
      case fd_compute_budget_program_instruction_enum_request_units_deprecated: {
        if( has_compute_units_limit_update | has_compute_units_price_update ) {
          /* FIXME: RETURN TXN ERR DUPLICATE TXN! */
          return 1;
        }

        has_compute_units_limit_update = 1;
        has_compute_units_price_update = 1;
        prioritization_fee_type = FD_COMPUTE_BUDGET_PRIORITIZATION_FEE_TYPE_DEPRECATED;
        updated_compute_unit_limit =  instruction.inner.request_units_deprecated.units;
        updated_compute_unit_price =  instruction.inner.request_units_deprecated.additional_fee;

        break;
      }
      case fd_compute_budget_program_instruction_enum_request_heap_frame: {
        if( has_requested_heap_size ) {
          /* FIXME: RETURN TXN ERR DUPLICATE TXN! */
          return 1;
        }
        
        has_requested_heap_size = 1;
        request_heap_frame_instr_idx = i;
        updated_requested_heap_size = instruction.inner.request_heap_frame;
        
        break;
      }
      case fd_compute_budget_program_instruction_enum_set_compute_unit_limit: {
        if( has_compute_units_limit_update ) {
          /* FIXME: RETURN TXN ERR DUPLICATE TXN! */
          return 1;
        }

        has_compute_units_limit_update = 1;
        updated_compute_unit_limit = instruction.inner.set_compute_unit_limit;

        break;
      }
      case fd_compute_budget_program_instruction_enum_set_compute_unit_price: {
        if( has_compute_units_price_update ) {
          /* FIXME: RETURN TXN ERR DUPLICATE TXN! */
          return 1;
        }
        
        has_compute_units_price_update = 1;
        prioritization_fee_type = FD_COMPUTE_BUDGET_PRIORITIZATION_FEE_TYPE_COMPUTE_UNIT_PRICE;
        updated_compute_unit_price = instruction.inner.set_compute_unit_price;

        break;
      }
      default: {
        FD_LOG_WARNING(( "unsupported compute budget program instruction: discriminant: %d", instruction.discriminant ));
      }
    } 
  }

  if( has_requested_heap_size ) {
    if( updated_requested_heap_size > FD_MAX_HEAP_FRAME_BYTES
        || updated_requested_heap_size < FD_MIN_HEAP_FRAME_BYTES
        || updated_requested_heap_size % FD_HEAP_FRAME_BYTES_GRANULARITY != 0 ) {
      /* FIXME: RETURN TXN ERR INVALID INSTR DATA! */
      (void)request_heap_frame_instr_idx;
      return 1;
    }

    ctx->heap_size = updated_requested_heap_size;
  }
  /* TODO: do we need to support default CUs per instr? */

  if( has_compute_units_limit_update ) {
    ctx->compute_unit_limit = (updated_compute_unit_limit < FD_MAX_COMPUTE_UNIT_LIMIT) 
        ? ctx->compute_unit_limit
        : FD_MAX_COMPUTE_UNIT_LIMIT;
  }

  if( has_compute_units_price_update ) {
    ctx->prioritization_fee_type = prioritization_fee_type;
    ctx->compute_unit_price = updated_compute_unit_price;
  }

  return 0;
}
