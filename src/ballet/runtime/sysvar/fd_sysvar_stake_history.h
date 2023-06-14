#ifndef HEADER_fd_src_ballet_runtime_fd_sysvar_stake_history_h
#define HEADER_fd_src_ballet_runtime_fd_sysvar_stake_history_h

#include "../../fd_ballet_base.h"
#include "../fd_executor.h"

/* The stake history sysvar contains the history of cluster-wide activations and de-activations per-epoch. Updated at the start of each epoch. */

/* Initialize the stake history sysvar account. */
void fd_sysvar_stake_history_init( fd_global_ctx_t* global );

/* Reads the current value of the stake history sysvar */
void fd_sysvar_stake_history_read( fd_global_ctx_t* global, fd_stake_history_t* result );

#endif /* HEADER_fd_src_ballet_runtime_fd_sysvar_stake_history_h */

