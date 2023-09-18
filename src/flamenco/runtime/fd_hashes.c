#include "fd_hashes.h"
#include "../../ballet/blake3/fd_blake3.h"
#include "../../ballet/sha256/fd_sha256.h"
#include <assert.h>
#include <stdio.h>
#include "../../ballet/base58/fd_base58.h"
#include "fd_acc_mgr.h"
#include "fd_runtime.h"
#include "fd_account.h"

#define SORT_NAME sort_pubkey_hash_pair
#define SORT_KEY_T fd_pubkey_hash_pair_t
#define SORT_BEFORE(a,b) ((memcmp(&a, &b, 32) < 0))
#include "../../util/tmpl/fd_sort.c"

#define FD_ACCOUNT_DELTAS_MERKLE_FANOUT (16UL)
#define FD_ACCOUNT_DELTAS_MAX_MERKLE_HEIGHT (16UL)

void
fd_hash_account_deltas(fd_global_ctx_t *global, fd_pubkey_hash_pair_t * pairs, ulong pairs_len, fd_hash_t * hash ) {
  fd_sha256_t shas[FD_ACCOUNT_DELTAS_MAX_MERKLE_HEIGHT];
  uchar num_hashes[FD_ACCOUNT_DELTAS_MAX_MERKLE_HEIGHT+1];

  // Init the number of hashes
  fd_memset( num_hashes, 0, sizeof(num_hashes) );

  sort_pubkey_hash_pair_inplace( pairs, pairs_len );
  for( ulong j = 0; j < FD_ACCOUNT_DELTAS_MAX_MERKLE_HEIGHT; ++j ) {
    fd_sha256_init( &shas[j] );
  }

  if( pairs_len == 0 ) {
    fd_sha256_fini( &shas[0], hash->hash );
    if (FD_UNLIKELY(global->log_level > 5)) {
      FD_LOG_NOTICE(( "M"
        "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
        FD_LOG_HEX16_FMT_ARGS(     hash->hash    ), FD_LOG_HEX16_FMT_ARGS(     hash->hash+16 )));
    }
    return;
  }

  if (FD_UNLIKELY(global->log_level > 2))
    FD_LOG_NOTICE(( "W %ld", pairs_len));

  for( ulong i = 0; i < pairs_len; ++i ) {

    // if (FD_UNLIKELY(global->log_level > 2)) {
    //   FD_LOG_NOTICE(( "account delta hash X { \"key\":%ld, \"pubkey\":\"%32J\", \"hash\":\"%32J\" },", i, pairs[i].pubkey.key, pairs[i].hash.hash));

    //   /*
    //   pubkey
    //   slot
    //   lamports
    //   owner
    //   executable
    //   rent_epoch
    //   data_len
    //   data
    //   hash
    //   */
    //   fd_pubkey_t current_owner;
    //   fd_acc_mgr_get_owner( global->acc_mgr, global->funk_txn, &pairs[i].pubkey, &current_owner );
    //   char encoded_owner[50];
    //   fd_base58_encode_32((uchar *) &current_owner, 0, encoded_owner);

    //   uchar * raw_acc_data = (uchar*) fd_acc_mgr_view_raw(global->acc_mgr, global->funk_txn, (fd_pubkey_t *) &pairs[i].pubkey, NULL, NULL);
    //   if (NULL == raw_acc_data)
    //     return;
    //   fd_account_meta_t * metadata = (fd_account_meta_t *)raw_acc_data;
    //   uchar * acc_data = fd_account_get_data(metadata);
    //   char * acc_data_str = malloc(5*metadata->dlen + 1);

    //   char * acc_data_str_cursor = acc_data_str;
    //   if (metadata->dlen > 0) {
    //     for( ulong j = 0; j < (metadata->dlen - 1); j++ ) {
    //       int x = sprintf(acc_data_str_cursor, "%u, ", acc_data[j]);
    //       acc_data_str_cursor += x;
    //     }
    //     sprintf(acc_data_str_cursor, "%u", acc_data[metadata->dlen - 1]);
    //   } else {
    //     *acc_data_str_cursor = 0;
    //   }

    //   FD_LOG_NOTICE(( "account_delta_hash_compare pubkey: (%s) slot: (%lu) lamports: (%lu), owner: (%s), executable: (%d), rent_epoch: (%lu), data_len: (%ld), hash: (%s) ",  encoded_pubkey, global->bank.slot, metadata->info.lamports, encoded_owner, metadata->info.executable, metadata->info.rent_epoch, metadata->dlen, encoded_hash ));
    //   printf( "account_delta_hash pubkey: %s, slot: %lu, lamports: %lu, owner: %s, executable: %d, rent_epoch: %lu, data_len: %ld, data: [%s] = %s\n",  encoded_pubkey, global->bank.slot, metadata->info.lamports, encoded_owner, metadata->info.executable, metadata->info.rent_epoch, metadata->dlen, acc_data_str, encoded_hash );

    //   free(acc_data_str);
    // }

    fd_sha256_append( &shas[0] , (uchar const *) pairs[i].hash.hash, sizeof( fd_hash_t ) );
    num_hashes[0]++;

    for( ulong j = 0; j < FD_ACCOUNT_DELTAS_MAX_MERKLE_HEIGHT; ++j ) {
      if (FD_UNLIKELY(global->log_level > 5)) {
        FD_LOG_NOTICE(( "Z %lu %lu %lu", i, j, shas[j].buf_used ));
      }
      if (num_hashes[j] == FD_ACCOUNT_DELTAS_MERKLE_FANOUT) {
        if (FD_UNLIKELY(global->log_level > 5)) {
          FD_LOG_NOTICE(( "Y %lu %lu %u %u", i, j, num_hashes[j], num_hashes[j+1] ));
        }
        num_hashes[j] = 0;
        num_hashes[j+1]++;
        fd_hash_t sub_hash;
        fd_sha256_fini( &shas[j], &sub_hash );
        fd_sha256_init( &shas[j] );
        if (FD_UNLIKELY(global->log_level > 5)) {
          char encoded_hash[50];
          fd_base58_encode_32((uchar *) sub_hash.hash, 0, encoded_hash);
          FD_LOG_NOTICE(( "V %lu %lu %s", i, j, encoded_hash ));
        }
        fd_sha256_append( &shas[j+1], (uchar const *) sub_hash.hash, sizeof( fd_hash_t ) );
      } else {
        break;
      }
    }
  }

  // TODO: use CZT on pairs_len
  ulong height = 0;
  for( long i = FD_ACCOUNT_DELTAS_MAX_MERKLE_HEIGHT-1; i >= 0; --i ) {
    if( num_hashes[i] != 0 ) {
      height = (ulong) i + 1;
      break;
    }
  }

  for( ulong i = 0; i < height; ++i ) {
#ifdef _VERBOSE
    FD_LOG_NOTICE(( "S %lu %u", i, num_hashes[i] ));
#endif
    if( num_hashes[i]==0 ) {
      continue;
    }
    // At level i, finalize and append to i + 1
    //fd_hash_t sub_hash;
    fd_sha256_fini( &shas[i], hash );
#ifdef _VERBOSE
    FD_LOG_NOTICE(( "Q (%lu)"
      "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
      i,
      FD_LOG_HEX16_FMT_ARGS(     hash->hash    ), FD_LOG_HEX16_FMT_ARGS(     hash->hash+16 )));
#endif
    num_hashes[i] = 0;
    num_hashes[i+1]++;

    if (i == (height-1)) {
      ulong tot_num_hashes = 0;
      for (ulong k = 0; k < FD_ACCOUNT_DELTAS_MAX_MERKLE_HEIGHT; ++k ) {
        tot_num_hashes += num_hashes[k];
      }

      assert(tot_num_hashes == 1);
#ifdef _VERBOSE
      FD_LOG_NOTICE(( "M"
        "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
        FD_LOG_HEX16_FMT_ARGS(     hash->hash    ), FD_LOG_HEX16_FMT_ARGS(     hash->hash+16 )));
#endif
      return;
    }
    fd_sha256_append( &shas[i+1], (uchar const *) hash->hash, sizeof( fd_hash_t ) );

    // There is now one more hash at level i+1

    // check, have we filled this level and ones above it.
    for( ulong j = i+1; j < height; ++j ) {
      // if the level is full, finalize and push into next level.
      if (num_hashes[j] == FD_ACCOUNT_DELTAS_MERKLE_FANOUT) {
        num_hashes[j] = 0;
        num_hashes[j+1]++;
        fd_hash_t sub_hash;
        fd_sha256_fini( &shas[j], &sub_hash );
        if (FD_UNLIKELY(global->log_level > 5)) {
          char encoded_hash[50];
          fd_base58_encode_32((uchar *) sub_hash.hash, 0, encoded_hash);
          FD_LOG_NOTICE(( "L %lu %lu %s", i, j, encoded_hash ));
        }
        fd_sha256_append( &shas[j+1], (uchar const *) sub_hash.hash, sizeof( fd_hash_t ) );
      }
    }
  }

  // If the level at the `height' was rolled into, do something about it

}

static void
fd_hash_bank( fd_global_ctx_t *global, fd_hash_t * hash, fd_pubkey_hash_vector_t * dirty_keys) {
  global->prev_banks_hash = global->bank.banks_hash;

  fd_hash_account_deltas( global, dirty_keys->elems, dirty_keys->cnt, &global->account_delta_hash );

  fd_sha256_t sha;
  fd_sha256_init( &sha );
  fd_sha256_append( &sha, (uchar const *) &global->bank.banks_hash, sizeof( fd_hash_t ) );
  fd_sha256_append( &sha, (uchar const *) &global->account_delta_hash, sizeof( fd_hash_t  ) );
  fd_sha256_append( &sha, (uchar const *) &global->signature_cnt, sizeof( ulong ) );
  fd_sha256_append( &sha, (uchar const *) &global->bank.poh, sizeof( fd_hash_t ) );

  fd_sha256_fini( &sha, hash->hash );

  fd_solcap_write_bank_preimage(
      global->capture,
      hash->hash,
      global->prev_banks_hash.hash,
      global->account_delta_hash.hash,
      &global->bank.poh.hash,
      global->signature_cnt );

  if (global->log_level > 0) {
    FD_LOG_NOTICE(( "bank_hash slot: %lu,  hash: %32J,  parent_hash: %32J,  accounts_delta: %32J,  signature_count: %ld,  last_blockhash: %32J",
        global->bank.slot, hash->hash, global->prev_banks_hash.hash, global->account_delta_hash.hash, global->signature_cnt, global->bank.poh.hash ));
  }
}


int
fd_update_hash_bank( fd_global_ctx_t * global,
                     fd_hash_t *       hash,
                     ulong             signature_cnt ) {

  fd_funk_t *          funk     = global->funk;
  fd_acc_mgr_t *       acc_mgr  = global->acc_mgr;
  fd_funk_txn_t *      txn      = global->funk_txn;
  ulong                slot     = global->bank.slot;
  fd_solcap_writer_t * capture  = global->capture;

  /* Collect list of changed accounts to be added to bank hash */

  fd_pubkey_hash_vector_t dirty_keys __attribute__ ((cleanup(fd_pubkey_hash_vector_destroy)));
  fd_pubkey_hash_vector_new(&dirty_keys);

  /* Iterate over accounts that have been changed in the current
     database transaction. */

  for( fd_funk_rec_t const * rec = fd_funk_txn_first_rec( funk, txn );
       NULL != rec;
       rec = fd_funk_txn_next_rec( funk, rec ) ) {

    if( !fd_acc_mgr_is_key( rec->pair.key  ) ) continue;
    if( !fd_funk_rec_is_modified(funk, rec ) ) continue;

    /* Get dirty account */

    fd_pubkey_t const *       acc_key  = fd_type_pun_const( rec->pair.key[0].uc );
    fd_funk_rec_t const *     rec      = NULL;

    int err = 0;
    uchar const * _raw = fd_acc_mgr_view_raw( acc_mgr, txn, acc_key, &rec, &err);
    if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) )
      return err;
    fd_account_meta_t const * acc_meta = (fd_account_meta_t const *)_raw;
    uchar const *             acc_data = _raw + acc_meta->hlen;

    /* Hash account */

    fd_hash_t acc_hash[1];
    if (FD_UNLIKELY(!FD_RAW_ACCOUNT_EXISTS(_raw))) {
      fd_memset( acc_hash->hash, 0, FD_HASH_FOOTPRINT );
    } else {
      fd_hash_account_current( acc_hash->hash, acc_meta, acc_key->key, acc_data, global );
    }

    /* If hash didn't change, nothing to do */

    if( 0==memcmp( acc_hash->hash, acc_meta->hash, sizeof(fd_hash_t) ) )
      continue;

    /* Upgrade to writable record */

    FD_BORROWED_ACCOUNT_DECL(acc_rec);
    acc_rec->const_rec = rec;

    err = fd_acc_mgr_modify( acc_mgr, txn, acc_key, 0, 0UL, acc_rec);
    if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) ) return err;

    /* Update hash */

    memcpy( acc_rec->meta->hash, acc_hash->hash, sizeof(fd_hash_t) );

    /* Logging ... */

    if( FD_UNLIKELY( acc_mgr->global->log_level > 2 ) ) {
      FD_LOG_WARNING(( "fd_acc_mgr_update_hash: %32J "
                        "slot: %ld "
                        "lamports: %ld  "
                        "owner: %32J  "
                        "executable: %s,  "
                        "rent_epoch: %ld, "
                        "data_len: %ld",
                        acc_key,
                        slot,
                        acc_rec->meta->info.lamports,
                        acc_rec->meta->info.owner,
                        acc_rec->meta->info.executable ? "true" : "false",
                        acc_rec->meta->info.rent_epoch,
                        acc_rec->meta->dlen ));
    }

    /* Add account to "dirty keys" list, which will be added to the
       bank hash. */

    fd_pubkey_hash_pair_t dirty_entry;
    memcpy( dirty_entry.pubkey.key, acc_key,        sizeof(fd_pubkey_t) );
    memcpy( dirty_entry.hash.hash,  acc_hash->hash, sizeof(fd_hash_t  ) );
    fd_pubkey_hash_vector_push( &dirty_keys, dirty_entry );

    /* Add to capture */

    err = fd_solcap_write_account(
        capture,
        acc_key->uc,
        &acc_rec->meta->info,
        acc_data,
        acc_rec->meta->dlen,
        acc_hash->hash );
    FD_TEST( err==0 );
  }

  /* Sort and hash "dirty keys" to the accounts delta hash. */

  if( FD_UNLIKELY(global->log_level > 2) )
    FD_LOG_WARNING(("slot %ld   dirty %ld", global->bank.slot, dirty_keys.cnt));

  global->signature_cnt = signature_cnt;
  fd_hash_bank( global, hash, &dirty_keys );

  return FD_EXECUTOR_INSTR_SUCCESS;
}

void const *
fd_hash_account_v0( uchar                     hash[ static 32 ],
                    fd_account_meta_t const * m,
                    uchar const               pubkey[ static 32 ],
                    uchar const             * data,
                    ulong                     slot ) {

  ulong         lamports   = m->info.lamports;  /* >0UL */
  ulong         rent_epoch = m->info.rent_epoch;
  uchar         executable = m->info.executable & 0x1;
  uchar const * owner      = (uchar const *)m->info.owner;

  fd_blake3_t b3[1];
  fd_blake3_init  ( b3 );
  fd_blake3_append( b3, &lamports,   sizeof( ulong ) );
  fd_blake3_append( b3, &slot,       sizeof( ulong ) );
  fd_blake3_append( b3, &rent_epoch, sizeof( ulong ) );
  fd_blake3_append( b3, data,        m->dlen         );
  fd_blake3_append( b3, &executable, sizeof( uchar ) );
  fd_blake3_append( b3, owner,       32UL            );
  fd_blake3_append( b3, pubkey,      32UL            );
  fd_blake3_fini  ( b3, hash );
  return hash;
}

void const *
fd_hash_account_v1( uchar                     hash[ static 32 ],
                    fd_account_meta_t const * m,
                    uchar const               pubkey[ static 32 ],
                    uchar const             * data ) {

  ulong         lamports   = m->info.lamports;  /* >0UL */
  ulong         rent_epoch = m->info.rent_epoch;
  uchar         executable = m->info.executable & 0x1;
  uchar const * owner      = (uchar const *)m->info.owner;

  fd_blake3_t b3[1];
  fd_blake3_init  ( b3 );
  fd_blake3_append( b3, &lamports,   sizeof( ulong ) );
  fd_blake3_append( b3, &rent_epoch, sizeof( ulong ) );
  fd_blake3_append( b3, data,        m->dlen         );
  fd_blake3_append( b3, &executable, sizeof( uchar ) );
  fd_blake3_append( b3, owner,       32UL            );
  fd_blake3_append( b3, pubkey,      32UL            );
  fd_blake3_fini  ( b3, hash );
  return hash;
}

void const *
fd_hash_account_current( uchar                     hash  [ static 32 ],
                         fd_account_meta_t const * account,
                         uchar const               pubkey[ static 32 ],
                         uchar const             * data,
                         fd_global_ctx_t const   * global ) {
  if( FD_FEATURE_ACTIVE( global, account_hash_ignore_slot ) )
    return fd_hash_account_v1( hash, account, pubkey, data );
  else
    return fd_hash_account_v0( hash, account, pubkey, data, global->bank.slot );
}
