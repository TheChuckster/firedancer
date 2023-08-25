#include "../fd_tests.h"
int test_2417(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 1;
  test.bt = "   2: solana_stake_program::stake_instruction::tests::test_stake_process_instruction_error_ordering             at ./src/stake_instruction.rs:6571:13   3: solana_stake_program::stake_instruction::tests::test_stake_process_instruction_error_ordering::old_warmup_cooldown             at ./src/stake_instruction.rs:6512:5   4: solana_stake_program::stake_instruction::tests::test_stake_process_instruction_error_ordering::old_warmup_cooldown::{{closure}}             at ./src/stake_instruction.rs:6512:5   5: core::ops::function::FnOnce::call_once             at /rustc/83964c156db1f444050a38b2498dbd0da6d5d503/library/core/src/ops/function.rs:250:5";
  test.test_name = "stake_instruction::tests::test_stake_process_instruction_error_ordering::old_warmup_cooldown";
  test.test_number = 2417;
  test.sysvar_cache.clock = "";
  test.sysvar_cache.epoch_schedule = "";
  test.sysvar_cache.epoch_rewards = "";
  test.sysvar_cache.fees = "";
  test.sysvar_cache.rent = "";
  test.sysvar_cache.slot_hashes = "";
  test.sysvar_cache.stake_history = "";
  test.sysvar_cache.slot_history = "";
  if (fd_executor_test_suite_check_filter(suite, &test)) return -9999;
  ulong test_accs_len = 0;
  fd_executor_test_acc_t* test_accs = fd_alloca( 1UL, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );
  fd_memset( test_accs, 0, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );


  uchar disabled_features[] = { 160 };
  test.disable_feature = disabled_features;
            
 // {'clock': '', 'epoch_schedule': '', 'epoch_rewards': '', 'fees': '', 'rent': '', 'slot_hashes': '', 'recent_blockhashes': '', 'stake_history': '', 'last_restart_slot': ''}
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_2417_raw[] = { 0x00,0x00,0x00,0x01,0x01,0x06,0xa1,0xd8,0x17,0x91,0x37,0x54,0x2a,0x98,0x34,0x37,0xbd,0xfe,0x2a,0x7a,0xb2,0x55,0x7f,0x53,0x5c,0x8a,0x78,0x72,0x2b,0x68,0xa4,0x9d,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff };
  test.raw_tx = fd_flamenco_native_prog_test_2417_raw;
  test.raw_tx_len = 81UL;
  test.expected_result = -3;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}
