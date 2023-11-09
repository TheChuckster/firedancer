#include "fd_exec_txn_ctx.h"

#include "../fd_acc_mgr.h"

int
fd_txn_borrowed_account_view_idx( fd_exec_txn_ctx_t * ctx,
                                  uchar idx,
                                  fd_borrowed_account_t * *  account ) {
  if( idx >= ctx->accounts_cnt ) {
    return FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT;
  }

  // TODO: check if readable???
  fd_borrowed_account_t * txn_account = &ctx->borrowed_accounts[idx];
  *account = txn_account;
  return FD_ACC_MGR_SUCCESS;
}

int
fd_txn_borrowed_account_view( fd_exec_txn_ctx_t * ctx,
                              fd_pubkey_t const *      pubkey,
                              fd_borrowed_account_t * * account ) {
  for( ulong i = 0; i < ctx->accounts_cnt; i++ ) {
    if( memcmp( pubkey->uc, ctx->accounts[i].uc, sizeof(fd_pubkey_t) )==0 ) {
      // TODO: check if readable???
      fd_borrowed_account_t * txn_account = &ctx->borrowed_accounts[i];
      *account = txn_account;

      if (FD_UNLIKELY(!FD_RAW_ACCOUNT_EXISTS(txn_account->const_meta)))
        return FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT;

      return FD_ACC_MGR_SUCCESS;
    }
  }

  return FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT;
}

int
fd_txn_borrowed_account_executable_view( fd_exec_txn_ctx_t * ctx,
                              fd_pubkey_t const *      pubkey,
                              fd_borrowed_account_t * * account ) {
  for( ulong i = 0; i < ctx->executable_cnt; i++ ) {
    if( memcmp( pubkey->uc, ctx->executable_accounts[i].pubkey->uc, sizeof(fd_pubkey_t) )==0 ) {
      // TODO: check if readable???
      fd_borrowed_account_t * txn_account = &ctx->executable_accounts[i];
      *account = txn_account;

      if (FD_UNLIKELY(!FD_RAW_ACCOUNT_EXISTS(txn_account->const_meta)))
        return FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT;

      return FD_ACC_MGR_SUCCESS;
    }
  }

  return FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT;
}

int
fd_txn_borrowed_account_modify_idx( fd_exec_txn_ctx_t * ctx,
                                uchar idx,
                                int do_create,
                                ulong min_data_sz,
                                fd_borrowed_account_t * *  account ) {
  if( idx >= ctx->accounts_cnt ) {
    return FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT;
  }

  fd_borrowed_account_t * txn_account = &ctx->borrowed_accounts[idx];
  int err = fd_acc_mgr_modify( ctx->acc_mgr, ctx->funk_txn, &ctx->accounts[idx], do_create, min_data_sz, txn_account );
  if( err != FD_ACC_MGR_SUCCESS ) {
    return err;
  }
  // TODO: check if writable???
  *account = txn_account;
  return FD_ACC_MGR_SUCCESS;
}

int
fd_txn_borrowed_account_modify( fd_exec_txn_ctx_t * ctx,
                                fd_pubkey_t const * pubkey,
                                int do_create,
                                ulong min_data_sz,
                                fd_borrowed_account_t * * account ) {
  for( ulong i = 0; i < ctx->accounts_cnt; i++ ) {
    if( memcmp( pubkey->uc, ctx->accounts[i].uc, sizeof(fd_pubkey_t) )==0 ) {
      // TODO: check if writable???
      fd_borrowed_account_t * txn_account = &ctx->borrowed_accounts[i];
      int err = fd_acc_mgr_modify( ctx->acc_mgr, ctx->funk_txn, &ctx->accounts[i], do_create, min_data_sz, txn_account );
      if( err != FD_ACC_MGR_SUCCESS ) {
        return err;
      }
      *account = txn_account;
      return FD_ACC_MGR_SUCCESS;
    }
  }

  return FD_ACC_MGR_ERR_UNKNOWN_ACCOUNT;
}

void *
fd_exec_txn_ctx_new( void * mem ) {
  if( FD_UNLIKELY( !mem ) ) {
    FD_LOG_WARNING(( "NULL mem" ));
    return NULL;
  }

  if( FD_UNLIKELY( !fd_ulong_is_aligned( (ulong)mem, FD_EXEC_TXN_CTX_ALIGN ) ) ) {
    FD_LOG_WARNING(( "misaligned mem" ));
    return NULL;
  }

  fd_memset(mem, 0, FD_EXEC_TXN_CTX_FOOTPRINT);

  fd_exec_txn_ctx_t * self = (fd_exec_txn_ctx_t *) mem;

  FD_COMPILER_MFENCE();
  self->magic = FD_EXEC_TXN_CTX_MAGIC;
  FD_COMPILER_MFENCE();

  return mem;
}

fd_exec_txn_ctx_t *
fd_exec_txn_ctx_join( void * mem ) {
  if( FD_UNLIKELY( !mem ) ) {
    FD_LOG_WARNING(( "NULL block" ));
    return NULL;
  }

  fd_exec_txn_ctx_t * ctx = (fd_exec_txn_ctx_t *) mem;

  if( FD_UNLIKELY( ctx->magic!=FD_EXEC_TXN_CTX_MAGIC ) ) {
    FD_LOG_WARNING(( "bad magic" ));
    return NULL;
  }

  return ctx;
}

void *
fd_exec_txn_ctx_leave( fd_exec_txn_ctx_t * ctx) {
  if( FD_UNLIKELY( !ctx ) ) {
    FD_LOG_WARNING(( "NULL block" ));
    return NULL;
  }

  if( FD_UNLIKELY( ctx->magic!=FD_EXEC_TXN_CTX_MAGIC ) ) {
    FD_LOG_WARNING(( "bad magic" ));
    return NULL;
  }

  return (void *) ctx;
}

void *
fd_exec_txn_ctx_delete( void * mem ) {
  if( FD_UNLIKELY( !mem ) ) {
    FD_LOG_WARNING(( "NULL mem" ));
    return NULL;
  }

  if( FD_UNLIKELY( !fd_ulong_is_aligned( (ulong)mem, FD_EXEC_TXN_CTX_ALIGN) ) )  {
    FD_LOG_WARNING(( "misaligned mem" ));
    return NULL;
  }

  fd_exec_txn_ctx_t * hdr = (fd_exec_txn_ctx_t *)mem;
  if( FD_UNLIKELY( hdr->magic!=FD_EXEC_TXN_CTX_MAGIC ) ) {
    FD_LOG_WARNING(( "bad magic" ));
    return NULL;
  }

  FD_COMPILER_MFENCE();
  FD_VOLATILE( hdr->magic ) = 0UL;
  FD_COMPILER_MFENCE();

  return mem;
}