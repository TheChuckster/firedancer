#include "../fd_tests.h"
int test_1603(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 1;
  test.bt = "   2: solana_stake_program::stake_instruction::tests::test_authorize_override             at ./src/stake_instruction.rs:1921:9   3: solana_stake_program::stake_instruction::tests::test_authorize_override::old_warmup_cooldown             at ./src/stake_instruction.rs:1814:5   4: solana_stake_program::stake_instruction::tests::test_authorize_override::old_warmup_cooldown::{{closure}}             at ./src/stake_instruction.rs:1814:5   5: core::ops::function::FnOnce::call_once             at /rustc/83964c156db1f444050a38b2498dbd0da6d5d503/library/core/src/ops/function.rs:250:5";
  test.test_name = "stake_instruction::tests::test_authorize_override::old_warmup_cooldown";
  test.test_number = 1603;
  test.sysvar_cache.clock = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==";
  test.sysvar_cache.epoch_schedule = "";
  test.sysvar_cache.epoch_rewards = "";
  test.sysvar_cache.fees = "";
  test.sysvar_cache.rent = "";
  test.sysvar_cache.slot_hashes = "";
  test.sysvar_cache.stake_history = "";
  test.sysvar_cache.slot_history = "";
  if (fd_executor_test_suite_check_filter(suite, &test)) return -9999;
  ulong test_accs_len = 3;
  fd_executor_test_acc_t* test_accs = fd_alloca( 1UL, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );
  fd_memset( test_accs, 0, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );


  uchar disabled_features[] = { 160 };
  test.disable_feature = disabled_features;
            
 // {'clock': 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==', 'epoch_schedule': '', 'epoch_rewards': '', 'fees': '', 'rent': '', 'slot_hashes': '', 'recent_blockhashes': '', 'stake_history': '', 'last_restart_slot': ''}
  fd_executor_test_acc_t* test_acc = test_accs;
  fd_base58_decode_32( "AsBGE6JNpbNdZkgQfPLtEE4A1Bv77BjhTakW2DjR7HKw",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 42UL;
  test_acc->result_lamports = 42UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_1603_acc_0_data[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x74,0xd7,0x1a,0x1c,0x77,0xa4,0x78,0x1b,0xb7,0xcc,0x02,0x13,0xb6,0x09,0xf4,0x36,0xa7,0xb3,0x59,0xb3,0x34,0x44,0x79,0x09,0x11,0x53,0x28,0x2f,0x1f,0x82,0xe7,0x92,0xf8,0xa5,0x38,0x75,0x24,0xb3,0x49,0xf4,0xd6,0x89,0x4f,0xc0,0xba,0x0c,0xa7,0x36,0x19,0x1f,0xcb,0xb5,0x94,0x71,0x03,0x22,0x72,0x9c,0xba,0x7e,0xc1,0x7d,0x9d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_1603_acc_0_data;
  test_acc->data_len        = 200UL;
  static uchar const fd_flamenco_native_prog_test_1603_acc_0_post_data[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x74,0xd7,0x1a,0x1c,0x77,0xa4,0x78,0x1b,0xb7,0xcc,0x02,0x13,0xb6,0x09,0xf4,0x36,0xa7,0xb3,0x59,0xb3,0x34,0x44,0x79,0x09,0x11,0x53,0x28,0x2f,0x1f,0x82,0xe7,0x92,0xf8,0xa5,0x38,0x75,0x24,0xb3,0x49,0xf4,0xd6,0x89,0x4f,0xc0,0xba,0x0c,0xa7,0x36,0x19,0x1f,0xcb,0xb5,0x94,0x71,0x03,0x22,0x72,0x9c,0xba,0x7e,0xc1,0x7d,0x9d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_1603_acc_0_post_data;
  test_acc->result_data_len = 200UL;
  test_acc++;
  fd_base58_decode_32( "AtiRmq2qmHQnKJ9b6BdbgJUvYD3PtHPKkCkvK5nhcRmn",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "11111111111111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "11111111111111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 0UL;
  test_acc->result_lamports = 0UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  test_acc++;
  fd_base58_decode_32( "SysvarC1ock11111111111111111111111111111111",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Sysvar1111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "Sysvar1111111111111111111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 1UL;
  test_acc->result_lamports = 1UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_1603_acc_2_data[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_1603_acc_2_data;
  test_acc->data_len        = 40UL;
  static uchar const fd_flamenco_native_prog_test_1603_acc_2_post_data[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_1603_acc_2_post_data;
  test_acc->result_data_len = 40UL;
  test_acc++;
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_1603_raw[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x04,0xc7,0x24,0x0d,0x51,0x50,0x9f,0xd1,0x52,0xf7,0x1f,0x6f,0xfe,0x23,0x70,0x31,0x5c,0xba,0x42,0xb4,0x54,0x2d,0xf5,0x4a,0x22,0xb1,0x05,0xbb,0x3c,0xbf,0x5e,0x1d,0x12,0x92,0x93,0xd3,0x9f,0x5f,0x38,0x9e,0x3a,0x2c,0xc4,0xb4,0xa7,0xac,0xa8,0x5d,0xc3,0xbd,0xe3,0x14,0x8e,0x35,0x6b,0x1f,0x52,0x25,0x06,0xe6,0xd8,0x3a,0x33,0x5d,0xba,0x06,0xa1,0xd8,0x17,0x91,0x37,0x54,0x2a,0x98,0x34,0x37,0xbd,0xfe,0x2a,0x7a,0xb2,0x55,0x7f,0x53,0x5c,0x8a,0x78,0x72,0x2b,0x68,0xa4,0x9d,0xc0,0x00,0x00,0x00,0x00,0x06,0xa7,0xd5,0x17,0x18,0xc7,0x74,0xc9,0x28,0x56,0x63,0x98,0x69,0x1d,0x5e,0xb6,0x8b,0x5e,0xb8,0xa3,0x9b,0x4b,0x6d,0x5c,0x73,0x55,0x5b,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x01,0x03,0x00,0x28,0x01,0x00,0x00,0x00,0x92,0xf8,0xa5,0x38,0x75,0x24,0xb3,0x49,0xf4,0xd6,0x89,0x4f,0xc0,0xba,0x0c,0xa7,0x36,0x19,0x1f,0xcb,0xb5,0x94,0x71,0x03,0x22,0x72,0x9c,0xba,0x7e,0xc1,0x7d,0x9d,0x01,0x00,0x00,0x00 };
  test.raw_tx = fd_flamenco_native_prog_test_1603_raw;
  test.raw_tx_len = 276UL;
  test.expected_result = -8;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}
