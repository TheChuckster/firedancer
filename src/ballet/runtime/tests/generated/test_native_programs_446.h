#include "../fd_tests.h"
int test_446(fd_executor_test_suite_t *suite) {
  fd_executor_test_t test;
  fd_memset( &test, 0, FD_EXECUTOR_TEST_FOOTPRINT );
  test.disable_cnt = 0;
  uchar disabled_features[] = { 103,105,106,108,109,110,111,112,113,114,116,117,118,120,121,122,123,124,125,126,127,128,15,2,24,26,27,29,30,33,55,56,61,62,75,76,77,78,79,80,82,83,87,89,90,92,98 };
  test.disable_feature = disabled_features;
  test.bt = "   2: solana_stake_program::stake_instruction::tests::process_instruction             at ./src/stake_instruction.rs:578:9   3: solana_stake_program::stake_instruction::tests::test_split_to_account_with_rent_exempt_reserve             at ./src/stake_instruction.rs:5150:28   4: solana_stake_program::stake_instruction::tests::test_split_to_account_with_rent_exempt_reserve::new_behavior             at ./src/stake_instruction.rs:5078:5   5: solana_stake_program::stake_instruction::tests::test_split_to_account_with_rent_exempt_reserve::new_behavior::{{closure}}             at ./src/stake_instruction.rs:5078:5";
  test.test_name = "stake_instruction::tests::test_split_to_account_with_rent_exempt_reserve::new_behavior";
  test.test_nonce  = 343;
  test.test_number = 446;
  if (fd_executor_test_suite_check_filter(suite, &test)) return -9999;
  ulong test_accs_len = 3;
  fd_executor_test_acc_t* test_accs = fd_alloca( 1UL, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );
  fd_memset( test_accs, 0, FD_EXECUTOR_TEST_ACC_FOOTPRINT * test_accs_len );

  fd_executor_test_acc_t* test_acc = test_accs;
  fd_base58_decode_32( "EfhLLymi8nnbBA1pgfJo7E67jmreAeAHGR48GPE99B84",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  test_acc->lamports        = 2004565760UL;
  test_acc->result_lamports = 1002282880UL;
  test_acc->executable      = 0;
  test_acc->rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_446_acc_0_data[] = { 0x02,0x00,0x00,0x00,0x80,0xd5,0x22,0x00,0x00,0x00,0x00,0x00,0xcb,0x11,0x9b,0x26,0x14,0x0f,0xe8,0x54,0x5f,0x7a,0xa7,0x03,0x95,0xad,0x30,0x1e,0x67,0x88,0xa4,0x79,0x7c,0xf8,0x3f,0xb0,0x78,0x76,0xa1,0xb4,0xd1,0x73,0xab,0xe1,0xcb,0x11,0x9b,0x26,0x14,0x0f,0xe8,0x54,0x5f,0x7a,0xa7,0x03,0x95,0xad,0x30,0x1e,0x67,0x88,0xa4,0x79,0x7c,0xf8,0x3f,0xb0,0x78,0x76,0xa1,0xb4,0xd1,0x73,0xab,0xe1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x69,0x58,0x77,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xd0,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_446_acc_0_data;
  test_acc->data_len        = 200UL;
  static uchar const fd_flamenco_native_prog_test_446_acc_0_post_data[] = { 0x02,0x00,0x00,0x00,0x80,0xd5,0x22,0x00,0x00,0x00,0x00,0x00,0xcb,0x11,0x9b,0x26,0x14,0x0f,0xe8,0x54,0x5f,0x7a,0xa7,0x03,0x95,0xad,0x30,0x1e,0x67,0x88,0xa4,0x79,0x7c,0xf8,0x3f,0xb0,0x78,0x76,0xa1,0xb4,0xd1,0x73,0xab,0xe1,0xcb,0x11,0x9b,0x26,0x14,0x0f,0xe8,0x54,0x5f,0x7a,0xa7,0x03,0x95,0xad,0x30,0x1e,0x67,0x88,0xa4,0x79,0x7c,0xf8,0x3f,0xb0,0x78,0x76,0xa1,0xb4,0xd1,0x73,0xab,0xe1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xca,0x9a,0x3b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xd0,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_446_acc_0_post_data;
  test_acc->result_data_len = 200UL;
  test_acc++;
  fd_base58_decode_32( "3Di8ZfRDzLvPq9Xr82KTTdTsqE14446Xw3vyg59hRVGo",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  test_acc->lamports        = 1002282880UL;
  test_acc->result_lamports = 2004565760UL;
  test_acc->executable      = 0;
  test_acc->rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_446_acc_1_data[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->data            = fd_flamenco_native_prog_test_446_acc_1_data;
  test_acc->data_len        = 200UL;
  static uchar const fd_flamenco_native_prog_test_446_acc_1_post_data[] = { 0x02,0x00,0x00,0x00,0x80,0xd5,0x22,0x00,0x00,0x00,0x00,0x00,0xcb,0x11,0x9b,0x26,0x14,0x0f,0xe8,0x54,0x5f,0x7a,0xa7,0x03,0x95,0xad,0x30,0x1e,0x67,0x88,0xa4,0x79,0x7c,0xf8,0x3f,0xb0,0x78,0x76,0xa1,0xb4,0xd1,0x73,0xab,0xe1,0xcb,0x11,0x9b,0x26,0x14,0x0f,0xe8,0x54,0x5f,0x7a,0xa7,0x03,0x95,0xad,0x30,0x1e,0x67,0x88,0xa4,0x79,0x7c,0xf8,0x3f,0xb0,0x78,0x76,0xa1,0xb4,0xd1,0x73,0xab,0xe1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x9f,0xbd,0x3b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xd0,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  test_acc->result_data     = fd_flamenco_native_prog_test_446_acc_1_post_data;
  test_acc->result_data_len = 200UL;
  test_acc++;
  fd_base58_decode_32( "SysvarRent111111111111111111111111111111111",  (uchar *) &test_acc->pubkey);
  fd_base58_decode_32( "Sysvar1111111111111111111111111111111111111",  (uchar *) &test_acc->owner);
  test_acc->lamports        = 1UL;
  test_acc->result_lamports = 1UL;
  test_acc->executable      = 0;
  test_acc->rent_epoch      = 0;
  static uchar const fd_flamenco_native_prog_test_446_acc_2_data[] = { 0x98,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x32 };
  test_acc->data            = fd_flamenco_native_prog_test_446_acc_2_data;
  test_acc->data_len        = 17UL;
  static uchar const fd_flamenco_native_prog_test_446_acc_2_post_data[] = { 0x98,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x32 };
  test_acc->result_data     = fd_flamenco_native_prog_test_446_acc_2_post_data;
  test_acc->result_data_len = 17UL;
  test_acc++;
  fd_base58_decode_32( "Stake11111111111111111111111111111111111111",  (unsigned char *) &test.program_id);
  static uchar const fd_flamenco_native_prog_test_446_raw[] = { 0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x03,0xcb,0x11,0x9b,0x26,0x14,0x0f,0xe8,0x54,0x5f,0x7a,0xa7,0x03,0x95,0xad,0x30,0x1e,0x67,0x88,0xa4,0x79,0x7c,0xf8,0x3f,0xb0,0x78,0x76,0xa1,0xb4,0xd1,0x73,0xab,0xe1,0x20,0xf8,0xd9,0x8f,0xe7,0xaa,0x6c,0xb8,0x74,0x36,0x41,0x1c,0x12,0xb2,0x9a,0xde,0x9c,0xa2,0xf5,0xfd,0xf4,0xa9,0x3c,0xde,0x01,0xb3,0x18,0x07,0x98,0x67,0xa3,0x44,0x06,0xa1,0xd8,0x17,0x91,0x37,0x54,0x2a,0x98,0x34,0x37,0xbd,0xfe,0x2a,0x7a,0xb2,0x55,0x7f,0x53,0x5c,0x8a,0x78,0x72,0x2b,0x68,0xa4,0x9d,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x02,0x00,0x01,0x0c,0x03,0x00,0x00,0x00,0x80,0x9f,0xbd,0x3b,0x00,0x00,0x00,0x00 };
  test.raw_tx = fd_flamenco_native_prog_test_446_raw;
  test.raw_tx_len = 215UL;
  test.expected_result = 0;
  test.custom_err = 0;

  test.accs_len = test_accs_len;
  test.accs = test_accs;

  return fd_executor_run_test( &test, suite );
}
