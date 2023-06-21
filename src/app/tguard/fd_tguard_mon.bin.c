#include "fd_tguard.h"

#if FD_HAS_TGUARD

/**********************************************************************/
/* FIXME: These APIs are probably useful to move to a monitor lib */

#include <stdio.h>
#include <signal.h>

/* TEXT_* are quick-and-dirty color terminal hacks.  Probably should
   do something more robust longer term. */

#define TEXT_NORMAL "\033[0m"
#define TEXT_BLUE   "\033[34m"
#define TEXT_GREEN  "\033[32m"
#define TEXT_YELLOW "\033[93m"
#define TEXT_RED    "\033[31m"

static void
fd_tguard_mon_sigaction( 
  int         sig,
  siginfo_t * info,
  void *      context 
) {
  (void)info;
  (void)context;
  FD_LOG_NOTICE(( "received POSIX signal %i; fd_tguard_mon.bin signing off...", sig ));
  fd_halt();
  exit (1); /* need explicit exit to make fd_tguard_cnc recoverable upon mon SIGINT */
}

static void
fd_tguard_mon_signal_trap( 
  int sig 
) {
  struct sigaction act[1];
  act->sa_sigaction = fd_tguard_mon_sigaction;
  if( FD_UNLIKELY( sigemptyset( &act->sa_mask ) ) ) FD_LOG_ERR(( "sigempty set failed" ));
  act->sa_flags = (int)(SA_SIGINFO | SA_RESETHAND);
  if( FD_UNLIKELY( sigaction( sig, act, NULL ) ) ) FD_LOG_ERR(( "unable to override signal %i", sig ));
}

/* printf_age prints _dt in ns as an age to stdout, will be exactly 10
   char wide.  Since pretty printing this value will often require
   rounding it, the rounding is in a round toward zero sense. */

static void
printf_age( long _dt ) {
  if( FD_UNLIKELY( _dt< 0L ) ) { printf( "   invalid" ); return; }
  if( FD_UNLIKELY( _dt==0L ) ) { printf( "        0s" ); return; }
  ulong rem = (ulong)_dt;
  ulong ns = rem % 1000UL; rem /= 1000UL; if( !rem /*no u*/ ) { printf( "      %3lun",           ns                   ); return; }
  ulong us = rem % 1000UL; rem /= 1000UL; if( !rem /*no m*/ ) { printf( "  %3lu.%03luu",         us, ns               ); return; }
  ulong ms = rem % 1000UL; rem /= 1000UL; if( !rem /*no s*/ ) { printf( "%3lu.%03lu%02lum",      ms, us, ns/10UL      ); return; }
  ulong  s = rem %   60UL; rem /=   60UL; if( !rem /*no m*/ ) { printf( "%2lu.%03lu%03lus",      s,  ms, us           ); return; }
  ulong  m = rem %   60UL; rem /=   60UL; if( !rem /*no h*/ ) { printf( "%2lu:%02lu.%03lu%1lu",  m,  s,  ms, us/100UL ); return; }
  ulong  h = rem %   24UL; rem /=   24UL; if( !rem /*no d*/ ) { printf( "%2lu:%02lu:%02lu.%1lu", h,  m,  s,  ms/100UL ); return; }
  ulong  d = rem %    7UL; rem /=    7UL; if( !rem /*no w*/ ) { printf( "  %1lud %2lu:%02lu",    d,  h,  m            ); return; }
  ulong  w = rem;                         if( w<=99UL       ) { printf( "%2luw %1lud %2luh",     w,  d,  h            ); return; }
  /* note that this can handle LONG_MAX fine */                 printf( "%6luw %1lud",           w,  d                );
}

/* printf_stale is printf_age with the tweak that ages less than or
   equal to expire (in ns) will be suppressed to limit visual chatter.
   Will be exactly 10 char wide and color coded. */

static void
printf_stale( long age,
              long expire ) {
  if( FD_UNLIKELY( age>expire ) ) {
    printf( TEXT_YELLOW );
    printf_age( age );
    printf( TEXT_NORMAL );
    return;
  }
  printf( TEXT_GREEN "         -" TEXT_NORMAL );
}

/* printf_heart will print to stdout whether or not a heartbeat was
   detected.  Will be exactly 5 char wide and color coded. */

static void
printf_heart( long hb_now,
              long hb_then ) {
  long dt = hb_now - hb_then;
  printf( "%s", (dt>0L) ? (TEXT_GREEN "    -" TEXT_NORMAL) :
                (!dt)   ? (TEXT_RED   " NONE" TEXT_NORMAL) :
                          (TEXT_BLUE  "RESET" TEXT_NORMAL) );
}

/* printf_sig will print the current and previous value of a cnc signal.
   to stdout.  Will be exactly 10 char wide and color coded. */

static char const *
sig_color( ulong sig ) {
  switch( sig ) {
  case FD_CNC_SIGNAL_BOOT: return TEXT_BLUE;   break; /* Blue -> waiting for tile to start */
  case FD_CNC_SIGNAL_HALT: return TEXT_YELLOW; break; /* Yellow -> waiting for tile to process */
  case FD_CNC_SIGNAL_RUN:  return TEXT_GREEN;  break; /* Green -> Normal */
  case FD_CNC_SIGNAL_FAIL: return TEXT_RED;    break; /* Red -> Definitely abnormal */
  default: break; /* Unknown, don't colorize */
  }
  return TEXT_NORMAL;
}

static void
printf_sig( ulong sig_now,
            ulong sig_then ) {
  char buf0[ FD_CNC_SIGNAL_CSTR_BUF_MAX ];
  char buf1[ FD_CNC_SIGNAL_CSTR_BUF_MAX ];
  printf( "%s%4s" TEXT_NORMAL "(%s%4s" TEXT_NORMAL ")",
          sig_color( sig_now  ), fd_cnc_signal_cstr( sig_now,  buf0 ),
          sig_color( sig_then ), fd_cnc_signal_cstr( sig_then, buf1 ) );
}

/* printf_err_bool will print to stdout a boolean flag that indicates
   if error condition was present now and then.  Will be exactly 12 char
   wide and color coded. */

static void
printf_err_bool( ulong err_now,
                 ulong err_then ) {
  printf( "%5s(%5s)", err_now  ? TEXT_RED "err" TEXT_NORMAL : TEXT_GREEN "  -" TEXT_NORMAL,
                      err_then ? TEXT_RED "err" TEXT_NORMAL : TEXT_GREEN "  -" TEXT_NORMAL );
}

/* printf_err_cnt will print to stdout a 64-bit counter holding the
   number of times an error condition has happened and how it has
   changed between now and then.  Will be exactly 19 char wide and color
   coded.  (The 64-bit counter will be truncated to 32-bit before pretty
   printing to make it more compact as cumulative value of such counters
   is often less important than how it is changed between now and then.) */

static void
printf_err_cnt( ulong cnt_now,
                ulong cnt_then ) {
  long delta = (long)(cnt_now - cnt_then);
  char const * color = (!delta)   ? TEXT_GREEN  /* no new error counts */
                     : (delta>0L) ? TEXT_RED    /* new error counts */
                     : (cnt_now)  ? TEXT_YELLOW /* decrease of existing error counts?? */
                     :              TEXT_BLUE;  /* reset of the error counter */
  if(      delta> 99999L ) printf( "%10u(%s>+99999" TEXT_NORMAL ")", (uint)cnt_now, color        );
  else if( delta<-99999L ) printf( "%10u(%s<-99999" TEXT_NORMAL ")", (uint)cnt_now, color        );
  else                     printf( "%10u(%s %+6li"  TEXT_NORMAL ")", (uint)cnt_now, color, delta );
}

/* printf_seq will print to stdout a 64-bit sequence number and how it
   has changed between now and then.  Will be exactly 25 char wide and
   color coded. */

static void
printf_seq( ulong seq_now,
            ulong seq_then ) {
  long delta = (long)(seq_now - seq_then);
  char const * color = (!delta)   ? TEXT_YELLOW /* no sequence numbers published */
                     : (delta>0L) ? TEXT_GREEN  /* new sequence numbers published */
                     : (seq_now)  ? TEXT_RED    /* sequence number went backward */
                     :              TEXT_BLUE;  /* sequence number reset */
  if(      delta> 99999L ) printf( "%16lu(%s>+99999" TEXT_NORMAL ")", seq_now, color        );
  else if( delta<-99999L ) printf( "%16lu(%s<-99999" TEXT_NORMAL ")", seq_now, color        );
  else                     printf( "%16lu(%s %+6li"  TEXT_NORMAL ")", seq_now, color, delta );
}

/* printf_rate prints to stdout:

     cvt*((overhead + (cnt_now - cnt_then)) / dt)

   Will be exactly 8 char wide, right justifed with aligned decimal
   point.  Uses standard engineering base 10 suffixes (e.g. 10.0e9 ->
   10.0G) to support wide dynamic range rate diagnostics.  Since pretty
   printing this value will often require rounding it, the rounding is
   roughly in a round toward near even zero sense (this could be
   improved numerically to make it even more strict rounding, e.g.
   rate*=1e-3 used below is not exact, but this is more than adequate
   for a quick-and-dirty low precision diagnostic. */

static void
printf_rate( double cvt,
             double overhead,
             ulong  cnt_now,
             ulong  cnt_then,
             long   dt  ) {
  if( FD_UNLIKELY( !((0.< cvt     ) & (cvt<=DBL_MAX)) |
                   !((0.<=overhead) & (cvt<=DBL_MAX)) |
                   (cnt_now<cnt_then)                 |
                   (dt<=0L)                           ) ) {
    printf( TEXT_RED " invalid" TEXT_NORMAL );
    return;
  }
  double rate = cvt*(overhead+(double)(cnt_now-cnt_then)) / (double)dt;
  if( FD_UNLIKELY( !((0.<=rate) & (rate<=DBL_MAX)) ) ) {
    printf( TEXT_RED "overflow" TEXT_NORMAL );
    return;
  }
  /**/          if( rate<=9999.9 ) { printf( " %6.1f ", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fK", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fM", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fG", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fT", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fP", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fE", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fZ", rate ); return; }
  rate *= 1e-3; if( rate<=9999.9 ) { printf( " %6.1fY", rate ); return; }
  /**/                               printf( ">9999.9Y" );
}

/* printf_pct prints to stdout:

     num_now-num_then + lhopital_num
     -------------------------------
     den_now-den_then + lhopital_den

   as a percentage.  Will be exactly 8 char wide, right justifed with
   aligned decimal point.  In the limit den_now->den_then and
   num_now->num_then (i.e. approaches -> 0/0), the result will limit to
   lhopital_num/lhopital_den (as such, lhopital_{num,den} should
   typically be picked to be a tiny non-denorms with the desired
   asymptotic limit ratio).  num and den are assumed to be monitonically
   increasing counters, lhopital_num should be a normal small-ish
   non-negative and lhopital_den should be a normal small-ish positive. */

static void
printf_pct( ulong  num_now,
            ulong  num_then,
            double lhopital_num,
            ulong  den_now,
            ulong  den_then,
            double lhopital_den ) {

  if( FD_UNLIKELY( (num_now<num_then)                              |
                   (den_now<den_then)                              |
                   !((0.<=lhopital_num) & (lhopital_num<=DBL_MAX)) |
                   !((0.< lhopital_den) & (lhopital_den<=DBL_MAX)) ) ) {
    printf( TEXT_RED " invalid" TEXT_NORMAL );
    return;
  }

  double pct = 100.*(((double)(num_now - num_then) + lhopital_num) / ((double)(den_now - den_then) + lhopital_den));

  if( FD_UNLIKELY( !((0.<=pct) & (pct<=DBL_MAX)) ) ) {
    printf( TEXT_RED "overflow" TEXT_NORMAL );
    return;
  }

  if( pct<=999.999 ) { printf( " %7.3f", pct ); return; }
  /**/                 printf( ">999.999" );
}

/**********************************************************************/

/* snap reads all the IPC diagnostics in a tguard instance and stores
   them into the easy to process structure snap */

struct snap {
  ulong pmap; /* Bit {0,1,2} set <> {cnc,mcache,fseq} values are valid */

  long  cnc_heartbeat;
  ulong cnc_signal;

  ulong cnc_diag_in_backp;
  ulong cnc_diag_backp_cnt;
  ulong cnc_diag_dedup_cnt;
  ulong cnc_diag_dedup_sz;
  ulong cnc_diag_filt_cnt;
  ulong cnc_diag_filt_sz;
  ulong cnc_diag_produce_cnt;
  ulong cnc_diag_produce_sz ;
  ulong cnc_diag_consume_cnt;
  ulong cnc_diag_consume_sz ;
  ulong cnc_diag_ingress_cnt;
  ulong cnc_diag_ingress_sz ;
  ulong cnc_diag_egress_cnt;
  ulong cnc_diag_egress_sz ;


  ulong mcache_seq;

  ulong fseq_seq;

  ulong fseq_diag_tot_cnt;
  ulong fseq_diag_tot_sz;
  ulong fseq_diag_filt_cnt;
  ulong fseq_diag_filt_sz;
  ulong fseq_diag_ovrnp_cnt;
  ulong fseq_diag_ovrnr_cnt;
  ulong fseq_diag_slow_cnt;
};

typedef struct snap snap_t;

static void
snap( ulong             tile_cnt,     /* Number of tiles to snapshot */
      snap_t *          snap_cur,     /* Snaphot for each tile, indexed [0,tile_cnt) */
      fd_cnc_t **       tile_cnc,     /* Local cnc    joins for each tile, NULL if n/a, indexed [0,tile_cnt) */
      fd_frag_meta_t ** tile_mcache,  /* Local mcache joins for each tile, NULL if n/a, indexed [0,tile_cnt) */
      ulong **          tile_fseq ) { /* Local fseq   joins for each tile, NULL if n/a, indexed [0,tile_cnt) */

  for( ulong tile_idx=0UL; tile_idx<tile_cnt; tile_idx++ ) {
    snap_t * snap = &snap_cur[ tile_idx ];

    ulong pmap = 0UL;

    fd_cnc_t const * cnc = tile_cnc[ tile_idx ];
    if( FD_LIKELY( cnc ) ) {
      snap->cnc_heartbeat = fd_cnc_heartbeat_query( cnc );
      snap->cnc_signal    = fd_cnc_signal_query   ( cnc );
      ulong const * cnc_diag = (ulong const *)fd_cnc_app_laddr_const( cnc );
      FD_COMPILER_MFENCE();
      snap->cnc_diag_in_backp    = cnc_diag[ FD_TGUARD_CNC_DIAG_IN_BACKP    ];
      snap->cnc_diag_backp_cnt   = cnc_diag[ FD_TGUARD_CNC_DIAG_BACKP_CNT   ];
      snap->cnc_diag_dedup_cnt   = cnc_diag[ FD_TGUARD_CNC_DIAG_DEDUP_CNT   ];
      snap->cnc_diag_dedup_sz    = cnc_diag[ FD_TGUARD_CNC_DIAG_DEDUP_SIZ   ];
      snap->cnc_diag_filt_cnt    = cnc_diag[ FD_TGUARD_CNC_DIAG_FILT_CNT    ];
      snap->cnc_diag_filt_sz     = cnc_diag[ FD_TGUARD_CNC_DIAG_FILT_SIZ    ];
      snap->cnc_diag_produce_cnt = cnc_diag[ FD_TGUARD_CNC_DIAG_PRODUCE_CNT ];
      snap->cnc_diag_produce_sz  = cnc_diag[ FD_TGUARD_CNC_DIAG_PRODUCE_SIZ ];
      snap->cnc_diag_consume_cnt = cnc_diag[ FD_TGUARD_CNC_DIAG_CONSUME_CNT ];
      snap->cnc_diag_consume_sz  = cnc_diag[ FD_TGUARD_CNC_DIAG_CONSUME_SIZ ];
      snap->cnc_diag_ingress_cnt = cnc_diag[ FD_TGUARD_CNC_DIAG_INGRESS_CNT ];
      snap->cnc_diag_ingress_sz  = cnc_diag[ FD_TGUARD_CNC_DIAG_INGRESS_SIZ ];
      snap->cnc_diag_egress_cnt  = cnc_diag[ FD_TGUARD_CNC_DIAG_EGRESS_CNT  ];
      snap->cnc_diag_egress_sz   = cnc_diag[ FD_TGUARD_CNC_DIAG_EGRESS_SIZ  ];
      FD_COMPILER_MFENCE();

      pmap |= 1UL;

      if (tile_idx == 2UL) { /* for tqos tile_idx==2, hack to reuse repurpose BACKP, DEDUP, and SHRED_FILT fields as mline for tqos */
        snap->mcache_seq  = snap->cnc_diag_produce_cnt;
        snap->mcache_seq -= snap->mcache_seq ? 1UL : 0UL; /* -1 to convert cnt to virtual seq */
        pmap |= 2UL;

        snap->fseq_seq  = snap->cnc_diag_consume_cnt;
        snap->fseq_seq -= snap->fseq_seq ? 1UL : 0UL; /* -1 to convert cnt to virtual seq */
        pmap |= 4UL;

        /* for tqos (tile_idx 2), FD_TGUARD_CNC_DIAG_FILT_ actually hold pkt cnt/sz tqos received from tmon */
      }
    }

    fd_frag_meta_t const * mcache = tile_mcache[ tile_idx ];
    if( FD_LIKELY( mcache ) ) {
      ulong const * seq = (ulong const *)fd_mcache_seq_laddr_const( mcache );
      snap->mcache_seq = fd_mcache_seq_query( seq );

      pmap |= 2UL;
    }

    ulong const * fseq = tile_fseq[ tile_idx ];
    if( FD_LIKELY( fseq ) ) {
      snap->fseq_seq = fd_fseq_query( fseq );
      ulong const * fseq_diag = (ulong const *)fd_fseq_app_laddr_const( fseq );
      FD_COMPILER_MFENCE();
      snap->fseq_diag_tot_cnt   = fseq_diag[ FD_FSEQ_DIAG_PUB_CNT   ];
      snap->fseq_diag_tot_sz    = fseq_diag[ FD_FSEQ_DIAG_PUB_SZ    ];
      snap->fseq_diag_filt_cnt  = fseq_diag[ FD_FSEQ_DIAG_FILT_CNT  ];
      snap->fseq_diag_filt_sz   = fseq_diag[ FD_FSEQ_DIAG_FILT_SZ   ];
      snap->fseq_diag_ovrnp_cnt = fseq_diag[ FD_FSEQ_DIAG_OVRNP_CNT ];
      snap->fseq_diag_ovrnr_cnt = fseq_diag[ FD_FSEQ_DIAG_OVRNR_CNT ];
      snap->fseq_diag_slow_cnt  = fseq_diag[ FD_FSEQ_DIAG_SLOW_CNT  ];
      FD_COMPILER_MFENCE();
      snap->fseq_diag_tot_cnt += snap->fseq_diag_filt_cnt;
      snap->fseq_diag_tot_sz  += snap->fseq_diag_filt_sz;
      pmap |= 4UL;
    }

    snap->pmap = pmap;
  }
}

int
main( int     argc,
      char ** argv ) {
  fd_boot( &argc, &argv );

  fd_tguard_mon_signal_trap( SIGTERM );
  fd_tguard_mon_signal_trap( SIGINT  );
  
  /* Parse command line arguments */

  char const * pod_gaddr =       fd_env_strip_cmdline_cstr  ( &argc, &argv, "--pod",      NULL, NULL                 );
  char const * cfg_path  =       fd_env_strip_cmdline_cstr  ( &argc, &argv, "--cfg",      NULL, NULL                 );
  long         dt_min    = (long)fd_env_strip_cmdline_double( &argc, &argv, "--dt-min",   NULL,   66666667.          );
  long         dt_max    = (long)fd_env_strip_cmdline_double( &argc, &argv, "--dt-max",   NULL, 1333333333.          );
  long         duration  = (long)fd_env_strip_cmdline_double( &argc, &argv, "--duration", NULL,          0.          );
  uint         seed      =       fd_env_strip_cmdline_uint  ( &argc, &argv, "--seed",     NULL, (uint)fd_tickcount() );

  if( FD_UNLIKELY( !pod_gaddr   ) ) FD_LOG_ERR(( "--pod not specified"                  ));
  if( FD_UNLIKELY( !cfg_path    ) ) FD_LOG_ERR(( "--cfg not specified"                  ));
  if( FD_UNLIKELY( dt_min<0L    ) ) FD_LOG_ERR(( "--dt-min should be positive"          ));
  if( FD_UNLIKELY( dt_max<dt_min) ) FD_LOG_ERR(( "--dt-max should be at least --dt-min" ));
  if( FD_UNLIKELY( duration<0L  ) ) FD_LOG_ERR(( "--duration should be non-negative"    ));

  /* Load up the configuration for this tguard instance */

  FD_LOG_INFO(( "using configuration in pod --pod %s at path --cfg %s", pod_gaddr, cfg_path ));

  uchar const * pod     = fd_wksp_pod_attach( pod_gaddr );
  uchar const * cfg_pod = fd_pod_query_subpod( pod, cfg_path );
  if( FD_UNLIKELY( !cfg_pod ) ) FD_LOG_ERR(( "path not found" ));

  ulong tile_cnt = 3UL ; /* main + tqos + tmon */

  /* Join all IPC objects for this tguard instance */

  char const **     tile_name   = fd_alloca( alignof(char const *    ), sizeof(char const *    )*tile_cnt );
  fd_cnc_t **       tile_cnc    = fd_alloca( alignof(fd_cnc_t *      ), sizeof(fd_cnc_t *      )*tile_cnt );
  fd_frag_meta_t ** tile_mcache = fd_alloca( alignof(fd_frag_meta_t *), sizeof(fd_frag_meta_t *)*tile_cnt );
  ulong **          tile_fseq   = fd_alloca( alignof(ulong *         ), sizeof(ulong *         )*tile_cnt );
  if( FD_UNLIKELY( (!tile_name) | (!tile_cnc) | (!tile_mcache) | (!tile_fseq) ) ) FD_LOG_ERR(( "fd_alloca failed" )); /* paranoia */
  
  do {
    ulong tile_idx = 0UL;

    tile_name[ tile_idx ] = "main";
    FD_LOG_INFO(( "joining %s.main.cnc", cfg_path ));
    tile_cnc[ tile_idx ] = fd_cnc_join( fd_wksp_pod_map( cfg_pod, "main.cnc" ) );
    if( FD_UNLIKELY( !tile_cnc[ tile_idx ] ) ) FD_LOG_ERR(( "fd_cnc_join failed" ));
    if( FD_UNLIKELY( fd_cnc_app_sz( tile_cnc[ tile_idx ] )<64UL ) ) FD_LOG_ERR(( "cnc app sz should be at least 64 bytes" ));
    tile_mcache[ tile_idx ] = NULL; /* main has no mcache */
    tile_fseq  [ tile_idx ] = NULL; /* main has no fseq */
    tile_idx++;
  
    tile_name[ tile_idx ] = "tmon";
    FD_LOG_INFO(( "joining %s.tmon.cnc", cfg_path ));
    tile_cnc[ tile_idx ] = fd_cnc_join( fd_wksp_pod_map( cfg_pod, "tmon.cnc" ) );
    if( FD_UNLIKELY( !tile_cnc[ tile_idx ] ) ) FD_LOG_ERR(( "fd_cnc_join failed" ));
    if( FD_UNLIKELY( fd_cnc_app_sz( tile_cnc[ tile_idx ] )<64UL ) ) FD_LOG_ERR(( "cnc app sz should be at least 64 bytes" ));
    FD_LOG_INFO(( "joining %s.tmon.mcache", cfg_path ));
    tile_mcache[ tile_idx ] = fd_mcache_join( fd_wksp_pod_map( cfg_pod, "tmon.mcache" ) );
    if( FD_UNLIKELY( !tile_mcache[ tile_idx ] ) ) FD_LOG_ERR(( "fd_mcache_join failed" ));
    FD_LOG_INFO(( "joining %s.tmon.fseq", cfg_path ));
    tile_fseq[ tile_idx ] = fd_fseq_join( fd_wksp_pod_map( cfg_pod, "tmon.fseq" ) );
    if( FD_UNLIKELY( !tile_fseq[ tile_idx ] ) ) FD_LOG_ERR(( "fd_fseq_join failed" ));
    tile_idx++;
  
    tile_name[ tile_idx ] = "tqos";
    FD_LOG_INFO(( "joining %s.tqos.cnc", cfg_path ));
    tile_cnc[ tile_idx ] = fd_cnc_join( fd_wksp_pod_map( cfg_pod, "tqos.cnc" ) );
    if( FD_UNLIKELY( !tile_cnc[ tile_idx ] ) ) FD_LOG_ERR(( "fd_cnc_join failed" ));
    if( FD_UNLIKELY( fd_cnc_app_sz( tile_cnc[ tile_idx ] )<64UL ) ) FD_LOG_ERR(( "cnc app sz should be at least 64 bytes" ));
    tile_mcache[ tile_idx ] = NULL; /* tqos has no mcache */
    tile_fseq  [ tile_idx ] = NULL; /* tqos has no fseq */
    tile_idx++; 
  } while(0);
  
  /* Setup local objects used by this app */

  fd_rng_t _rng[1];
  fd_rng_t * rng = fd_rng_join( fd_rng_new( _rng, seed, 0UL ) );

  snap_t * snap_prv = (snap_t *)fd_alloca( alignof(snap_t), sizeof(snap_t)*2UL*tile_cnt );
  if( FD_UNLIKELY( !snap_prv ) ) FD_LOG_ERR(( "fd_alloca failed" )); /* Paranoia */
  snap_t * snap_cur = snap_prv + tile_cnt;

  /* Get the inital reference diagnostic snapshot */

  snap( tile_cnt, snap_prv, tile_cnc, tile_mcache, tile_fseq );
  long then; long tic; fd_tempo_observe_pair( &then, &tic );

  /* Monitor for duration ns.  Note that for duration==0, this
     will still do exactly one pretty print. */

  FD_LOG_NOTICE(( "monitoring --dt-min %li ns, --dt-max %li ns, --duration %li ns, --seed %u", dt_min, dt_max, duration, seed ));

  double ns_per_tic = 1./fd_tempo_tick_per_ns( NULL ); /* calibrate during first wait */

  long stop = then + duration;
  for(;;) {

    /* Wait a somewhat randomized amount and then make a diagnostic
       snapshot */

    fd_log_wait_until( then + dt_min + (long)fd_rng_ulong_roll( rng, 1UL+(ulong)(dt_max-dt_min) ) );

    snap( tile_cnt, snap_cur, tile_cnc, tile_mcache, tile_fseq );
    long now; long toc; fd_tempo_observe_pair( &now, &toc );
    
    /* Pretty print a comparison between this diagnostic snapshot and
       the previous one. */

    /* FIXME: CONSIDER DOING CNC ACKS AND INCL IN SNAPSHOT */
    /* FIXME: CONSIDER INCLUDING TILE UPTIME */
    /* FIXME: CONSIDER ADDING INFO LIKE PID OF INSTANCE */

    /* ensure display from bottom so no change for next cnc run */
    for (ulong i=128; i; i--) printf("\n"); 

    char now_cstr[ FD_LOG_WALLCLOCK_CSTR_BUF_SZ ];
    printf("\n");
    printf( "snapshot for %s\n", fd_log_wallclock_cstr( now, now_cstr ) );
    printf( "  tile "
      "|      stale "
      "| heart "
      "|        sig "
      "| in backp "
      "|           backp cnt "
      "|      shred_filt cnt "
      "|     shred_dedup cnt "
      "|                    tx seq "
      "|                    rx seq\n" 
    );
    printf( "-------"
      "+------------"
      "+-------"
      "+------------"
      "+----------"
      "+---------------------"
      "+---------------------"
      "+---------------------"
      "+---------------------------"
      "+---------------------------\n"
    );
    for( ulong tile_idx=0UL; tile_idx<tile_cnt; tile_idx++ ) {
      snap_t * prv = &snap_prv[ tile_idx ];
      snap_t * cur = &snap_cur[ tile_idx ];
      
      /* col: tile */
      printf( " %5s", tile_name[ tile_idx ] );

      /* col: | stale | heart | sig | in backp | backp cnt | sv_filt cn | */
      if( FD_LIKELY( cur->pmap & 1UL ) ) {
        /* co: | stale */
        printf( " | " ); printf_stale   ( (long)(0.5+ns_per_tic*(double)(toc - cur->cnc_heartbeat)), dt_min );

        /* co: | heart */
        printf( " | " ); printf_heart   ( cur->cnc_heartbeat,        prv->cnc_heartbeat        );

        /* co: | sig */
        printf( " | " ); printf_sig     ( cur->cnc_signal,           prv->cnc_signal           );

        /* co: | in backp */
        printf( " | " ); printf_err_bool( cur->cnc_diag_in_backp,    prv->cnc_diag_in_backp    );

        /* co: | backp cnt */
        printf( " | " ); printf_err_cnt ( cur->cnc_diag_backp_cnt,   prv->cnc_diag_backp_cnt   );

        /* co: | shred_filt cnt */
        printf( " | " ); printf_err_cnt ( cur->cnc_diag_filt_cnt, prv->cnc_diag_filt_cnt );

        /* co: | shred_dedup cnt */
        printf( " | " ); printf_err_cnt ( cur->cnc_diag_dedup_cnt, prv->cnc_diag_dedup_cnt );
      } 
      else {
        printf( " "       
          "|          - "
          "|     - "
          "|          - "
          "|        - "
          "|        - "
          "|                   -"
          "|                   -" );
      }

      /* col: | tx seq |*/
      if( FD_LIKELY( cur->pmap & 2UL ) ) {
        printf( " | " ); printf_seq( cur->mcache_seq, prv->mcache_seq );
      } else {
        printf( " |                         -" );
      }

      /* col: | rx seq  */
      if( FD_LIKELY( cur->pmap & 4UL ) ) {
        printf( " | " ); printf_seq( cur->fseq_seq, prv->fseq_seq );
      } else {
        printf( " |                         -" );
      }

      printf( "\n" );
    }
    printf( "\n" );
    printf( "         link |  in PPS  |  in bps  |  out PPS |  out bps |  o/i PPS |  o/i bps |           ovrnp cnt |           ovrnr cnt |            slow cnt\n" );
    printf( "--------------+----------+----------+----------+----------+----------+----------+---------------------+---------------------+---------------------\n" );
    /* 0: main  1: tmon  2: tqos */
    for( ulong tile_idx=1UL; tile_idx<tile_cnt; tile_idx++ ) {
      printf( " ->%6s -> ", tile_name[tile_idx] );

      long dt = now-then;
      snap_t * prv = &snap_prv[ tile_idx ];
      snap_t * cur = &snap_cur[ tile_idx ];

      ulong cur_ingress_cnt = cur->cnc_diag_ingress_cnt;
      ulong cur_ingress_sz  = cur->cnc_diag_ingress_sz;
      ulong prv_ingress_cnt = prv->cnc_diag_ingress_cnt;
      ulong prv_ingress_sz  = prv->cnc_diag_ingress_sz;

      ulong cur_egress_cnt = cur->cnc_diag_egress_cnt;
      ulong cur_egress_sz  = cur->cnc_diag_egress_sz;
      ulong prv_egress_cnt = prv->cnc_diag_egress_cnt;
      ulong prv_egress_sz  = prv->cnc_diag_egress_sz;

      /* tqos tile (2) has no fseq, so create the equiv. */
      ulong cur_ovrnp_cnt = tile_idx == 2 ? 
        snap_cur[1].cnc_diag_egress_cnt - snap_cur[2].cnc_diag_egress_cnt :
        cur->fseq_diag_ovrnp_cnt;
      ulong prv_ovrnp_cnt = tile_idx == 2 ? 
        snap_prv[1].cnc_diag_egress_cnt - snap_prv[2].cnc_diag_egress_cnt : 
        prv->fseq_diag_ovrnp_cnt;
      ulong cur_ovrnr_cnt = tile_idx == 2 ? 0 : cur->fseq_diag_ovrnr_cnt;
      ulong prv_ovrnr_cnt = tile_idx == 2 ? 0 : prv->fseq_diag_ovrnr_cnt;
      ulong cur_slow_cnt  = tile_idx == 2 ? 0 : cur->fseq_diag_slow_cnt;
      ulong prv_slow_cnt  = tile_idx == 2 ? 0 : prv->fseq_diag_slow_cnt;

      /* in PPS */
      printf( " | " ); printf_rate( 1e9, 0., cur_ingress_cnt, prv_ingress_cnt, dt );
      
      /* in bps */
      printf( " | " ); printf_rate( 8e9, 0., cur_ingress_sz, prv_ingress_sz, dt );

      /* out PPS */
      printf( " | " ); printf_rate( 1e9, 0., cur_egress_cnt, prv_egress_cnt, dt );
      
      /* out bps */
      printf( " | " ); printf_rate( 8e9, 0., cur_egress_sz, prv_egress_sz, dt ); 

      /* o/i PPS */
      printf( " | " ); printf_pct ( cur_egress_cnt,  prv_egress_cnt,   0.,
                                    cur_ingress_cnt, prv_ingress_cnt,  DBL_MIN );
      
      /* o/i bps */
      printf( " | " ); printf_pct ( cur_egress_sz,  prv_egress_sz,   0.,
                                    cur_ingress_sz, prv_ingress_sz,  DBL_MIN ); 

      printf( " | " ); printf_err_cnt( cur_ovrnp_cnt, prv_ovrnp_cnt );
      printf( " | " ); printf_err_cnt( cur_ovrnr_cnt, prv_ovrnr_cnt );
      printf( " | " ); printf_err_cnt( cur_slow_cnt,  prv_slow_cnt  );
      printf( "\n" );
    }
    printf( "\n" );

    /* Stop once we've been monitoring for duration ns */

    if( FD_UNLIKELY( (now-stop)>=0L ) ) break;

    /* Still more monitoring to do ... wind up for the next iteration by
       swaping the two snap arrays. */

    then = now; tic = toc;
    snap_t * tmp = snap_prv; snap_prv = snap_cur; snap_cur = tmp;
  }

  /* Monitoring done ... clean up */

  FD_LOG_NOTICE(( "cleaning up" ));
  fd_rng_delete( fd_rng_leave( rng ) );
  for( ulong tile_idx=tile_cnt; tile_idx; tile_idx-- ) {
    if( FD_LIKELY( tile_fseq  [ tile_idx-1UL ] ) ) fd_wksp_pod_unmap( fd_fseq_leave  ( tile_fseq  [ tile_idx-1UL ] ) );
    if( FD_LIKELY( tile_mcache[ tile_idx-1UL ] ) ) fd_wksp_pod_unmap( fd_mcache_leave( tile_mcache[ tile_idx-1UL ] ) );
    if( FD_LIKELY( tile_cnc   [ tile_idx-1UL ] ) ) fd_wksp_pod_unmap( fd_cnc_leave   ( tile_cnc   [ tile_idx-1UL ] ) );
  }
  fd_wksp_pod_detach( pod );
  fd_halt();
  return 0;
}

#else

int
main( int     argc,
      char ** argv ) {
  fd_boot( &argc, &argv );
  FD_LOG_ERR(( "unsupported for this build target" ));
  fd_halt();
  return 0;
}

#endif

