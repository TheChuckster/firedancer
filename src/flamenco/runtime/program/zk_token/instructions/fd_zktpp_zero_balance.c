#include "../fd_zktpp_private.h"
#include "../bulletproofs/fd_bulletproofs.h"
#include "../encryption/fd_zktpp_encryption.h"
#include "../twisted_elgamal/fd_twisted_elgamal.h"

int
fd_zktpp_verify_proof_zero_balance( FD_FN_UNUSED void * context, FD_FN_UNUSED void * proof ) {
  //TODO
  fd_bulletproofs_placeholder( context );
  fd_zktpp_encryption_placeholder( context );
  fd_twisted_elgamal_placeholder( context );
  return FD_EXECUTOR_INSTR_SUCCESS;
}
