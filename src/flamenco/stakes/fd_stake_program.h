#ifndef HEADER_fd_src_flamenco_runtime_program_fd_stake_program_h
#define HEADER_fd_src_flamenco_runtime_program_fd_stake_program_h

#include "../fd_flamenco_base.h"
#include "../runtime/fd_executor.h"
#include "../runtime/fd_runtime.h"
#include "../runtime/program/fd_vote_program.h"

FD_PROTOTYPES_BEGIN

#define SOLANA_STAKE_PROGRAM_ID ( (uchar *)"Stake11111111111111111111111111111111111111" )

/* Entry-point for the Solana Stake Program */
int
fd_executor_stake_program_execute_instruction( instruction_ctx_t ctx );

/* Initializes an account which holds configuration used by the stake program.
   https://github.com/solana-labs/solana/blob/8f2c8b8388a495d2728909e30460aa40dcc5d733/sdk/program/src/stake/config.rs
 */
void
fd_stake_program_config_init( fd_global_ctx_t * global );

int
fd_stake_get_state( fd_borrowed_account_t const * self,
                    fd_valloc_t const *           valloc,
                    fd_stake_state_v2_t *         out );

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_runtime_program_fd_stake_program_h */
