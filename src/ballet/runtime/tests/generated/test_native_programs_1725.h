#include "../fd_tests.h"
int test_1725(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 47;
  test.bt = "   2: solana_vote_program::vote_processor::tests::test_vote_process_instruction_decode_bail             at ./src/vote_processor.rs:511:9   3: solana_vote_program::vote_processor::tests::test_vote_process_instruction_decode_bail::{{closure}}             at ./src/vote_processor.rs:510:5   4: core::ops::function::FnOnce::call_once             at /rustc/0677edc86e342f333d4828b0ee1ef395a4e70fe5/library/core/src/ops/function.rs:227:5   5: core::ops::function::FnOnce::call_once             at /rustc/0677edc86e342f333d4828b0ee1ef395a4e70fe5/library/core/src/ops/function.rs:227:5";
  test.test_name = "vote_processor::tests::test_vote_process_instruction_decode_bail";
  test.test_number = 1725;
  if (fd_executor_test_suite_check_filter(suite, &test)) return -9999;
  ulong test_accs_len = 0;
  fd_executor_test_acc_t* test_accs = fd_alloca( 1UL, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );
  fd_memset( test_accs, 0, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );


  uchar disabled_features[] = { 103,105,106,108,109,110,111,112,113,114,116,117,118,120,121,122,123,124,125,126,127,128,15,2,24,26,27,29,30,33,55,56,61,62,75,76,77,78,79,80,82,83,87,89,90,92,98 };
  test.disable_feature = disabled_features;
            
  fd_base58_decode_32( "Vote111111111111111111111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_1725_raw[] = { 0x00,0x00,0x00,0x01,0x01,0x07,0x61,0x48,0x1d,0x35,0x74,0x74,0xbb,0x7c,0x4d,0x76,0x24,0xeb,0xd3,0xbd,0xb3,0xd8,0x35,0x5e,0x73,0xd1,0x10,0x43,0xfc,0x0d,0xa3,0x53,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00 };
  test.raw_tx = fd_flamenco_native_prog_test_1725_raw;
  test.raw_tx_len = 73UL;
  test.expected_result = -20;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}
