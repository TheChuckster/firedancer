#include "../fd_tests.h"
int test_307(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 1;
  test.bt = "   2: solana_stake_program::stake_instruction::tests::test_authorize_override             at ./src/stake_instruction.rs:1872:24   3: solana_stake_program::stake_instruction::tests::test_authorize_override::old_warmup_cooldown             at ./src/stake_instruction.rs:1814:5   4: solana_stake_program::stake_instruction::tests::test_authorize_override::old_warmup_cooldown::{{closure}}             at ./src/stake_instruction.rs:1814:5   5: core::ops::function::FnOnce::call_once             at /rustc/83964c156db1f444050a38b2498dbd0da6d5d503/library/core/src/ops/function.rs:250:5";
  test.test_name = "stake_instruction::tests::test_authorize_override::old_warmup_cooldown";
  test.test_number = 307;
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
  fd_base58_decode_32( "AHSf5ChWxegJUZtCvUzcU7A2R2f6BugEiyFuMCedWhoR",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 42UL;
  test_acc->result_lamports = 42UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_307_acc_0_data[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x6b,0xf1,0x44,0x96,0xb6,0x9a,0xee,0xf1,0x8a,0xb8,0xd7,0x0d,0xa2,0xfc,0x44,0x2b,0x8e,0x3d,0x32,0x48,0xad,0x90,0x8c,0x14,0x6c,0x9f,0xd2,0x96,0xb9,0x69,0x80,0x89,0xef,0x7c,0x2e,0xe5,0x48,0xae,0x40,0x70,0xae,0x1b,0xd4,0x04,0x5b,0x64,0xd2,0x46,0xd3,0x08,0xf7,0x0c,0x55,0xed,0xc7,0xd5,0xe0,0x79,0x4c,0x3d,0x8b,0x53,0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_307_acc_0_data;
  test_acc->data_len        = 200UL;
  static uchar const fd_flamenco_native_prog_test_307_acc_0_post_data[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf3,0x88,0xa9,0x5a,0xa9,0xad,0x1f,0x4c,0xbc,0x31,0xc8,0xb5,0xf8,0xe9,0x71,0x5f,0xf9,0xc2,0x50,0x6c,0xcc,0x2e,0x7d,0x2c,0xcf,0x41,0x75,0x6d,0x54,0x6d,0xce,0x65,0x89,0xef,0x7c,0x2e,0xe5,0x48,0xae,0x40,0x70,0xae,0x1b,0xd4,0x04,0x5b,0x64,0xd2,0x46,0xd3,0x08,0xf7,0x0c,0x55,0xed,0xc7,0xd5,0xe0,0x79,0x4c,0x3d,0x8b,0x53,0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_307_acc_0_post_data;
  test_acc->result_data_len = 200UL;
  test_acc++;
  fd_base58_decode_32( "HijaHbFDF3Y5pPPoideMtvHjxkEWYmfK7fiF2W1KmRLK",  (uchar *) &test_acc->pubkey);
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
  static uchar const fd_flamenco_native_prog_test_307_acc_2_data[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_307_acc_2_data;
  test_acc->data_len        = 40UL;
  static uchar const fd_flamenco_native_prog_test_307_acc_2_post_data[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_307_acc_2_post_data;
  test_acc->result_data_len = 40UL;
  test_acc++;
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_307_raw[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x04,0xf8,0x6b,0xf1,0x44,0x96,0xb6,0x9a,0xee,0xf1,0x8a,0xb8,0xd7,0x0d,0xa2,0xfc,0x44,0x2b,0x8e,0x3d,0x32,0x48,0xad,0x90,0x8c,0x14,0x6c,0x9f,0xd2,0x96,0xb9,0x69,0x80,0x89,0xef,0x7c,0x2e,0xe5,0x48,0xae,0x40,0x70,0xae,0x1b,0xd4,0x04,0x5b,0x64,0xd2,0x46,0xd3,0x08,0xf7,0x0c,0x55,0xed,0xc7,0xd5,0xe0,0x79,0x4c,0x3d,0x8b,0x53,0xcc,0x06,0xa1,0xd8,0x17,0x91,0x37,0x54,0x2a,0x98,0x34,0x37,0xbd,0xfe,0x2a,0x7a,0xb2,0x55,0x7f,0x53,0x5c,0x8a,0x78,0x72,0x2b,0x68,0xa4,0x9d,0xc0,0x00,0x00,0x00,0x00,0x06,0xa7,0xd5,0x17,0x18,0xc7,0x74,0xc9,0x28,0x56,0x63,0x98,0x69,0x1d,0x5e,0xb6,0x8b,0x5e,0xb8,0xa3,0x9b,0x4b,0x6d,0x5c,0x73,0x55,0x5b,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x01,0x03,0x00,0x28,0x01,0x00,0x00,0x00,0xf3,0x88,0xa9,0x5a,0xa9,0xad,0x1f,0x4c,0xbc,0x31,0xc8,0xb5,0xf8,0xe9,0x71,0x5f,0xf9,0xc2,0x50,0x6c,0xcc,0x2e,0x7d,0x2c,0xcf,0x41,0x75,0x6d,0x54,0x6d,0xce,0x65,0x00,0x00,0x00,0x00 };
  test.raw_tx = fd_flamenco_native_prog_test_307_raw;
  test.raw_tx_len = 276UL;
  test.expected_result = 0;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}
