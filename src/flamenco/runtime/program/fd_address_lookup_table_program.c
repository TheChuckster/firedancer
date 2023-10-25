#include "fd_program_util.h"
#include "fd_zk_token_proof_program.h"
#include "../fd_executor.h"
#include "../context/fd_exec_txn_ctx.h"
#include "../fd_acc_mgr.h"
#include "../fd_pubkey_utils.h"
#include "../fd_account.h"
#include "../sysvar/fd_sysvar_clock.h"
#include <string.h>

struct fd_addrlut {
  fd_address_lookup_table_state_t state;

  fd_pubkey_t const * addr;  /* points into account data */
  ulong               addr_cnt;
};

typedef struct fd_addrlut fd_addrlut_t;

#define FD_ADDRLUT_META_SZ       (56UL)
#define FD_ADDRLUT_MAX_ADDR_CNT (256UL)

static fd_addrlut_t *
fd_addrlut_new( void * mem ) {

  if( FD_UNLIKELY( !mem ) ) {
    FD_LOG_WARNING(( "NULL mem" ));
    return NULL;
  }

  if( FD_UNLIKELY( !fd_ulong_is_aligned( (ulong)mem, alignof(fd_addrlut_t) ) ) ) {
    FD_LOG_WARNING(( "misaligned mem" ));
    return NULL;
  }

  return fd_type_pun( mem );
}

static int
fd_addrlut_deserialize( fd_addrlut_t * lut,
                        uchar const *  data,
                        ulong          data_sz ) {

  lut = fd_addrlut_new( lut ); FD_TEST( lut );

  fd_bincode_decode_ctx_t decode =
    { .data    = data,
      .dataend = data+data_sz };
  if( FD_UNLIKELY( fd_address_lookup_table_state_decode( &lut->state, &decode )!=FD_BINCODE_SUCCESS ) )
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_DATA;

  if( lut->state.discriminant==fd_address_lookup_table_state_enum_uninitialized )
    return FD_EXECUTOR_INSTR_ERR_UNINITIALIZED_ACCOUNT;
  FD_TEST( lut->state.discriminant == fd_address_lookup_table_state_enum_lookup_table );

  if( data_sz < FD_ADDRLUT_META_SZ )
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_DATA;

  uchar const * raw_addr_data    = data   +FD_ADDRLUT_META_SZ;
  ulong         raw_addr_data_sz = data_sz-FD_ADDRLUT_META_SZ;

  if( !fd_ulong_is_aligned( raw_addr_data_sz, 32UL ) )
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_DATA;

  lut->addr     = fd_type_pun_const( raw_addr_data );
  lut->addr_cnt = raw_addr_data_sz / 32UL;

  return FD_EXECUTOR_INSTR_SUCCESS;
}

static int
fd_addrlut_serialize_meta( fd_address_lookup_table_state_t const * state,
                           uchar * data,
                           ulong   data_sz ) {

  if( FD_UNLIKELY( data_sz<FD_ADDRLUT_META_SZ ) )
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_DATA;

  fd_bincode_encode_ctx_t encode =
    { .data    = data,
      .dataend = data+FD_ADDRLUT_META_SZ };

  int bin_err = fd_address_lookup_table_state_encode( state, &encode );
  FD_TEST( !bin_err );

  fd_memset( data, 0, (ulong)encode.dataend - (ulong)encode.data );
  return FD_EXECUTOR_INSTR_SUCCESS;
}

static int
create_lookup_table( fd_exec_instr_ctx_t *       ctx,
                     fd_addrlut_create_t const * create ) {

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L58-L62 */
  fd_borrowed_account_t * lut_acct;
  int acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 0, &lut_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  ulong               lut_lamports = lut_acct->const_meta->info.lamports;
  fd_pubkey_t const * lut_key      = lut_acct->pubkey;
  uchar const *       lut_owner    = lut_acct->const_meta->info.owner;

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L63-L70 */
  if( !FD_FEATURE_ACTIVE( ctx->slot_ctx, relax_authority_signer_check_for_lookup_table_creation )
      && lut_acct->const_meta->dlen != 0UL ) {
    /* TODO Log: "Table account must not be allocated" */
    return FD_EXECUTOR_INSTR_ERR_ACC_ALREADY_INITIALIZED;
  }
  /* TODO release lut_acct borrow
     https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L71 */

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L73-L75 */
  fd_borrowed_account_t * authority_acct;
  acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 1, &authority_acct );
  fd_pubkey_t const * authority_key = authority_acct->pubkey;

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L76-L83 */
  if( !FD_FEATURE_ACTIVE( ctx->slot_ctx, relax_authority_signer_check_for_lookup_table_creation )
      && !fd_instr_acc_is_signer_idx( ctx->instr, 1 ) ) {
    /* TODO Log: "Authority account must be a signer" */
    return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
  }
  /* TODO release authority_acct borrow
     https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L84 */

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L86-L88 */
  fd_borrowed_account_t * payer_acct;
  acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 2, &payer_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  fd_pubkey_t const * payer_key = payer_acct->pubkey;

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L89-L92 */
  if( !fd_instr_acc_is_signer_idx( ctx->instr, 2 ) ) {
    /* TODO Log: "Payer account must be a signer" */
    return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
  }

  /* TODO release payer_acct borrow
     https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L93 */

  fd_slot_hashes_t slot_hashes = {0};
  int slot_hashes_err = fd_sysvar_slot_hashes_read( ctx->slot_ctx, &slot_hashes );
  if( FD_UNLIKELY( slot_hashes_err ) ) {
    /* TODO what error to return if sysvar read fails? */
    return FD_EXECUTOR_INSTR_ERR_GENERIC_ERR;
  }

  /* TODO Binary search derivation slot in slot hashes sysvar */
  ulong derivation_slot = 1UL;
  if( 1 ) {
    /* TODO Log: {} is not a recent slot */
    return FD_EXECUTOR_INSTR_ERR_INVALID_INSTR_DATA;
  }

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L109-L118 */
  fd_pubkey_t derived_tbl_key[1];
  do {
    fd_sha256_t sha[1]; fd_sha256_init( sha );
    fd_sha256_append( sha, authority_key->key, 32UL );
    fd_sha256_append( sha, &derivation_slot,    8UL );
    fd_sha256_append( sha, &create->bump_seed,  1UL );
    fd_sha256_append( sha, fd_solana_address_lookup_table_program_id.key, 32UL );
    fd_sha256_append( sha, "ProgramDerivedAddress", 21UL );
    fd_sha256_fini( sha, derived_tbl_key->key );
  } while(0);
  if( FD_UNLIKELY( !fd_ed25519_validate_public_key( derived_tbl_key->key ) ) )
    return FD_EXECUTOR_INSTR_ERR_INVALID_SEEDS;

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L120-L127 */
  if( FD_UNLIKELY( 0!=memcmp( lut_key->key, derived_tbl_key->key, 32UL ) ) ) {
    /* TODO Log: "Table address must match derived address: {}" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L129-L135 */
  if( FD_FEATURE_ACTIVE( ctx->slot_ctx, relax_authority_signer_check_for_lookup_table_creation )
      && 0==memcmp( lut_owner, fd_solana_address_lookup_table_program_id.key, 32UL ) ) {
    return FD_EXECUTOR_INSTR_SUCCESS;
  }

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L137-L142 */
  ulong tbl_acct_data_len = 0x38UL;
  ulong required_lamports = fd_rent_exempt_minimum_balance( ctx->slot_ctx, tbl_acct_data_len );
        required_lamports = fd_ulong_max( required_lamports, 1UL );
        required_lamports = fd_ulong_sat_sub( required_lamports, lut_lamports );

  /* https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L144-L149 */
  if( required_lamports > 0UL ) {
    FD_LOG_WARNING(( "TODO: CPI to system program" ));
    return FD_EXECUTOR_INSTR_ERR_UNSUPPORTED_PROGRAM_ID;  /* transfer */
  }

  (void)payer_key;
  FD_LOG_WARNING(( "TODO: CPI to system program" ));  /* allocate */
  FD_LOG_WARNING(( "TODO: CPI to system program" ));  /* assign */

  /* TODO Native cross program invocations ... */

  /* TODO: Acquire writable handle */
  fd_address_lookup_table_state_t state[1];
  fd_address_lookup_table_state_new( state );
  state->discriminant = fd_address_lookup_table_state_enum_lookup_table;
  fd_address_lookup_table_new( &state->inner.lookup_table );
  fd_memcpy( state->inner.lookup_table.meta.authority.key, authority_key->key, 32UL );
  /* TODO set state */

  FD_LOG_WARNING(( "TODO" ));
  (void)ctx; (void)create;
  return FD_EXECUTOR_INSTR_ERR_UNSUPPORTED_PROGRAM_ID;
}

static int
freeze_lookup_table( fd_exec_instr_ctx_t * ctx ) {

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L176-L178 */
  fd_borrowed_account_t * lut_acct;
  int acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 0, &lut_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L178-L181 */
  if( FD_UNLIKELY( 0!=memcmp( lut_acct->const_meta->info.owner, fd_solana_address_lookup_table_program_id.key, sizeof(fd_pubkey_t) ) ) ) {
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_OWNER;
  }
  /* TODO release lut_acct borrow
     https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L71 */

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L183-L186 */
  fd_borrowed_account_t * authority_acct;
  acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 1, &authority_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  fd_pubkey_t const * authority_key = authority_acct->pubkey;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L186-L189 */
  if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx->instr, 1UL ) ) ) {
    /* TODO Log: "Authority account must be a signer" */
    return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
  }
  /* TODO release authority_acct borrow
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L190 */

  /* TODO Re-borrow LUT account
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L192-L193 */

  uchar const * lut_data    = lut_acct->const_data;
  ulong         lut_data_sz = lut_acct->const_meta->dlen;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L195 */
  fd_addrlut_t lut[1];
  int state_err = fd_addrlut_deserialize( lut, (uchar *)lut_data, lut_data_sz );
  if( FD_UNLIKELY( state_err ) ) return state_err;

  fd_address_lookup_table_t * state = &lut->state.inner.lookup_table;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L197-L200 */
  if( FD_UNLIKELY( !state->meta.has_authority ) ) {
    /* TODO Log: "Lookup table is already frozen" */
    return FD_EXECUTOR_INSTR_ERR_ACC_IMMUTABLE;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L201-L203 */
  if( FD_UNLIKELY( 0!=memcmp( state->meta.authority.key, authority_key->key, sizeof(fd_pubkey_t) ) ) ) {
    /* TODO Log: "Incorrect Authority" */
    return FD_EXECUTOR_INSTR_ERR_INCORRECT_AUTHORITY;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L204-L207 */
  if( FD_UNLIKELY( state->meta.deactivation_slot != ULONG_MAX ) ) {
    /* TODO Log: "Deactivated tables cannot be frozen" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L208-L211 */
  if( FD_UNLIKELY( !lut->addr_cnt ) ) {
    /* TODO Log: "Empty lookup tables cannot be frozen" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_INSTR_DATA;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L216 */
  // TODO Check whether we can set data

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L213-L219 */
  state->meta.has_authority = 0;
  state_err = fd_addrlut_serialize_meta( &lut->state, lut_acct->data, lut_acct->meta->dlen );
  if( FD_UNLIKELY( state_err ) ) return state_err;

  return FD_EXECUTOR_INSTR_SUCCESS;
}

static int
extend_lookup_table( fd_exec_instr_ctx_t *       ctx,
                     fd_addrlut_extend_t const * extend ) {

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L230-L232 */
  fd_borrowed_account_t * lut_acct;
  int acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 0, &lut_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  fd_pubkey_t const * lut_key = lut_acct->pubkey;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L233-L235 */
  if( FD_UNLIKELY( 0!=memcmp( lut_acct->const_meta->info.owner, fd_solana_address_lookup_table_program_id.key, sizeof(fd_pubkey_t) ) ) ) {
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_OWNER;
  }
  /* TODO release lut_acct borrow
     https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L236 */

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L238-240 */
  fd_borrowed_account_t * authority_acct;
  acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 1, &authority_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  fd_pubkey_t const * authority_key = authority_acct->pubkey;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L241-244 */
  if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx->instr, 1UL ) ) ) {
    /* TODO Log: "Authority account must be a signer" */
    return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
  }
  /* TODO release authority_acct borrow
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L245 */

  /* TODO Re-borrow LUT account
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L247-248 */

  uchar const * lut_data     = lut_acct->const_data;
  ulong         lut_data_sz  = lut_acct->const_meta->dlen;
  ulong         lut_lamports = lut_acct->const_meta->info.lamports;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L251 */
  fd_addrlut_t lut[1];
  int state_err = fd_addrlut_deserialize( lut, (uchar *)lut_data, lut_data_sz );
  if( FD_UNLIKELY( state_err ) ) return state_err;

  fd_address_lookup_table_t * state = &lut->state.inner.lookup_table;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L253-L255 */
  if( FD_UNLIKELY( !state->meta.has_authority ) ) {
    return FD_EXECUTOR_INSTR_ERR_ACC_IMMUTABLE;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L256-L258 */
  if( FD_UNLIKELY( 0!=memcmp( state->meta.authority.key, authority_key->key, sizeof(fd_pubkey_t) ) ) ) {
    return FD_EXECUTOR_INSTR_ERR_INCORRECT_AUTHORITY;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L259-L262 */
  if( FD_UNLIKELY( state->meta.deactivation_slot != ULONG_MAX ) ) {
    /* TODO Log: "Deactivated tables cannot be extended" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L263-L269 */
  if( FD_UNLIKELY( lut->addr_cnt >= FD_ADDRLUT_MAX_ADDR_CNT ) ) {
    /* TODO Log: "Lookup table is full and cannot contain more addresses" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L271-L274 */
  if( FD_UNLIKELY( !extend->new_addrs_len ) ) {
    /* TODO Log: "Must extend with at least one address" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_INSTR_DATA;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L276-L279 */
  ulong old_addr_cnt = lut->addr_cnt;
  ulong new_addr_cnt = lut->addr_cnt + extend->new_addrs_len;
  if( FD_UNLIKELY( new_addr_cnt > FD_ADDRLUT_MAX_ADDR_CNT ) ) {
    /* TODO Log: "Extended lookup table length {} would exceed max capacity of {}" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_INSTR_DATA;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L290 */
  fd_sol_sysvar_clock_t clock[1];
  int clock_err = fd_sysvar_clock_read( ctx->slot_ctx, clock );
  if( FD_UNLIKELY( clock_err ) ) return clock_err;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L291-L299 */
  if( clock->slot != state->meta.last_extended_slot ) {
    state->meta.last_extended_slot             = clock->slot;
    state->meta.last_extended_slot_start_index = (uchar)lut->addr_cnt;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L308 */
  ulong new_table_data_sz = FD_ADDRLUT_META_SZ + new_addr_cnt * sizeof(fd_pubkey_t);
  int modify_err = fd_instr_borrowed_account_modify( ctx, lut_acct->pubkey, 0, new_table_data_sz, &lut_acct );
  if( FD_UNLIKELY( modify_err ) ) return FD_EXECUTOR_INSTR_ERR_ACC_IMMUTABLE;  /* TODO this error code is wrong */

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L307-L310 */
  state_err = fd_addrlut_serialize_meta( &lut->state, lut_acct->data, lut_acct->meta->dlen );
  if( FD_UNLIKELY( state_err ) ) return state_err;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L311-L313 */
  do {
    uchar * new_keys = lut_acct->data + FD_ADDRLUT_META_SZ + old_addr_cnt * sizeof(fd_pubkey_t);
    fd_memcpy( new_keys, extend->new_addrs, extend->new_addrs_len * sizeof(fd_pubkey_t) );
  } while(0);
  lut->addr     = (fd_pubkey_t *)(lut_acct->data + FD_ADDRLUT_META_SZ);
  lut->addr_cnt = new_addr_cnt;

  /* TODO release borrow
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L315 */

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L317-L321 */
  ulong required_lamports =
    fd_rent_exempt_minimum_balance( ctx->slot_ctx, new_table_data_sz );
  required_lamports = fd_ulong_max    ( required_lamports, 1UL );
  required_lamports = fd_ulong_sat_sub( required_lamports, lut_lamports );

  if( required_lamports ) {
    /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L324-L326 */
    fd_borrowed_account_t * payer_acct = NULL;
    acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 2, &payer_acct );
    if( FD_UNLIKELY( acct_err ) ) {
      /* TODO return code */
      return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
    }
    fd_pubkey_t const * payer_key = payer_acct->pubkey;

    /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L327-L330 */
    if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx->instr, 2UL ) ) ) {
      /* TODO Log: "Payer account must be a signer" */
      return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
    }

    /* TODO release payer acct borrow
       https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L331 */

    /* TODO CPI to system program */
    (void)lut_key; (void)payer_key;
  }

  return FD_EXECUTOR_INSTR_SUCCESS;
}

static int
deactivate_lookup_table( fd_exec_instr_ctx_t * ctx ) {

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L346-347 */
  fd_borrowed_account_t * lut_acct;
  int acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 0, &lut_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L348-L350 */
  if( FD_UNLIKELY( 0!=memcmp( lut_acct->const_meta->info.owner, fd_solana_address_lookup_table_program_id.key, sizeof(fd_pubkey_t) ) ) ) {
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_OWNER;
  }
  /* TODO release lut_acct borrow
     https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L351 */

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L353-L355 */
  fd_borrowed_account_t * authority_acct;
  acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 1, &authority_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  fd_pubkey_t const * authority_key = authority_acct->pubkey;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L356-L359 */
  if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx->instr, 1UL ) ) ) {
    /* TODO Log: "Authority account must be a signer" */
    return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
  }
  /* TODO release authority_acct borrow
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L360 */

  /* TODO Re-borrow LUT account
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L362-L363 */

  uchar const * lut_data    = lut_acct->const_data;
  ulong         lut_data_sz = lut_acct->const_meta->dlen;

  /* TODO Implement AddressLookupTable::deserialize */
  (void)lut_data; (void)lut_data_sz;
  (void)authority_key;

  FD_LOG_WARNING(( "TODO" ));
  (void)ctx;
  return FD_EXECUTOR_INSTR_ERR_UNSUPPORTED_PROGRAM_ID;
}

static int
close_lookup_table( fd_exec_instr_ctx_t * ctx ) {

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L395-L396 */
  fd_borrowed_account_t * lut_acct;
  int acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 0, &lut_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L397-L399 */
  if( FD_UNLIKELY( 0!=memcmp( lut_acct->const_meta->info.owner, fd_solana_address_lookup_table_program_id.key, sizeof(fd_pubkey_t) ) ) ) {
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_OWNER;
  }
  /* TODO release lut_acct borrow
     https://github.com/solana-labs/solana/blob/56ccffdaa5394f179dce6c0383918e571aca8bff/programs/address-lookup-table/src/processor.rs#L400 */

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L402-L404 */
  fd_borrowed_account_t * authority_acct;
  acct_err = fd_instr_ctx_try_borrow_instruction_account( ctx, ctx->txn_ctx, 1, &authority_acct );
  if( FD_UNLIKELY( acct_err ) ) {
    /* TODO return code */
    return FD_EXECUTOR_INSTR_ERR_ACC_BORROW_FAILED;
  }
  fd_pubkey_t const * authority_key = authority_acct->pubkey;

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L405-L408 */
  if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx->instr, 1UL ) ) ) {
    /* TODO Log: "Authority account must be a signer" */
    return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
  }
  /* TODO release authority_acct borrow
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L409 */

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L411 */
  if( FD_UNLIKELY( ctx->instr->acct_cnt < 3 ) ) {
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;
  }

  /* https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L412-L420 */
  /* TODO is this pointer comparison safe? */
  if( FD_UNLIKELY( ctx->instr->borrowed_accounts[0]
                == ctx->instr->borrowed_accounts[2] ) ) {
    /* TODO Log: "Lookup table cannot be recipient of reclaimed lamports" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* TODO Re-borrow LUT account
     https://github.com/solana-labs/solana/blob/abf3b3e527c8b24b122ab2cccb34d9aff05f8c15/programs/address-lookup-table/src/processor.rs#L422-L423 */

  ulong         withdrawn_lamports = lut_acct->const_meta->info.lamports;
  uchar const * lut_data           = lut_acct->const_data;
  ulong         lut_data_sz        = lut_acct->const_meta->dlen;

  /* TODO Implement AddressLookupTable::deserialize */
  (void)lut_data; (void)lut_data_sz;
  (void)authority_key; (void)withdrawn_lamports;

  FD_LOG_WARNING(( "TODO" ));
  (void)ctx;
  return FD_EXECUTOR_INSTR_ERR_UNSUPPORTED_PROGRAM_ID;
}

int
fd_executor_address_lookup_table_program_execute_instruction( fd_exec_instr_ctx_t * ctx ) {

  uchar const * instr_data    = ctx->instr->data;
  ulong         instr_data_sz = ctx->instr->data_sz;

  FD_SCRATCH_SCOPED_FRAME;

  fd_bincode_decode_ctx_t decode = {
    .valloc  = fd_scratch_virtual(),
    .data    = instr_data,
    .dataend = instr_data + instr_data_sz
  };
  fd_addrlut_instruction_t instr[1];
  /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/address-lookup-table/src/processor.rs#L31 */
  if( FD_UNLIKELY( fd_addrlut_instruction_decode( instr, &decode ) != FD_BINCODE_SUCCESS ) )
    return FD_EXECUTOR_INSTR_ERR_INVALID_INSTR_DATA;

  switch( instr->discriminant ) {
  case fd_addrlut_instruction_enum_create_lut:
    return create_lookup_table( ctx, &instr->inner.create_lut );
  case fd_addrlut_instruction_enum_freeze_lut:
    return freeze_lookup_table( ctx );
  case fd_addrlut_instruction_enum_extend_lut:
    return extend_lookup_table( ctx, &instr->inner.extend_lut );
  case fd_addrlut_instruction_enum_deactivate_lut:
    return deactivate_lookup_table( ctx );
  case fd_addrlut_instruction_enum_close_lut:
    return close_lookup_table( ctx );
  default:
    break;
  }

  return FD_EXECUTOR_INSTR_SUCCESS;
}
