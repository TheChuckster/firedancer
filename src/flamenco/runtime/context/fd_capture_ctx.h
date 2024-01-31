#ifndef HEADER_fd_src_flamenco_runtime_context_fd_capture_ctx_h
#define HEADER_fd_src_flamenco_runtime_context_fd_capture_ctx_h

#include "../../capture/fd_solcap_writer.h"

/* Context needed to do solcap capture during execution of transactions */
#define FD_CAPTURE_CTX_ALIGN (8UL)
struct __attribute__((aligned(FD_CAPTURE_CTX_ALIGN))) fd_capture_ctx {
  ulong magic; /* ==FD_CAPTURE_CTX_MAGIC */

  int                      trace_dirfd;
  int                      trace_mode;
  fd_solcap_writer_t *     capture;
};
typedef struct fd_capture_ctx fd_capture_ctx_t;
#define FD_CAPTURE_CTX_FOOTPRINT ( sizeof(fd_capture_ctx_t) + fd_solcap_writer_footprint() )
#define FD_CAPTURE_CTX_MAGIC (0x193ECD2A6C395195UL) /* random */

FD_PROTOTYPES_BEGIN

void *
fd_capture_ctx_new( void * mem );

fd_capture_ctx_t *
fd_capture_ctx_join( void * mem );

void *
fd_capture_ctx_leave( fd_capture_ctx_t * ctx );

void *
fd_capture_ctx_delete( void * mem );

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_runtime_context_fd_capture_ctx_h */