#include "../fd_tests.h"
int test_2055(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 1;
  test.bt = "   2: solana_stake_program::stake_instruction::tests::test_split_from_larger_sized_account             at ./src/stake_instruction.rs:5244:13   3: solana_stake_program::stake_instruction::tests::test_split_from_larger_sized_account::old_warmup_cooldown             at ./src/stake_instruction.rs:5182:5   4: solana_stake_program::stake_instruction::tests::test_split_from_larger_sized_account::old_warmup_cooldown::{{closure}}             at ./src/stake_instruction.rs:5182:5   5: core::ops::function::FnOnce::call_once             at /rustc/83964c156db1f444050a38b2498dbd0da6d5d503/library/core/src/ops/function.rs:250:5";
  test.test_name = "stake_instruction::tests::test_split_from_larger_sized_account::old_warmup_cooldown";
  test.test_number = 2055;
  test.sysvar_cache.clock = "";
  test.sysvar_cache.epoch_schedule = "";
  test.sysvar_cache.epoch_rewards = "";
  test.sysvar_cache.fees = "";
  test.sysvar_cache.rent = "mA0AAAAAAAAAAAAAAAAAQDI=";
  test.sysvar_cache.slot_hashes = "";
  test.sysvar_cache.stake_history = "";
  test.sysvar_cache.slot_history = "";
  if (fd_executor_test_suite_check_filter(suite, &test)) return -9999;
  ulong test_accs_len = 3;
  fd_executor_test_acc_t* test_accs = fd_alloca( 1UL, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );
  fd_memset( test_accs, 0, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );


  uchar disabled_features[] = { 160 };
  test.disable_feature = disabled_features;
            
 // {'clock': '', 'epoch_schedule': '', 'epoch_rewards': '', 'fees': '', 'rent': 'mA0AAAAAAAAAAAAAAAAAQDI=', 'slot_hashes': '', 'recent_blockhashes': '', 'stake_history': '', 'last_restart_slot': ''}
  fd_executor_test_acc_t* test_acc = test_accs;
  fd_base58_decode_32( "ttJQtjCz6uVDCNHuySmxVJwxD3SeurUXXjK5m9z25Yc",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 2005957760UL;
  test_acc->result_lamports = 2005957760UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_2055_acc_0_data[] = { 0x02,0x00,0x00,0x00,0x40,0x74,0x2d,0x00,0x00,0x00,0x00,0x00,0x0d,0x4a,0xa0,0x68,0x38,0xbf,0x4c,0xbe,0xe7,0xa5,0xaf,0xa7,0x85,0xe2,0x81,0xc6,0x0f,0xb6,0xfa,0x74,0x12,0xaa,0x89,0x3b,0xcd,0x47,0x99,0xb9,0xf6,0x3c,0xb6,0x71,0x0d,0x4a,0xa0,0x68,0x38,0xbf,0x4c,0xbe,0xe7,0xa5,0xaf,0xa7,0x85,0xe2,0x81,0xc6,0x0f,0xb6,0xfa,0x74,0x12,0xaa,0x89,0x3b,0xcd,0x47,0x99,0xb9,0xf6,0x3c,0xb6,0x71,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x08,0x63,0x77,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xd0,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_2055_acc_0_data;
  test_acc->data_len        = 300UL;
  static uchar const fd_flamenco_native_prog_test_2055_acc_0_post_data[] = { 0x02,0x00,0x00,0x00,0x40,0x74,0x2d,0x00,0x00,0x00,0x00,0x00,0x0d,0x4a,0xa0,0x68,0x38,0xbf,0x4c,0xbe,0xe7,0xa5,0xaf,0xa7,0x85,0xe2,0x81,0xc6,0x0f,0xb6,0xfa,0x74,0x12,0xaa,0x89,0x3b,0xcd,0x47,0x99,0xb9,0xf6,0x3c,0xb6,0x71,0x0d,0x4a,0xa0,0x68,0x38,0xbf,0x4c,0xbe,0xe7,0xa5,0xaf,0xa7,0x85,0xe2,0x81,0xc6,0x0f,0xb6,0xfa,0x74,0x12,0xaa,0x89,0x3b,0xcd,0x47,0x99,0xb9,0xf6,0x3c,0xb6,0x71,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x08,0x63,0x77,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xd0,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_2055_acc_0_post_data;
  test_acc->result_data_len = 300UL;
  test_acc++;
  fd_base58_decode_32( "3B8UqBXCXgvuuyPLAZDNKPv1JPiAEWpHbgtAfxrp2hp3",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 2282880UL;
  test_acc->result_lamports = 2282880UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_2055_acc_1_data[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_2055_acc_1_data;
  test_acc->data_len        = 200UL;
  static uchar const fd_flamenco_native_prog_test_2055_acc_1_post_data[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_2055_acc_1_post_data;
  test_acc->result_data_len = 200UL;
  test_acc++;
  fd_base58_decode_32( "SysvarRent111111111111111111111111111111111",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Sysvar1111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  fd_base58_decode_32( "Sysvar1111111111111111111111111111111111111",  (uchar *) &test_acc->result_owner);
  test_acc->lamports        = 1UL;
  test_acc->result_lamports = 1UL;
  test_acc->executable      = 0;
  test_acc->result_executable= 0;
  test_acc->rent_epoch      = 0;
  test_acc->result_rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_2055_acc_2_data[] = { 0x98,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x32 };
  test_acc->data            = fd_flamenco_native_prog_test_2055_acc_2_data;
  test_acc->data_len        = 17UL;
  static uchar const fd_flamenco_native_prog_test_2055_acc_2_post_data[] = { 0x98,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x32 };
  test_acc->result_data     = fd_flamenco_native_prog_test_2055_acc_2_post_data;
  test_acc->result_data_len = 17UL;
  test_acc++;
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_2055_raw[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x03,0x0d,0x4a,0xa0,0x68,0x38,0xbf,0x4c,0xbe,0xe7,0xa5,0xaf,0xa7,0x85,0xe2,0x81,0xc6,0x0f,0xb6,0xfa,0x74,0x12,0xaa,0x89,0x3b,0xcd,0x47,0x99,0xb9,0xf6,0x3c,0xb6,0x71,0x20,0x4f,0xa3,0xb1,0x61,0xbf,0xce,0x96,0x55,0x46,0x8d,0x4b,0x68,0x15,0x21,0x41,0xdb,0xc9,0xa4,0xb7,0x7e,0x5e,0xfd,0x1f,0x8c,0x58,0x58,0xec,0xa2,0x85,0x05,0xc0,0x06,0xa1,0xd8,0x17,0x91,0x37,0x54,0x2a,0x98,0x34,0x37,0xbd,0xfe,0x2a,0x7a,0xb2,0x55,0x7f,0x53,0x5c,0x8a,0x78,0x72,0x2b,0x68,0xa4,0x9d,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x02,0x00,0x01,0x0c,0x03,0x00,0x00,0x00,0x81,0x7c,0x90,0x77,0x00,0x00,0x00,0x00 };
  test.raw_tx = fd_flamenco_native_prog_test_2055_raw;
  test.raw_tx_len = 215UL;
  test.expected_result = -6;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}
