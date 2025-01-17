#define _GNU_SOURCE
#include "../../../../disco/tiles.h"

#include "generated/sign_seccomp.h"

#include "../../../../disco/keyguard/fd_keyguard.h"
#include "../../../../disco/keyguard/fd_keyload.h"

#include <errno.h>
#include <sys/mman.h>

#define MAX_IN (32UL)

/* fd_sign_in_ctx_t is a context object for each in (producer) mcache
   connected to the sign tile. */

typedef struct {
  ulong            seq;
  fd_frag_meta_t * mcache;
  uchar *          data;
} fd_sign_out_ctx_t;

typedef struct {
  uchar             _data[ FD_KEYGUARD_SIGN_REQ_MTU ];

  int               in_role[ MAX_IN ];
  uchar *           in_data[ MAX_IN ];
  ushort            in_mtu [ MAX_IN ];

  fd_sign_out_ctx_t out[ MAX_IN ];

  fd_sha512_t       sha512 [ 1 ];

  uchar const *     public_key;
  uchar const *     private_key;
} fd_sign_ctx_t;

FD_FN_CONST static inline ulong
scratch_align( void ) {
  return alignof( fd_sign_ctx_t );
}

FD_FN_PURE static inline ulong
scratch_footprint( fd_topo_tile_t const * tile ) {
  (void)tile;
  ulong l = FD_LAYOUT_INIT;
  l = FD_LAYOUT_APPEND( l, alignof( fd_sign_ctx_t ), sizeof( fd_sign_ctx_t ) );
  return FD_LAYOUT_FINI( l, scratch_align() );
}

FD_FN_CONST static inline void *
mux_ctx( void * scratch ) {
  return (void*)fd_ulong_align_up( (ulong)scratch, alignof( fd_sign_ctx_t ) );
}

/* during_frag is called between pairs for sequence number checks, as
   we are reading incoming frags.  We don't actually need to copy the
   fragment here, see fd_dedup.c for why we do this.*/

static void FD_FN_SENSITIVE
during_frag_sensitive( void * _ctx,
                       ulong  in_idx,
                       ulong  seq,
                       ulong  sig,
                       ulong  chunk,
                       ulong  sz,
                       int *  opt_filter ) {
  (void)seq;
  (void)sig;
  (void)chunk;
  (void)sz;
  (void)opt_filter;

  fd_sign_ctx_t * ctx = (fd_sign_ctx_t *)_ctx;
  FD_TEST( in_idx<MAX_IN );

  int  role = ctx->in_role[ in_idx ];
  uint mtu  = ctx->in_mtu [ in_idx ];

  if( sz>mtu ) {
    FD_LOG_EMERG(( "oversz signing request (role=%d sz=%lu mtu=%u)", role, sz, mtu ));
  }
  fd_memcpy( ctx->_data, ctx->in_data[ in_idx ], sz );
}


static void
during_frag( void * _ctx,
             ulong  in_idx,
             ulong  seq,
             ulong  sig,
             ulong  chunk,
             ulong  sz,
             int *  opt_filter ) {
  during_frag_sensitive( _ctx, in_idx, seq, sig, chunk, sz, opt_filter );
}

static void FD_FN_SENSITIVE
after_frag_sensitive( void *             _ctx,
                      ulong              in_idx,
                      ulong              seq,
                      ulong *            opt_sig,
                      ulong *            opt_chunk,
                      ulong *            opt_sz,
                      ulong *            opt_tsorig,
                      int *              opt_filter,
                      fd_mux_context_t * mux ) {
  (void)seq;
  (void)opt_chunk;
  (void)opt_tsorig;
  (void)opt_filter;
  (void)mux;

  fd_sign_ctx_t * ctx = (fd_sign_ctx_t *)_ctx;

  ulong sz        = *opt_sz;
  ulong sig       = *opt_sig;
  int   sign_type = (int)(uint)sig;

  FD_TEST( in_idx<MAX_IN );

  int role = ctx->in_role[ in_idx ];

  fd_keyguard_authority_t authority = {0};
  memcpy( authority.identity_pubkey, ctx->public_key, 32 );

  if( FD_UNLIKELY( !fd_keyguard_payload_authorize( &authority, ctx->_data, sz, role, sign_type ) ) ) {
    FD_LOG_EMERG(( "fd_keyguard_payload_authorize failed (role=%d sign_type=%d)", role, sign_type ));
  }

  switch( sign_type ) {
  case FD_KEYGUARD_SIGN_TYPE_ED25519: {
    fd_ed25519_sign( ctx->out[ in_idx ].data, ctx->_data, sz, ctx->public_key, ctx->private_key, ctx->sha512 );
    break;
  }
  case FD_KEYGUARD_SIGN_TYPE_SHA256_ED25519: {
    uchar hash[ 32 ];
    fd_sha256_hash( ctx->_data, sz, hash );
    fd_ed25519_sign( ctx->out[ in_idx ].data, hash, 32UL, ctx->public_key, ctx->private_key, ctx->sha512 );
    break;
  }
  default:
    FD_LOG_EMERG(( "invalid sign type: %d", sign_type ));
  }

  fd_mcache_publish( ctx->out[ in_idx ].mcache, 128UL, ctx->out[ in_idx ].seq, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL );
  ctx->out[ in_idx ].seq = fd_seq_inc( ctx->out[ in_idx ].seq, 1UL );
}

static void
after_frag( void *             _ctx,
            ulong              in_idx,
            ulong              seq,
            ulong *            opt_sig,
            ulong *            opt_chunk,
            ulong *            opt_sz,
            ulong *            opt_tsorig,
            int *              opt_filter,
            fd_mux_context_t * mux ) {
  after_frag_sensitive( _ctx, in_idx, seq, opt_sig, opt_chunk, opt_sz, opt_tsorig, opt_filter, mux );
}

static void FD_FN_SENSITIVE
privileged_init_sensitive( fd_topo_t *      topo,
                           fd_topo_tile_t * tile,
                           void *           scratch ) {
  (void)topo;

  FD_SCRATCH_ALLOC_INIT( l, scratch );
  fd_sign_ctx_t * ctx = FD_SCRATCH_ALLOC_APPEND( l, alignof( fd_sign_ctx_t ), sizeof( fd_sign_ctx_t ) );

  uchar const * identity_key = fd_keyload_load( tile->sign.identity_key_path, /* pubkey only: */ 0 );
  ctx->private_key = identity_key;
  ctx->public_key  = identity_key + 32UL;

    /* The stack can be taken over and reorganized by under AddressSanitizer,
     which causes this code to fail.  */
#if FD_HAS_ASAN
  FD_LOG_WARNING(( "!!! SECURITY WARNING !!! YOU ARE RUNNING THE SIGNING TILE "
                   "WITH ADDRESS SANITIZER ENABLED. THIS CAN LEAK SENSITIVE "
                   "DATA INCLUDING YOUR PRIVATE KEYS INTO CORE DUMPS IF THIS "
                   "PROCESS ABORTS. IT IS HIGHLY ADVISED TO NOT TO RUN IN THIS "
                   "MODE IN PRODUCTION!" ));
#else
  /* Prevent the stack from showing up in core dumps just in case the
     private key somehow ends up in there. */
  FD_TEST( fd_tile_stack0() );
  FD_TEST( fd_tile_stack_sz() );
  if( FD_UNLIKELY( madvise( (void*)fd_tile_stack0(), fd_tile_stack_sz(), MADV_DONTDUMP ) ) )
    FD_LOG_ERR(( "madvise failed (%i-%s)", errno, fd_io_strerror( errno ) ));
#endif
}

static void
privileged_init( fd_topo_t *      topo,
                 fd_topo_tile_t * tile,
                 void *           scratch ) {
  privileged_init_sensitive( topo, tile, scratch );
}

static void FD_FN_SENSITIVE
unprivileged_init_sensitive( fd_topo_t *      topo,
                             fd_topo_tile_t * tile,
                             void *           scratch ) {
  FD_SCRATCH_ALLOC_INIT( l, scratch );
  fd_sign_ctx_t * ctx = FD_SCRATCH_ALLOC_APPEND( l, alignof( fd_sign_ctx_t ), sizeof( fd_sign_ctx_t ) );
  FD_TEST( fd_sha512_join( fd_sha512_new( ctx->sha512 ) ) );

  uchar check_public_key[ 32 ];
  fd_ed25519_public_from_private( check_public_key, ctx->private_key, ctx->sha512 );
  if( FD_UNLIKELY( memcmp( check_public_key, ctx->public_key, 32 ) ) )
    FD_LOG_EMERG(( "The public key in the identity key file does not match the public key derived from the private key. "
                   "Firedancer will not use the key pair to sign as it might leak the private key." ));

  FD_TEST( tile->in_cnt<=MAX_IN );
  FD_TEST( tile->in_cnt==tile->out_cnt );

  for( ulong i=0; i<MAX_IN; i++ ) ctx->in_role[ i ] = -1;

  for( ulong i=0; i<tile->in_cnt; i++ ) {
    fd_topo_link_t * in_link = &topo->links[ tile->in_link_id[ i ] ];
    fd_topo_link_t * out_link = &topo->links[ tile->out_link_id[ i ] ];

    if( in_link->mtu > FD_KEYGUARD_SIGN_REQ_MTU ) FD_LOG_CRIT(( "oversz link[%lu].mtu=%lu", i, in_link->mtu ));
    ctx->in_data[ i ] = in_link->dcache;
    ctx->in_mtu [ i ] = (ushort)in_link->mtu;

    ctx->out[ i ].mcache = out_link->mcache;
    ctx->out[ i ].data   = out_link->dcache;
    ctx->out[ i ].seq    = 0UL;

    if( !strcmp( in_link->name, "shred_sign" ) ) {
      ctx->in_role[ i ] = FD_KEYGUARD_ROLE_LEADER;
      FD_TEST( !strcmp( out_link->name, "sign_shred" ) );
      FD_TEST( in_link->mtu==32UL );
      FD_TEST( out_link->mtu==64UL );
    } else if( !strcmp( in_link->name, "quic_sign" ) ) {
      ctx->in_role[ i ] = FD_KEYGUARD_ROLE_QUIC;
      FD_TEST( !strcmp( out_link->name, "sign_quic" ) );
      FD_TEST( in_link->mtu==130UL );
      FD_TEST( out_link->mtu==64UL );
    } else if ( !strcmp( in_link->name, "gossip_sign" ) ) {
      ctx->in_role[ i ] = FD_KEYGUARD_ROLE_GOSSIP;
      FD_TEST( !strcmp( out_link->name, "sign_gossip" ) );
      FD_TEST( in_link->mtu==2048UL );
      FD_TEST( out_link->mtu==64UL );
    } else if ( !strcmp( in_link->name, "repair_sign")) {
      ctx->in_role[ i ] = FD_KEYGUARD_ROLE_REPAIR;
      FD_TEST( !strcmp( out_link->name, "repair_gossip" ) );
      FD_TEST( in_link->mtu==2048UL );
      FD_TEST( out_link->mtu==64UL );
    } else if ( !strcmp(in_link->name, "voter_sign" ) ) {
      ctx->in_role[ i ] = FD_KEYGUARD_ROLE_VOTER;
      FD_TEST( !strcmp( out_link->name, "sign_voter"  ) );
      FD_TEST( in_link->mtu==FD_TXN_MTU  );
      FD_TEST( out_link->mtu==64UL );
    } else {
      FD_LOG_CRIT(( "unexpected link %s", in_link->name ));
    }
  }

  ulong scratch_top = FD_SCRATCH_ALLOC_FINI( l, 1UL );
  if( FD_UNLIKELY( scratch_top > (ulong)scratch + scratch_footprint( tile ) ) )
    FD_LOG_ERR(( "scratch overflow %lu %lu %lu", scratch_top - (ulong)scratch - scratch_footprint( tile ), scratch_top, (ulong)scratch + scratch_footprint( tile ) ));
}

static void
unprivileged_init( fd_topo_t *      topo,
                   fd_topo_tile_t * tile,
                   void *           scratch ) {
  unprivileged_init_sensitive( topo, tile, scratch );
}

static ulong
populate_allowed_seccomp( void *               scratch,
                          ulong                out_cnt,
                          struct sock_filter * out ) {
  (void)scratch;
  populate_sock_filter_policy_sign( out_cnt, out, (uint)fd_log_private_logfile_fd() );
  return sock_filter_policy_sign_instr_cnt;
}

static ulong
populate_allowed_fds( void * scratch,
                      ulong  out_fds_cnt,
                      int *  out_fds ) {
  (void)scratch;
  if( FD_UNLIKELY( out_fds_cnt < 2 ) ) FD_LOG_ERR(( "out_fds_cnt %lu", out_fds_cnt ));

  ulong out_cnt = 0;
  out_fds[ out_cnt++ ] = 2; /* stderr */
  if( FD_LIKELY( -1!=fd_log_private_logfile_fd() ) )
    out_fds[ out_cnt++ ] = fd_log_private_logfile_fd(); /* logfile */
  return out_cnt;
}

static long
lazy( fd_topo_tile_t * tile ) {
  (void)tile;
  /* See explanation in fd_pack */
  return 128L * 300L;
}

fd_topo_run_tile_t fd_tile_sign = {
  .name                     = "sign",
  .mux_flags                = FD_MUX_FLAG_COPY | FD_MUX_FLAG_MANUAL_PUBLISH,
  .burst                    = 1UL,
  .mux_ctx                  = mux_ctx,
  .mux_during_frag          = during_frag,
  .mux_after_frag           = after_frag,
  .lazy                     = lazy,
  .populate_allowed_seccomp = populate_allowed_seccomp,
  .populate_allowed_fds     = populate_allowed_fds,
  .scratch_align            = scratch_align,
  .scratch_footprint        = scratch_footprint,
  .privileged_init          = privileged_init,
  .unprivileged_init        = unprivileged_init,
};
