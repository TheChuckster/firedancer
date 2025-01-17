#include "fd_exec_sol_compat.h"
#include "../../nanopb/pb_encode.h"
#include "../../nanopb/pb_decode.h"
#include "generated/elf.pb.h"
#include "generated/invoke.pb.h"
#include "generated/vm.pb.h"
#include <assert.h>
#include <stdlib.h>
#include "../../vm/fd_vm.h"
#include "fd_vm_validate_test.h"

/* This file defines stable APIs for compatibility testing.

   For the "compat" shared library used by the differential fuzzer,
   ideally the symbols defined in this file would be the only visible
   globals.  Unfortunately, we currently export all symbols, which leads
   to great symbol table bloat from fd_types.c. */

typedef struct {
  ulong   struct_size;
  ulong * cleaned_up_features;
  ulong   cleaned_up_feature_cnt;
  ulong * supported_features;
  ulong   supported_feature_cnt;
} sol_compat_features_t;

static sol_compat_features_t features;
static ulong cleaned_up_features[] =
  { 0xd924059c5749c4c1,  // secp256k1_program_enabled
    0x8f688d4e3ab17a60,  // enable_early_verification_of_account_modifications
    0x50a615bae8ca3874,  // native_programs_consume_cu
    0x65b79c7f3e7441b3,  // require_custodian_for_locked_stake_authorize
  };

static ulong supported_features[] =
  { 0xe8f97382b03240a1,  // system_transfer_zero_check
    0x10a1e092dd7f1573,  // dedupe_config_program_signers
    0xfba69c4970d7ad9d,  // vote_stake_checked_instructions
    0x65b79c7f3e7441b3,  // require_custodian_for_locked_stake_authorize
    0x74b022574093eeec,  // reduce_stake_warmup_cooldown
    0x6d22c4ce75df6f0b,  // stake_merge_with_unmatched_credits_observed
    0x4b241cb4c6f3b3b2,  // require_rent_exempt_split_destination
    0x74326f811fd7d861,  // vote_state_add_vote_latency
    0x86fa44f01141c71a,  // timely_vote_credits
    0xe9d32123513c4d0d,  // timely_vote_credits
    0x5795654d01457757,  // vote_authorize_with_seed
    0x8a8eb9085ca2bb0b,  // commission_updates_only_allowed_in_first_half_of_epoch
    0x7bc99a080444c8d9,  // allow_votes_to_directly_update_vote_state
    0x2ca5833736ba5c69,  // compact_vote_state_updates
    0x7e787d5c6d662d23,  // reject_callx_r10
    0x0b9047b5bb9ef961,  // move_stake_and_move_lamports_ixs
  };

static       uchar *     smem;
static const ulong       smax = 1UL<<30;
static       fd_wksp_t * wksp = NULL;

#define WKSP_TAG 2

void
sol_compat_init( void ) {
  assert( !smem );

  int argc = 1;
  char * argv[2] = { (char *)"fd_exec_sol_compat", NULL };
  char ** argv_ = argv;
  setenv( "FD_LOG_PATH", "", 1 );
  fd_boot( &argc, &argv_ );
  fd_log_level_logfile_set(5);
  fd_log_level_core_set(4);  /* abort on FD_LOG_ERR */

  sol_compat_wksp_init();
}

void
sol_compat_wksp_init( void ) {
  ulong cpu_idx = fd_tile_cpu_id( fd_tile_idx() );
  if( cpu_idx>=fd_shmem_cpu_cnt() ) cpu_idx = 0UL;
  wksp = fd_wksp_new_anonymous( FD_SHMEM_NORMAL_PAGE_SZ, 65536, fd_shmem_cpu_idx( fd_shmem_numa_idx( cpu_idx ) ), "wksp", 0UL );
  assert( wksp );

  smem = malloc( smax );  /* 1 GiB */
  assert( smem );

  features.struct_size           = sizeof(sol_compat_features_t);
  features.cleaned_up_features    = cleaned_up_features;
  features.cleaned_up_feature_cnt = sizeof(cleaned_up_features)/sizeof(ulong);
  features.supported_features    = supported_features;
  features.supported_feature_cnt = sizeof(supported_features)/sizeof(ulong);
}

void
sol_compat_fini( void ) {
  fd_wksp_delete_anonymous( wksp );
  free( smem );
  wksp = NULL;
  smem = NULL;
}

void
sol_compat_check_wksp_usage( void ) {
  fd_wksp_usage_t usage[1];
  ulong tags[1] = { WKSP_TAG };
  fd_wksp_usage( wksp, tags, 1, usage );
  if( usage->used_sz ) {
    FD_LOG_ERR(( "%lu bytes leaked in %lu allocations", usage->used_sz, usage->used_cnt ));
  }
}

fd_exec_instr_test_runner_t *
sol_compat_setup_scratch_and_runner( void * fmem ) {
  // Setup scratch
  fd_scratch_attach( smem, fmem, smax, 64UL );

  // Setup test runner
  void * runner_mem = fd_wksp_alloc_laddr( wksp, fd_exec_instr_test_runner_align(), fd_exec_instr_test_runner_footprint(), WKSP_TAG );
  fd_exec_instr_test_runner_t * runner = fd_exec_instr_test_runner_new( runner_mem, WKSP_TAG );

  return runner;
}

void
sol_compat_cleanup_scratch_and_runner( fd_exec_instr_test_runner_t * runner ) {
  /* Cleanup test runner */
  fd_wksp_free_laddr( fd_exec_instr_test_runner_delete( runner ) );
  /* Cleanup scratch */
  fd_scratch_detach( NULL );
}

void *
sol_compat_decode( void *               decoded,
                   uchar const *        in,
                   ulong                in_sz,
                   pb_msgdesc_t const * decode_type ) {
  pb_istream_t istream = pb_istream_from_buffer( in, in_sz );
  int decode_ok = pb_decode_ex( &istream, decode_type, decoded, PB_DECODE_NOINIT );
  if( !decode_ok ) {
    pb_release( decode_type, decoded );
    return NULL;
  }
  return decoded;
}

void const *
sol_compat_encode( uchar *              out,
                   ulong *              out_sz,
                   void const *         to_encode,
                   pb_msgdesc_t const * encode_type ) {
  pb_ostream_t ostream = pb_ostream_from_buffer( out, *out_sz );
  int encode_ok = pb_encode( &ostream, encode_type, to_encode );
  if( !encode_ok ) {
    return NULL;
  }
  *out_sz = ostream.bytes_written;
  return to_encode;
}

typedef ulong( exec_test_run_fn_t )( fd_exec_instr_test_runner_t *,
                                     void const *,
                                     void **,
                                     void *,
                                     ulong );

void
sol_compat_execute_wrapper( fd_exec_instr_test_runner_t * runner,
                            void * input,
                            void ** output,
                            exec_test_run_fn_t * exec_test_run_fn ) {
  // fd_scratch_push();
  do {
    ulong out_bufsz = 100000000;  /* 100 MB */
    void * out0 = fd_scratch_prepare( 1UL );
    assert( out_bufsz < fd_scratch_free() );
    fd_scratch_publish( (void *)( (ulong)out0 + out_bufsz ) );
    ulong out_used = exec_test_run_fn( runner, input, output, out0, out_bufsz );
    if( FD_UNLIKELY( !out_used ) ) {
      *output = NULL;
      break;
    }
  } while(0);
  // fd_scratch_pop();
}

/*
 * fixtures
 */

int
sol_compat_cmp_binary_strict( void const * effects,
                              void const * expected,
                              pb_msgdesc_t const * encode_type ) {
#define MAX_SZ 1024*1024
  if( effects==NULL ) {
    FD_LOG_WARNING(( "No output effects" ));
    return 0;
  }

  ulong out_sz = MAX_SZ;
  uchar out[MAX_SZ];
  if( !sol_compat_encode( out, &out_sz, effects, encode_type ) ) {
    FD_LOG_WARNING(( "Error encoding effects" ));
    return 0;
  }

  ulong exp_sz = MAX_SZ;
  uchar exp[MAX_SZ];
  if( !sol_compat_encode( exp, &exp_sz, expected, encode_type ) ) {
    FD_LOG_WARNING(( "Error encoding expected" ));
    return 0;
  }

  if( out_sz!=exp_sz ) {
    FD_LOG_WARNING(( "Binary cmp failed: different size. out_sz=%lu exp_sz=%lu", out_sz, exp_sz  ));
    return 0;
  }
  if( !fd_memeq( out, exp, out_sz ) ) {
    FD_LOG_WARNING(( "Binary cmp failed: different values." ));
    return 0;
  }

  return 1;
}

int
sol_compat_cmp_success_fail_only( void const * _effects,
                                  void const * _expected ) {
  fd_exec_test_instr_effects_t * effects  = (fd_exec_test_instr_effects_t *)_effects;
  fd_exec_test_instr_effects_t * expected = (fd_exec_test_instr_effects_t *)_expected;

  if( effects==NULL ) {
    FD_LOG_WARNING(( "No output effects" ));
    return 0;
  }

  if( effects->custom_err || expected->custom_err ) {
    FD_LOG_WARNING(( "Unexpected custom error" ));
    return 0;
  }

  int res = effects->result;
  int exp = expected->result;

  if( res==exp ) {
    return 1;
  }

  if( res>0 && exp>0 ) {
    FD_LOG_INFO(( "Accepted: res=%d exp=%d", res, exp ));
    return 1;
  }

  return 0;
}

int
sol_compat_instr_fixture( fd_exec_instr_test_runner_t * runner,
                          uchar const *                 in,
                          ulong                         in_sz ) {
  // Decode fixture
  fd_exec_test_instr_fixture_t fixture[1] = {0};
  void * res = sol_compat_decode( &fixture, in, in_sz, &fd_exec_test_instr_fixture_t_msg );
  if ( res==NULL ) {
    FD_LOG_WARNING(( "Invalid instr fixture." ));
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, &fixture->input, &output, (exec_test_run_fn_t *)fd_exec_instr_test_run );

  // Compare effects
  int ok = sol_compat_cmp_binary_strict( output, &fixture->output, &fd_exec_test_instr_effects_t_msg );

  // Cleanup
  pb_release( &fd_exec_test_instr_fixture_t_msg, fixture );
  return ok;
}

int
sol_compat_precompile_fixture( fd_exec_instr_test_runner_t * runner,
                               uchar const *                 in,
                               ulong                         in_sz ) {
  // Decode fixture
  fd_exec_test_instr_fixture_t fixture[1] = {0};
  void * res = sol_compat_decode( &fixture, in, in_sz, &fd_exec_test_instr_fixture_t_msg );
  if ( res==NULL ) {
    FD_LOG_WARNING(( "Invalid instr fixture." ));
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, &fixture->input, &output, (exec_test_run_fn_t *)fd_exec_instr_test_run );

  // Compare effects
  int ok = sol_compat_cmp_success_fail_only( output, &fixture->output );

  // Cleanup
  pb_release( &fd_exec_test_instr_fixture_t_msg, fixture );
  return ok;
}

int
sol_compat_elf_loader_fixture( fd_exec_instr_test_runner_t * runner,
                               uchar const *                 in,
                               ulong                         in_sz ) {
  // Decode fixture
  fd_exec_test_elf_loader_fixture_t fixture[1] = {0};
  void * res = sol_compat_decode( &fixture, in, in_sz, &fd_exec_test_elf_loader_fixture_t_msg );
  if ( res==NULL ) {
    FD_LOG_WARNING(( "Invalid elf_loader fixture." ));
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, &fixture->input, &output, (exec_test_run_fn_t *)fd_sbpf_program_load_test_run );

  // Compare effects
  int ok = sol_compat_cmp_binary_strict( output, &fixture->output, &fd_exec_test_elf_loader_effects_t_msg );

  // Cleanup
  pb_release( &fd_exec_test_elf_loader_fixture_t_msg, fixture );
  return ok;
}

int
sol_compat_syscall_fixture( fd_exec_instr_test_runner_t * runner,
                            uchar const *                 in,
                            ulong                         in_sz ) {
  // Decode fixture
  fd_exec_test_syscall_fixture_t fixture[1] = {0};
  if ( !sol_compat_decode( &fixture, in, in_sz, &fd_exec_test_syscall_fixture_t_msg ) ) {
    FD_LOG_WARNING(( "Invalid syscall fixture." ));
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, &fixture->input, &output, (exec_test_run_fn_t *)fd_exec_vm_syscall_test_run );

  // Compare effects
  int ok = sol_compat_cmp_binary_strict( output, &fixture->output, &fd_exec_test_syscall_effects_t_msg );

  // Cleanup
  pb_release( &fd_exec_test_syscall_fixture_t_msg, fixture );
  return ok;
}

int 
sol_compat_validate_vm_fixture( fd_exec_instr_test_runner_t * runner,
                                uchar const *                 in,
                                ulong                         in_sz ) {
  // Decode fixture
  fd_exec_test_validate_vm_fixture_t fixture[1] = {0};
  if( !sol_compat_decode( &fixture, in, in_sz, &fd_exec_test_validate_vm_fixture_t_msg ) ) {
    FD_LOG_WARNING(( "Invalid validate_vm fixture." ));
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner,
                              &fixture->input,
                              &output, 
                              (exec_test_run_fn_t *)fd_exec_vm_validate_test_run );
  // Compare effects
  int ok = sol_compat_cmp_binary_strict( output, &fixture->output, &fd_exec_test_validate_vm_effects_t_msg );

  // Cleanup
  pb_release( &fd_exec_test_validate_vm_fixture_t_msg, fixture );
  return ok;
}

/*
 * execute_v1
 */

int
sol_compat_instr_execute_v1( uchar *       out,
                             ulong *       out_sz,
                             uchar const * in,
                             ulong         in_sz ) {
  // Setup
  ulong fmem[ 64 ];
  fd_exec_instr_test_runner_t * runner = sol_compat_setup_scratch_and_runner( fmem );

  // Decode context
  fd_exec_test_instr_context_t input[1] = {0};
  void * res = sol_compat_decode( &input, in, in_sz, &fd_exec_test_instr_context_t_msg );
  if ( res==NULL ) {
    sol_compat_cleanup_scratch_and_runner( runner );
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, input, &output, (exec_test_run_fn_t *)fd_exec_instr_test_run );

  // Encode effects
  int ok = 0;
  if( output ) {
    ok = !!sol_compat_encode( out, out_sz, output, &fd_exec_test_instr_effects_t_msg );
  }

  // Cleanup
  pb_release( &fd_exec_test_instr_context_t_msg, input );
  sol_compat_cleanup_scratch_and_runner( runner );

  // Check wksp usage is 0
  sol_compat_check_wksp_usage();

  return ok;
}

int
sol_compat_txn_execute_v1( uchar *       out,
                           ulong *       out_sz,
                           uchar const * in,
                           ulong         in_sz ) {
  // Setup
  ulong fmem[ 64 ];
  fd_exec_instr_test_runner_t * runner = sol_compat_setup_scratch_and_runner( fmem );

  // Decode context
  fd_exec_test_txn_context_t input[1] = {0};
  void * res = sol_compat_decode( &input, in, in_sz, &fd_exec_test_txn_context_t_msg );
  if ( res==NULL ) {
    sol_compat_cleanup_scratch_and_runner( runner );
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, input, &output, (exec_test_run_fn_t *)fd_exec_txn_test_run );

  // Encode effects
  int ok = 0;
  if( output ) {
    ok = !!sol_compat_encode( out, out_sz, output, &fd_exec_test_txn_result_t_msg );
  }

  // Cleanup
  pb_release( &fd_exec_test_txn_context_t_msg, input );
  sol_compat_cleanup_scratch_and_runner( runner );

  // Check wksp usage is 0
  sol_compat_check_wksp_usage();
  return ok;
}

int
sol_compat_elf_loader_v1( uchar *       out,
                          ulong *       out_sz,
                          uchar const * in,
                          ulong         in_sz ) {
  ulong fmem[ 64 ];
  fd_scratch_attach( smem, fmem, smax, 64UL );
  fd_scratch_push();

  pb_istream_t istream = pb_istream_from_buffer( in, in_sz );
  fd_exec_test_elf_loader_ctx_t input[1] = {0};
  int decode_ok = pb_decode_ex( &istream, &fd_exec_test_elf_loader_ctx_t_msg, input, PB_DECODE_NOINIT );
  if( !decode_ok ) {
    pb_release( &fd_exec_test_elf_loader_ctx_t_msg, input );
    return 0;
  }

  fd_exec_test_elf_loader_effects_t * output = NULL;
  do {
    ulong out_bufsz = 100000000;
    void * out0 = fd_scratch_prepare( 1UL );
    assert( out_bufsz < fd_scratch_free() );
    fd_scratch_publish( (void *)( (ulong)out0 + out_bufsz ) );
    ulong out_used = fd_sbpf_program_load_test_run( NULL, input, &output, out0, out_bufsz );
    if( FD_UNLIKELY( !out_used ) ) {
      output = NULL;
      break;
    }
  } while(0);

  int ok = 0;

  if( output ) {
    pb_ostream_t ostream = pb_ostream_from_buffer( out, *out_sz );
    int encode_ok = pb_encode( &ostream, &fd_exec_test_elf_loader_effects_t_msg, output );
    if( encode_ok ) {
      *out_sz = ostream.bytes_written;
      ok = 1;
    }
  }

  pb_release( &fd_exec_test_elf_loader_ctx_t_msg, input );
  fd_scratch_pop();
  fd_scratch_detach( NULL );

  // Check wksp usage is 0
  sol_compat_check_wksp_usage();

  return ok;
}

sol_compat_features_t const *
sol_compat_get_features_v1( void ) {
  return &features;
}

int
sol_compat_vm_syscall_execute_v1( uchar *       out,
                                  ulong *       out_sz,
                                  uchar const * in,
                                  ulong         in_sz ) {
  // Setup
  ulong fmem[ 64 ];
  fd_exec_instr_test_runner_t * runner = sol_compat_setup_scratch_and_runner( fmem );

  // Decode context
  fd_exec_test_syscall_context_t input[1] = {0};
  void * res = sol_compat_decode( &input, in, in_sz, &fd_exec_test_syscall_context_t_msg );
  if ( res==NULL ) {
    sol_compat_cleanup_scratch_and_runner( runner );
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, input, &output, (exec_test_run_fn_t *)fd_exec_vm_syscall_test_run );

  // Encode effects
  int ok = 0;
  if( output ) {
    ok = !!sol_compat_encode( out, out_sz, output, &fd_exec_test_syscall_effects_t_msg );
  }

  // Cleanup
  pb_release( &fd_exec_test_syscall_context_t_msg, input );
  sol_compat_cleanup_scratch_and_runner( runner );

  // Check wksp usage is 0
  sol_compat_check_wksp_usage();

  return ok;
}

int
sol_compat_vm_validate_v1(  uchar *       out,
                            ulong *       out_sz,
                            uchar const * in,
                            ulong         in_sz) {
  // Setup
  ulong fmem[ 64 ];
  fd_exec_instr_test_runner_t * runner = sol_compat_setup_scratch_and_runner( fmem );

  // Decode context
  fd_exec_test_full_vm_context_t input[1] = {0};
  void * res = sol_compat_decode( &input, in, in_sz, &fd_exec_test_full_vm_context_t_msg );
  if ( res==NULL ) {
    sol_compat_cleanup_scratch_and_runner( runner );
    return 0;
  }

  // Execute
  void * output = NULL;
  sol_compat_execute_wrapper( runner, input, &output, (exec_test_run_fn_t *)fd_exec_vm_validate_test_run );

  // Encode effects
  int ok = 0;
  if( output ) {
    ok = !!sol_compat_encode( out, out_sz, output, &fd_exec_test_validate_vm_effects_t_msg );
  }

  // cleanup
  pb_release( &fd_exec_test_full_vm_context_t_msg, input );
  sol_compat_cleanup_scratch_and_runner( runner );

  // Check wksp usage is 0
  sol_compat_check_wksp_usage();

  return ok;
}
