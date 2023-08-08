#ifndef HEADER_fd_src_flamenco_runtime_fd_hashes_h
#define HEADER_fd_src_flamenco_runtime_fd_hashes_h

#include "fd_banks_solana.h"
#include "../features/fd_features.h"

struct fd_pubkey_hash_pair {
  fd_pubkey_t pubkey;
  fd_hash_t   hash;
};
typedef struct fd_pubkey_hash_pair fd_pubkey_hash_pair_t;

typedef struct fd_global_ctx fd_global_ctx_t;

#define VECT_NAME fd_pubkey_hash_vector
#define VECT_ELEMENT fd_pubkey_hash_pair_t
#include "fd_vector.h"
#undef VECT_NAME
#undef VECT_ELEMENT

FD_PROTOTYPES_BEGIN

void fd_hash_account_deltas(fd_global_ctx_t *global, fd_pubkey_hash_pair_t * pairs, ulong pairs_len, fd_hash_t * hash );

int fd_update_hash_bank( fd_global_ctx_t * global, fd_hash_t * hash, ulong signature_cnt );

/* fd_hash_account_v0 is the legacy method to compute the account
   hash.  It includes the following content:
    - lamports
    - slot
    - rent_epoch
    - data
    - executable
    - owner
    - pubkey

   Writes the resulting hash to hash, and returns hash. */

void const *
fd_hash_account_v0( uchar                     hash[ static 32 ],
                    fd_account_meta_t const * account,
                    uchar const               pubkey[ static 32 ],
                    uchar const             * data,
                    ulong                     slot );

/* fd_hash_account_v1 was introduced in Oct-2022 via
   https://github.com/solana-labs/solana/pull/28405 and is enabled via
   the "account_hash_ignore_slot" feature gate.

   It is like fd_hash_account_with_slot, but omits the slot param. */

void const *
fd_hash_account_v1( uchar                     hash  [ static 32 ],
                    fd_account_meta_t const * account,
                    uchar const               pubkey[ static 32 ],
                    uchar const             * data );

/* fd_hash_account_current chooses the correct account hash function
   based on feature activation state. */

static inline void const *
fd_hash_account_current( uchar                     hash  [ static 32 ],
                         fd_account_meta_t const * account,
                         uchar const               pubkey[ static 32 ],
                         uchar const             * data,
                         fd_features_t const     * features,
                         ulong                     slot ) {
  if( features->account_hash_ignore_slot )
    return fd_hash_account_v1( hash, account, pubkey, data );
  else
    return fd_hash_account_v0( hash, account, pubkey, data, slot );
}

FD_PROTOTYPES_END

#endif /* HEADER_fd_src_flamenco_runtime_fd_hashes_h */
