#include "../fd_tests.h"
int test_51(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 0;
  test.bt = "   2: solana_bpf_loader_program::tests::test_bpf_loader_upgradeable_set_upgrade_authority_checked             at ./src/lib.rs:3265:9   3: solana_bpf_loader_program::tests::test_bpf_loader_upgradeable_set_upgrade_authority_checked::{{closure}}             at ./src/lib.rs:3060:68   4: core::ops::function::FnOnce::call_once             at /rustc/84c898d65adf2f39a5a98507f1fe0ce10a2b8dbc/library/core/src/ops/function.rs:250:5   5: core::ops::function::FnOnce::call_once             at /rustc/84c898d65adf2f39a5a98507f1fe0ce10a2b8dbc/library/core/src/ops/function.rs:250:5";
  test.test_name = "tests::test_bpf_loader_upgradeable_set_upgrade_authority_checked";
  test.test_number = 51;
  test.sysvar_cache.clock = "";
  test.sysvar_cache.epoch_schedule = "";
  test.sysvar_cache.epoch_rewards = "";
  test.sysvar_cache.fees = "";
  test.sysvar_cache.rent = "";
  test.sysvar_cache.slot_hashes = "";
  test.sysvar_cache.stake_history = "";
  test.sysvar_cache.slot_history = "";
  if (fd_executor_test_suite_check_filter(suite, &test)) return -9999;
  ulong test_accs_len = 2;
  fd_executor_test_acc_t* test_accs = fd_alloca( 1UL, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );
  fd_memset( test_accs, 0, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );

 // {'clock': '', 'epoch_schedule': '', 'epoch_rewards': '', 'fees': '', 'rent': '', 'slot_hashes': '', 'recent_blockhashes': '', 'stake_history': '', 'last_restart_slot': ''}
  fd_executor_test_acc_t* test_acc = test_accs;
  fd_base58_decode_32( "BvnuVZhsueeSkDaiuDdyYLfYPF4auP4F7pQnVbLAsY4n",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "BPFLoaderUpgradeab1e11111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "BPFLoaderUpgradeab1e11111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 1UL;
  test_acc->result_lamports = 1UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_51_acc_0_data[] = { 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_51_acc_0_data;
  test_acc->data_len        = 45UL;
  static uchar const fd_flamenco_native_prog_test_51_acc_0_post_data[] = { 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_51_acc_0_post_data;
  test_acc->result_data_len = 45UL;
  test_acc++;
  fd_base58_decode_32( "1111113XU2ZQXVsSffvgmkXuiF1We5N6cnMnFLE8X",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "1111113XsNAPdoA9r8oWCtcw8aKzmM9j83JVUv4Ss",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "1111113XsNAPdoA9r8oWCtcw8aKzmM9j83JVUv4Ss",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 1UL;
  test_acc->result_lamports = 1UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  test_acc++;
  fd_base58_decode_32( "BPFLoaderUpgradeab1e11111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_51_raw[] = { 0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x01,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23,0x08,0x9f,0x0b,0x4b,0x9e,0x75,0xca,0xae,0x20,0x51,0xed,0xe3,0x31,0x13,0xb8,0xf1,0xbb,0x54,0xde,0xd7,0xfb,0x26,0xb9,0xa2,0xfb,0x39,0x83,0x6b,0x34,0xa2,0x9e,0xa2,0x5c,0x8e,0xa0,0xd8,0xec,0x8b,0xc0,0x2c,0x3c,0x9b,0xfc,0xe5,0xb5,0x0a,0x39,0xdc,0xd9,0x7f,0xf9,0x65,0xa2,0x2b,0x6c,0x23,0x53,0x95,0x2f,0x33,0x5a,0xcd,0xf7,0x02,0xa8,0xf6,0x91,0x4e,0x88,0xa1,0xb0,0xe2,0x10,0x15,0x3e,0xf7,0x63,0xae,0x2b,0x00,0xc2,0xb9,0x3d,0x16,0xc1,0x24,0xd2,0xc0,0x53,0x7a,0x10,0x04,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x03,0x02,0x00,0x01,0x04,0x07,0x00,0x00,0x00 };
  test.raw_tx = fd_flamenco_native_prog_test_51_raw;
  test.raw_tx_len = 304UL;
  test.expected_result = -43;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}
