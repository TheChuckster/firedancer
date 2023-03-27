// sudo /home/jsiegel/repos/firedancer-private/build/linux/gcc/x86_64/bin/fd_shmem_cfg init 0777 jsiegel ""
// sudo /home/jsiegel/repos/firedancer-private/build/linux/gcc/x86_64/bin/fd_shmem_cfg alloc 64 gigantic 0
// sudo /home/jsiegel/repos/firedancer-private/build/linux/gcc/x86_64/bin/fd_shmem_cfg alloc 512 huge 0

//  --ledger /dev/shm/mainnet-ledger --db /dev/shm/funk --cmd replay --start-slot 179138256 --end-slot 179138258  --txn-exe sim  --index-max 120000000 --pages 15

//  --ledger /dev/shm/mainnet-ledger --db /dev/shm/funk --cmd replay --start-slot 179138205 --end-slot 279138205  --txn-exe sim  --index-max 120000000 --pages 15
//  --ledger /dev/shm/mainnet-ledger --db /dev/shm/funk --cmd ingest --start-slot 179138205 --end-slot 279138205 --manifest /dev/shm/mainnet-ledger/snapshot/tmp-snapshot-archive-JfVTLu/snapshots/179248368/179248368
// run --ledger /home/jsiegel/mainnet-ledger --db /home/jsiegel/funk --cmd ingest --accounts /home/jsiegel/mainnet-ledger/accounts --pages 15 --index-max 120000000
// run --ledger /dev/shm/mainnet-ledger --db /dev/shm/funk --cmd ingest --accounts /dev/shm/mainnet-ledger/accounts --pages 15 --index-max 120000000

// --ledger /home/jsiegel/repos/solana/test-ledger --db /home/jsiegel/repos/solana//test-ledger/funk --cmd replay --start-slot 0 --end-slot 200  --txn-exe sim  --index-max 120000000 --pages 15

// run --ledger /home/jsiegel/test-ledger --db /home/jsiegel/test-ledger/funk --cmd replay --start-slot 0 --end-slot 200 --index-max 120000000 --pages 15

// --ledger /home/jsiegel/test-ledger --db /home/jsiegel/funk --cmd validate --accounts /home/jsiegel/test-ledger/accounts/ --pages 15 --index-max 120000000 --manifest /home/jsiegel/test-ledger/200/snapshots/200/200 --start-slot 200

// --ledger /home/jsiegel/test-ledger --db /home/jsiegel/funk --cmd validate --accounts /home/jsiegel/test-ledger/accounts/ --pages 15 --index-max 120000000 --manifest /home/jsiegel/test-ledger/100/snapshots/100/100 --start-slot 100

// --ledger /home/jsiegel/test-ledger --db /home/jsiegel/funk --cmd accounts --accounts /home/jsiegel/test-ledger/accounts/ --pages 15 --index-max 120000000 --start-slot 5 --end-slot 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <fcntl.h>
#include "fd_rocksdb.h"
#include "fd_banks_solana.h"
#include "fd_hashes.h"
#include "fd_executor.h"
#include "../../funk/fd_funk.h"
#include "../../util/alloc/fd_alloc.h"
#include "../base58/fd_base58.h"
#include "../poh/fd_poh.h"
#include "../bmtree/fd_bmtree.h"
#include "../sha256/fd_sha256.h"

#include <dirent.h>

uchar do_valgrind = 1;

int fd_alloc_fprintf( fd_alloc_t * join, FILE *       stream );

char* allocf(void *arg, ulong align, ulong len) {
  if (NULL == arg) {
    FD_LOG_ERR(( "yo dawg.. you passed a NULL as a fd_alloc pool"));
  }

  if (do_valgrind) {
    char * ptr = malloc(fd_ulong_align_up(sizeof(char *) + len, align));
    char * ret = (char *) fd_ulong_align_up( (ulong) (ptr + sizeof(char *)), align );
    *((char **)(ret - sizeof(char *))) = ptr;
    return ret;
  } else 
    return fd_alloc_malloc(arg, align, len);
}

void freef(void *arg, void *ptr) {
  if (NULL == arg) {
    FD_LOG_ERR(( "yo dawg.. you passed a NULL as a fd_alloc pool"));
  }

  if (do_valgrind) 
    free(*((char **)((char *) ptr - sizeof(char *))));
  else
    fd_alloc_free(arg, ptr);
}

struct global_state {
  ulong        end_slot;
  ulong        start_slot;
  ulong        pages;
  uchar        txn_exe;

  int          argc;
  char       **argv;

  char const * name;
  char const * ledger;
  char const * db;
  char const * end_slot_opt;
  char const * start_slot_opt;
  char const * manifest;
  char const * accounts;
  char const * cmd;
  char const * txn_exe_opt;
  char const * pages_opt;

  fd_wksp_t *         wksp;
  fd_alloc_t *        alloc;
  fd_funk_t*          funk;
  fd_acc_mgr_t*       acc_mgr;
  fd_rocksdb_t        rocks_db;
  fd_genesis_solana_t gen;
  uchar               genesis_hash[FD_SHA256_HASH_SZ];

  fd_poh_state_t      poh;
};
typedef struct global_state global_state_t;

static void usage(const char* progname) {
  fprintf(stderr, "USAGE: %s\n", progname);
  fprintf(stderr, " --wksp        <name>       workspace name\n");
  fprintf(stderr, " --ledger      <dir>        ledger directory\n");
  fprintf(stderr, " --db          <file>       firedancer db file\n");
  fprintf(stderr, " --end-slot    <num>        stop iterating at block...\n");
  fprintf(stderr, " --start-slot  <num>        start iterating at block...\n");
  fprintf(stderr, " --manifest    <file>       What manifest file should I pay attention to\n");
  fprintf(stderr, " --accounts    <dir>        What accounts should I slurp in\n");
  fprintf(stderr, " --cmd         <operation>  What operation should we test\n");
  fprintf(stderr, " --skip-exe    [skip,sim]   Should we skip executing transactions\n");
  fprintf(stderr, " --index-max   <bool>       How big should the index table be?\n");
  fprintf(stderr, " --validate    <bool>       Validate the funk db\n");
}

// pub const FULL_SNAPSHOT_ARCHIVE_FILENAME_REGEX: &str = r"^snapshot-(?P<slot>[[:digit:]]+)-(?P<hash>[[:alnum:]]+)\.(?P<ext>tar|tar\.bz2|tar\.zst|tar\.gz|tar\.lz4)$";
// pub const INCREMENTAL_SNAPSHOT_ARCHIVE_FILENAME_REGEX: &str = r"^incremental-snapshot-(?P<base>[[:digit:]]+)-(?P<slot>[[:digit:]]+)-(?P<hash>[[:alnum:]]+)\.(?P<ext>tar|tar\.bz2|tar\.zst|tar\.gz|tar\.lz4)$";



int ingest(global_state_t *state) {
  if (NULL == state->accounts)  {
    usage(state->argv[0]);
    exit(1);
  }

  regex_t reg;
  // Where were those regular expressions for snapshots?
  if (regcomp(&reg, "[0-9]+\\.[0-9]+", REG_EXTENDED) != 0) {
    FD_LOG_ERR(( "compile failed" ));
  }

  FD_LOG_WARNING(("starting read of %s", state->accounts));

  DIR *dir = opendir(state->accounts);

  struct dirent * ent;

  ulong files = 0;
  ulong accounts = 0;
  ulong odd = 0;

  while ( NULL != (ent = readdir(dir)) ) {
    if ( regexec(&reg, ent->d_name, 0, NULL, 0) == 0 )  {
        struct stat s;
        char buf[1000];

        strcpy(buf, ent->d_name);
        char *p = buf;
        while (*p != '.') p++; // It sure as heck better have a . or the regexec would fail
        *p = '\0';

        ulong slot = (ulong) atol(buf);
        
        sprintf(buf, "%s/%s", state->accounts, ent->d_name);
        stat(buf,  &s);
        unsigned char *r = (unsigned char *)allocf(state->alloc, 8UL, (unsigned long) (unsigned long) s.st_size);
        unsigned char *b = r;
        files++;
        int fd = open(buf, O_RDONLY);
        ssize_t n = read(fd, b, (unsigned long) s.st_size);
        if (n < 0) {
          FD_LOG_ERR(( "??" ));
        }
        unsigned char *eptr = &b[(ulong) ((ulong)n - (ulong)sizeof(fd_solana_account_hdr_t))];
        if (n != s.st_size) {
          FD_LOG_ERR(( "??" ));
        }

        while (b < eptr) {
          fd_solana_account_hdr_t *hdr = (fd_solana_account_hdr_t *)b;
          // Look for corruption...
          if ((b + hdr->meta.data_len) > (r + n)) {
            // some kind of corruption in the file?
            break;
          }
          // Sanitize accounts...
          if ((hdr->info.lamports == 0) | ((hdr->info.executable & ~1) != 0))
            break;

          accounts++;

          if ((accounts % 1000000) == 0) {
            FD_LOG_WARNING(("accounts %ld", accounts));
          }


          do {
            fd_account_meta_t metadata;
            int read_result = fd_acc_mgr_get_metadata( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, &metadata );
            if ( FD_UNLIKELY( read_result == FD_ACC_MGR_SUCCESS ) ) {
              if (metadata.slot > slot)
                break;
            }

            if (fd_acc_mgr_write_append_vec_account( state->acc_mgr,  slot, hdr) != FD_ACC_MGR_SUCCESS) {
              FD_LOG_ERR(("writing failed: accounts %ld", accounts));
            }
            read_result = fd_acc_mgr_get_metadata( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, &metadata );
            if ( FD_UNLIKELY( read_result != FD_ACC_MGR_SUCCESS ) ) 
              FD_LOG_ERR(("wtf"));
            if ((metadata.magic != FD_ACCOUNT_META_MAGIC) || (metadata.hlen != sizeof(metadata))) 
              FD_LOG_ERR(("wtf2"));
            
            uchar * account_data = (uchar *) allocf(state->alloc , 8UL,  hdr->meta.data_len);
            fd_account_meta_t account_hdr;
            read_result = fd_acc_mgr_get_account_data( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, (uchar *) &account_hdr, 0, sizeof(fd_account_meta_t));
            
            read_result = fd_acc_mgr_get_account_data( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, account_data, sizeof(fd_account_meta_t), account_hdr.dlen);
            if ( FD_UNLIKELY( read_result != FD_ACC_MGR_SUCCESS ) ) 
              FD_LOG_ERR(("wtf3"));

            fd_solana_account_t account = {
              .lamports = hdr->info.lamports,
              .rent_epoch = hdr->info.rent_epoch,
              .data_len = hdr->meta.data_len,
              .data = account_data,
              .executable = (uchar) hdr->info.executable
            };
            fd_memcpy( account.owner.key, hdr->info.owner, 32 );
            
            uchar hash[32];
            fd_hash_account( &account, slot, (fd_pubkey_t const *)  &hdr->meta.pubkey, (fd_hash_t *) hash );
            //fd_hash_account( &account, 1, (fd_pubkey_t const *)  &pubkey, (fd_hash_t *) hash );
            
            // If account hashes are mismatched, fail
            if ( memcmp( hdr->hash.value, hash, 32 ) ) {
              FD_LOG_ERR(( "FAIL (account hashes mismatched)"
                 "\n\tGot"
                 "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT
                 "\n\tExpected"
                 "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
                 FD_LOG_HEX16_FMT_ARGS(     hdr->hash.value    ), FD_LOG_HEX16_FMT_ARGS(     hdr->hash.value+16 ),
                 FD_LOG_HEX16_FMT_ARGS(     hash    ), FD_LOG_HEX16_FMT_ARGS(     hash+16 ) 
              ));
            }
            freef( state->alloc, account_data );
          } while (0);

#if 0
          if (memcmp(hdr->info.owner, sprog, sizeof(sprog)) != 0) {
            // system progs are exempt no matter their lamports...
            ulong exempt = (hdr->meta.data_len + 128) * ((ulong) ((double)state->gen.rent.lamports_per_uint8_year * state->gen.rent.exemption_threshold));
            if (hdr->info.lamports < exempt) {
              odd++;
              char pubkey[50];
              fd_memset(pubkey, 0, sizeof(pubkey));
              fd_base58_encode_32((uchar *) hdr->meta.pubkey, pubkey);
              char owner[50];
              fd_memset(owner, 0, sizeof(owner));
              fd_base58_encode_32((uchar *) hdr->info.owner, owner);
              printf("file: %s owner: %s pubkey: %s datalen: %ld lamports: %ld  rent_epoch: %ld\n", buf, owner, pubkey, hdr->meta.data_len, hdr->info.lamports, hdr->info.rent_epoch);
            }
          }
#endif
          b += fd_ulong_align_up(hdr->meta.data_len + sizeof(*hdr), 8);
        }

        close(fd);
        freef(state->alloc, r);
    }
  }

  closedir(dir);
  regfree(&reg);

  FD_LOG_WARNING(("files %ld  accounts %ld  odd %ld", files, accounts, odd));

  return 0;
}

int slot_dump(global_state_t *state) {
  if (NULL == state->accounts)  {
    usage(state->argv[0]);
    exit(1);
  }
  regex_t reg;
  // Where were those regular expressions for snapshots?
  if (regcomp(&reg, "[0-9]+\\.[0-9]+", REG_EXTENDED) != 0) {
    FD_LOG_ERR(( "compile failed" ));
  }

  FD_LOG_WARNING(("starting read of %s", state->accounts));

  DIR *dir = opendir(state->accounts);

  struct dirent * ent;
  uchar sprog[32];
  fd_memset(sprog, 0, sizeof(sprog));

  ulong files = 0;
  ulong accounts = 0;
  ulong odd = 0;

  while ( NULL != (ent = readdir(dir)) ) {
    if ( regexec(&reg, ent->d_name, 0, NULL, 0) == 0 )  {
      struct stat s;
      char buf[1000];

      strcpy(buf, ent->d_name);
      char *p = buf;
      while (*p != '.') p++;
      *p = '\0';

      ulong slot = (ulong) atol(buf);
       
      if ((slot < state->start_slot) | (slot > state->end_slot))
        continue;

      sprintf(buf, "%s/%s", state->accounts, ent->d_name);
      stat(buf,  &s);
      unsigned char *r = (unsigned char *)allocf(state->alloc, 8UL, (unsigned long) (unsigned long) s.st_size);
      unsigned char *b = r;
      files++;
      int fd = open(buf, O_RDONLY);
      ssize_t n = read(fd, b, (unsigned long) s.st_size);
      if (n < 0) {
        FD_LOG_ERR(( "??" ));
      }
      unsigned char *eptr = &b[(ulong) ((ulong)n - (ulong)sizeof(fd_solana_account_hdr_t))];
      if (n != s.st_size) {
        FD_LOG_ERR(( "??" ));
      }

      while (b < eptr) {
        fd_solana_account_hdr_t *hdr = (fd_solana_account_hdr_t *)b;
        // Look for corruption...
        if ((b + hdr->meta.data_len) > (r + n)) {
          // some kind of corruption in the file?
          break;
        }
        // Sanitize accounts...
        if ((hdr->info.lamports == 0) | ((hdr->info.executable & ~1) != 0))
          break;

        accounts++;

        do {
          char pubkey[50];
          fd_memset(pubkey, 0, sizeof(pubkey));
          fd_base58_encode_32((uchar *) hdr->meta.pubkey, 0, pubkey);

          if (strcmp(pubkey, "SysvarC1ock11111111111111111111111111111111"))
            break;

          fd_solana_account_t account = {
            .lamports = hdr->info.lamports,
            .rent_epoch = hdr->info.rent_epoch,
            .data_len = hdr->meta.data_len,
            .data = b + sizeof(*hdr),
            .executable = (uchar) hdr->info.executable
          };
          fd_memcpy( account.owner.key, hdr->info.owner, 32 );
            
          uchar hash[32];
          fd_hash_account( &account, slot, (fd_pubkey_t const *)  &hdr->meta.pubkey, (fd_hash_t *) hash );

          char encoded_hash[50];
          fd_base58_encode_32((uchar *) hash, 0, encoded_hash);

          char owner[50];
          fd_memset(owner, 0, sizeof(owner));
          fd_base58_encode_32((uchar *) hdr->info.owner, 0, owner);
          printf("owner: %48s pubkey: %48s hash: %48s file: %s\n", owner, pubkey, encoded_hash, buf);

          struct fd_sol_sysvar_clock a;
          memset(&a, 0, sizeof(a));

          const void * o = &b[sizeof(*hdr)];
          unsigned char *outend = & (((unsigned char *) o)[hdr->meta.data_len]);

          fd_sol_sysvar_clock_decode(&a, &o, outend, allocf, state->alloc);
          printf("{slot = %ld, epoch_start_timestamp = %ld, epoch = %ld, leader_schedule_epoch = %ld, unix_timestamp = %ld}\n",
            a.slot, a.epoch_start_timestamp, a.epoch, a.leader_schedule_epoch, a.unix_timestamp);
          fd_sol_sysvar_clock_destroy(&a, freef, state->alloc);

        } while (0);

        b += fd_ulong_align_up(hdr->meta.data_len + sizeof(*hdr), 8);
      }

      close(fd);
      freef(state->alloc, r);
    }
  }

  closedir(dir);
  regfree(&reg);

  FD_LOG_WARNING(("files %ld  accounts %ld  odd %ld", files, accounts, odd));

  return 0;
}

int validate_bank_hashes(global_state_t *state) {
  if (NULL == state->accounts)  {
    usage(state->argv[0]);
    exit(1);
  }

  regex_t reg;
  // Where were those regular expressions for snapshots?
  if (regcomp(&reg, "[0-9]+\\.[0-9]+", REG_EXTENDED) != 0) {
    FD_LOG_ERR(( "compile failed" ));
  }

  FD_LOG_WARNING(("starting read of %s", state->accounts));

  DIR *dir = opendir(state->accounts);

  struct dirent * ent;

  ulong files = 0;
  ulong accounts = 0;
  ulong odd = 0;

  ulong pairs_len = 0;
  fd_pubkey_hash_pair_t pairs[1000];
  while ( NULL != (ent = readdir(dir)) ) {
    if ( regexec(&reg, ent->d_name, 0, NULL, 0) == 0 )  {
        struct stat s;
        char buf[1000];

        strcpy(buf, ent->d_name);
        char *p = buf;
        while (*p != '.') p++; // It surhdr->meta.pubkeye as heck better have a . or the regexec would fail
        *p = '\0';

        ulong slot = (ulong) atol(buf);
       
        if (slot != state->start_slot) {
          continue;
        }

        sprintf(buf, "%s/%s", state->accounts, ent->d_name);
        stat(buf,  &s);
        unsigned char *r = (unsigned char *)allocf(state->alloc, 8UL, (unsigned long) (unsigned long) s.st_size);
        unsigned char *b = r;
        files++;
        int fd = open(buf, O_RDONLY);
        ssize_t n = read(fd, b, (unsigned long) s.st_size);
        if (n < 0) {
          FD_LOG_ERR(( "??" ));
        }
        unsigned char *eptr = &b[(ulong) ((ulong)n - (ulong)sizeof(fd_solana_account_hdr_t))];
        if (n != s.st_size) {
          FD_LOG_ERR(( "??" ));
        }

        while (b < eptr) {
          fd_solana_account_hdr_t *hdr = (fd_solana_account_hdr_t *)b;
          // Look for corruption...
          if ((b + hdr->meta.data_len) > (r + n)) {
            // some kind of corruption in the file?
            break;
          }
          // Sanitize accounts...
          if ((hdr->info.lamports == 0) | ((hdr->info.executable & ~1) != 0))
            break;

          accounts++;

          if ((accounts % 1000000) == 0) {
            FD_LOG_WARNING(("accounts %ld", accounts));
          }

          do {
            fd_account_meta_t metadata;
            int read_result = fd_acc_mgr_get_metadata( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, &metadata );
            if ( FD_UNLIKELY( read_result == FD_ACC_MGR_SUCCESS ) ) {
              if (metadata.slot > slot)
                break;
            }

            if (fd_acc_mgr_write_append_vec_account( state->acc_mgr,  slot, hdr) != FD_ACC_MGR_SUCCESS) {
              FD_LOG_ERR(("writing failed: accounts %ld", accounts));
            }
            read_result = fd_acc_mgr_get_metadata( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, &metadata );
            if ( FD_UNLIKELY( read_result != FD_ACC_MGR_SUCCESS ) ) 
              FD_LOG_ERR(("wtf"));
            if ((metadata.magic != FD_ACCOUNT_META_MAGIC) || (metadata.hlen != sizeof(metadata))) 
              FD_LOG_ERR(("wtf2"));
            

            uchar * account_data = (uchar *) allocf(state->alloc , 8UL,  hdr->meta.data_len);
            fd_account_meta_t account_hdr;
            read_result = fd_acc_mgr_get_account_data( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, (uchar *) &account_hdr, 0, sizeof(fd_account_meta_t));
            
            read_result = fd_acc_mgr_get_account_data( state->acc_mgr, (fd_pubkey_t *) &hdr->meta.pubkey, account_data, sizeof(fd_account_meta_t), account_hdr.dlen);
            if ( FD_UNLIKELY( read_result != FD_ACC_MGR_SUCCESS ) ) 
              FD_LOG_ERR(("wtf3"));

            fd_solana_account_t account = {
              .lamports = hdr->info.lamports,
              .rent_epoch = hdr->info.rent_epoch,
              .data_len = hdr->meta.data_len,
              .data = account_data,
              .executable = (uchar) hdr->info.executable
            };
            fd_memcpy( account.owner.key, hdr->info.owner, 32 );
            
            uchar hash[32];
            fd_hash_account( &account, slot, (fd_pubkey_t const *)  &hdr->meta.pubkey, (fd_hash_t *) hash );
            //fd_hash_account( &account, 1, (fd_pubkey_t const *)  &pubkey, (fd_hash_t *) hash );
            
            // If account hashes are mismatched, fail
            if ( memcmp( hdr->hash.value, hash, 32 ) ) {
              FD_LOG_ERR(( "FAIL (account hashes mismatched)"
                 "\n\tGot"
                 "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT
                 "\n\tExpected"
                 "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
                 FD_LOG_HEX16_FMT_ARGS(     hdr->hash.value    ), FD_LOG_HEX16_FMT_ARGS(     hdr->hash.value+16 ),
                 FD_LOG_HEX16_FMT_ARGS(     hash    ), FD_LOG_HEX16_FMT_ARGS(     hash+16 ) 
              ));
            }
            fd_memcpy(pairs[pairs_len].pubkey.key, hdr->meta.pubkey, 32);
            fd_memcpy(pairs[pairs_len].hash.hash, hash, 32);
            pairs_len++;
            freef(state->alloc,  account_data);
          } while (0);

          b += fd_ulong_align_up(hdr->meta.data_len + sizeof(*hdr), 8);
        }

        close(fd);
        freef(state->alloc, r);
    }
  }

  closedir(dir);
  regfree(&reg);

  FD_LOG_WARNING(("files %ld  accounts %ld  odd %ld", files, accounts, odd));

  for (ulong i = 0; i < pairs_len; i++) {
    FD_LOG_NOTICE(( "Pairs (%lu)"
       "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT
       "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
       i,
       FD_LOG_HEX16_FMT_ARGS( pairs[i].pubkey.key ), FD_LOG_HEX16_FMT_ARGS( pairs[i].pubkey.key+16 ),
       FD_LOG_HEX16_FMT_ARGS( pairs[i].hash.hash ), FD_LOG_HEX16_FMT_ARGS( pairs[i].hash.hash+16 )));


  }

  struct stat s;
  stat(state->manifest,  &s);

  FD_LOG_WARNING(("reading manifest: %s", state->manifest));

  unsigned char *b = (unsigned char *)allocf(state->alloc, 1, (unsigned long) (unsigned long) s.st_size);
  int fd = open(state->manifest, O_RDONLY);
  ssize_t n = read(fd, b, (unsigned long) s.st_size);
  close(fd);

  FD_TEST(n == s.st_size);
  unsigned char *outend = &b[n];
  const void * o = b;

  FD_LOG_WARNING(("deserializing version bank"));

  struct fd_deserializable_versioned_bank a;
  memset(&a, 0, sizeof(a));
  fd_deserializable_versioned_bank_decode(&a, &o, outend, allocf, state->alloc);
  
  FD_LOG_WARNING(("deserializing accounts"));
  struct fd_solana_accounts_db_fields db;
  memset(&db, 0, sizeof(b));
  fd_solana_accounts_db_fields_decode(&db, &o, outend, allocf, state->alloc);

  FD_LOG_WARNING(("cleaning up"));

  fd_hash_t bank_hash;
  fd_hash_bank(&a, pairs, pairs_len, &bank_hash);
    FD_LOG_NOTICE(( "Bank Hash (%lu)"
       "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT
       "\n\t\t" FD_LOG_HEX16_FMT "  " FD_LOG_HEX16_FMT,
       a.slot,
       FD_LOG_HEX16_FMT_ARGS( bank_hash.hash ), FD_LOG_HEX16_FMT_ARGS( bank_hash.hash+16 ),
       FD_LOG_HEX16_FMT_ARGS( a.hash.hash ), FD_LOG_HEX16_FMT_ARGS( a.hash.hash+16 )));

  fd_deserializable_versioned_bank_destroy(&a, freef, state->alloc);
  fd_solana_accounts_db_fields_destroy(&db, freef, state->alloc);
  freef(state->alloc, b);

  return 0;
}

int manifest(global_state_t *state) {
  struct stat s;
  stat(state->manifest,  &s);

  FD_LOG_WARNING(("reading manifest: %s", state->manifest));

  unsigned char *b = (unsigned char *)allocf(state->alloc, 1, (unsigned long) (unsigned long) s.st_size);
  int fd = open(state->manifest, O_RDONLY);
  ssize_t n = read(fd, b, (unsigned long) s.st_size);
  close(fd);

  FD_TEST(n == s.st_size);
  unsigned char *outend = &b[n];
  const void * o = b;

  FD_LOG_WARNING(("deserializing version bank"));

  struct fd_deserializable_versioned_bank a;
  memset(&a, 0, sizeof(a));
  fd_deserializable_versioned_bank_decode(&a, &o, outend, allocf, state->alloc);
  
  for (ulong i = 0; i < a.ancestors_len; i++) {
    FD_LOG_WARNING(("QQQ %lu %lu", a.ancestors[i].slot, a.ancestors[i].val ));
  }

  FD_LOG_WARNING(("deserializing accounts"));
  struct fd_solana_accounts_db_fields db;
  memset(&db, 0, sizeof(b));
  fd_solana_accounts_db_fields_decode(&db, &o, outend, allocf, state->alloc);

  FD_LOG_WARNING(("cleaning up"));

  fd_deserializable_versioned_bank_destroy(&a, freef, state->alloc);
  fd_solana_accounts_db_fields_destroy(&db, freef, state->alloc);
  freef(state->alloc, b);

  return 0;
}

void
fd_sim_txn(global_state_t *state, FD_FN_UNUSED fd_executor_t* executor, fd_txn_t * txn, fd_rawtxn_b_t* txn_raw, struct fd_funk_xactionid const* funk_txn ) {

/*      The order of these addresses is important, because it determines the
     "permission flags" for the account in this transaction.
     Accounts ordered:
                                          Index Range                                 |   Signer?    |  Writeable?
     ---------------------------------------------------------------------------------|--------------|-------------
      [0,                                     signature_cnt - readonly_signed_cnt)    |  signer      |   writable
      [signature_cnt - readonly_signed_cnt,   signature_cnt)                          |  signer      |   readonly
      [signature_cnt,                         acct_addr_cnt - readonly_unsigned_cnt)  |  not signer  |   writable
      [acct_addr_cnt - readonly_unsigned_cnt, acct_addr_cnt)                          |  not signer  |   readonly
*/

  fd_pubkey_t *tx_accs   = (fd_pubkey_t *)((uchar *)txn_raw->raw + txn->acct_addr_off);
  for (ushort i = 0; i < txn->acct_addr_cnt; i++) {
    fd_pubkey_t *addr = &tx_accs[i];

      long what = -1;
      if (i < (txn->signature_cnt - txn->readonly_signed_cnt))
        what = 0;
      else if ((i >= (txn->signature_cnt - txn->readonly_signed_cnt)))
        what = 1;
      else if ((i >= txn->signature_cnt) && (i < (txn->acct_addr_cnt - txn->readonly_unsigned_cnt)))
        what = 2;
      else
        what = 3;

    fd_account_meta_t metadata;
    if ( fd_acc_mgr_get_metadata( state->acc_mgr, addr, &metadata ) != FD_ACC_MGR_SUCCESS) {
      if (what > 1) {
        char pubkey[33];
        fd_base58_encode_32((uchar *) addr, NULL, pubkey);
        FD_LOG_WARNING(("missing account: %ld %s", what, pubkey));
      }
      continue;
    }
    if ((what == 0) | (what == 2)) {
      metadata.info.lamports++;
      int write_result = fd_acc_mgr_write_account( state->acc_mgr, funk_txn, addr, (uchar*)&metadata, sizeof(metadata) );
      if ( FD_UNLIKELY( write_result != FD_ACC_MGR_SUCCESS ) ) {
        FD_LOG_ERR(("wtf"));
      }
    }
  }
}

int replay(global_state_t *state) {
  // Lets start executing!
  void *fd_executor_raw = malloc(FD_EXECUTOR_FOOTPRINT);
  fd_executor_t* executor = fd_executor_join(fd_executor_new(fd_executor_raw, state->acc_mgr, FD_EXECUTOR_FOOTPRINT));

  fd_rng_t rnd_mem;
  void *shrng = fd_rng_new(&rnd_mem, 0, 0);
  fd_rng_t * rng  = fd_rng_join( shrng );

  struct fd_funk_xactionid funk_txn;

  uchar boot_boh = 1;
  if (0 == state->start_slot) {
    fd_memcpy(state->poh.state, state->genesis_hash, sizeof(state->genesis_hash));
    boot_boh = 0;
  }

  fd_rocksdb_root_iter_t iter;
  fd_rocksdb_root_iter_new ( & iter );

  fd_slot_meta_t m;
  fd_memset(&m, 0, sizeof(m));

  int ret = fd_rocksdb_root_iter_seek ( &iter, &state->rocks_db, state->start_slot, &m, allocf, state->alloc);
  if (ret < 0) {
    FD_LOG_ERR(("fd_rocksdb_root_iter_seek returned %d", ret));
  }

  do {
    ulong slot;
    ret = fd_rocksdb_root_iter_slot ( & iter, &slot );
    if (ret < 0) {
      FD_LOG_ERR(("fd_rocksdb_root_iter_slot returned %d", ret));
    }

    if (slot >= state->end_slot)
      break;

    if ((slot % 10) == 0)
      FD_LOG_WARNING(("reading slot %ld", slot));

    if (state->txn_exe == 2) {
      ulong *p = (ulong *) &funk_txn.id[0];
      p[0] = fd_rng_ulong(rng);
      p[1] = fd_rng_ulong(rng);
      p[2] = fd_rng_ulong(rng);
      p[3] = fd_rng_ulong(rng);

      fd_funk_fork(state->funk, fd_funk_root(state->acc_mgr->funk), &funk_txn);
    }

    // SysvarS1otHashes111111111111111111111111111
    //new.update_slot_hashes();
    //                 .map(|account| from_account::<SlotHashes, _>(account).unwrap())
    //
    //                 pub type SlotHash = (Slot, Hash);
    //                 pub struct SlotHashes(Vec<SlotHash>);

    // SysvarS1otHistory11111111111111111111111111
    //new.update_stake_history(Some(parent_epoch));
    //                 .map(|account| from_account::<SlotHistory, _>(account).unwrap())
    //
    //pub struct SlotHistory {
    //  pub bits: BitVec<u64>,
    //  pub next_slot: Slot,
    // }

    //new.update_clock(Some(parent_epoch));

    // 
    //new.update_fees();

    // "SysvarRecentB1ockHashes11111111111111111111",
    // pub struct Entry {
    //   pub blockhash: Hash,
    //   pub fee_calculator: FeeCalculator,
    // }
    // pub struct RecentBlockhashes(Vec<Entry>);

    do {
      fd_slot_blocks_t *slot_data = fd_rocksdb_get_microblocks(&state->rocks_db, &m, allocf, state->alloc);

      // free 
      fd_slot_meta_destroy(&m, freef, state->alloc);

      if (NULL == slot_data) {
        FD_LOG_WARNING(("fd_rocksdb_get_microblocks returned NULL for slot %ld", slot));
        break;
      }

      ulong blob_idx = 0;
      ulong entry_idx = 0;
      // execute slot_block...
      uchar *blob = slot_data->first_blob;
      while (NULL != blob) {
        blob_idx++;
        uchar *blob_ptr = blob + FD_BLOB_DATA_START;
        uint cnt = *((uint *) (blob + 8));
        while (cnt > 0) {
          fd_microblock_t * micro_block = fd_microblock_join( blob_ptr );

          entry_idx++;

#ifdef _VHASH
          char outhash_base58[50];
          fd_memset(outhash_base58, 0, sizeof(outhash_base58));
#endif

          if (micro_block->txn_max_cnt > 0) {
            if (micro_block->hdr.hash_cnt > 0)
              fd_poh_append(&state->poh, micro_block->hdr.hash_cnt - 1);

            for ( ulong txn_idx = 0; txn_idx < micro_block->txn_max_cnt; txn_idx++ ) {
              fd_txn_t* txn_descriptor = (fd_txn_t *)&micro_block->txn_tbl[ txn_idx ];
              fd_rawtxn_b_t* txn_raw   = (fd_rawtxn_b_t *)&micro_block->raw_tbl[ txn_idx ];

              switch (state->txn_exe) {
              case 0:
                fd_execute_txn( executor, txn_descriptor, txn_raw );
                break;
              case 2:
                fd_sim_txn( state, executor, txn_descriptor, txn_raw, &funk_txn );
                break;
              default: // skip
                break;
              } // switch (state->txn_exe)
            } // for ( ulong txn_idx = 0; txn_idx < micro_block->txn_max_cnt; txn_idx++ )

            uchar outhash[32];
            fd_microblock_mixin(micro_block, outhash);

#ifdef _VHASH
            fd_base58_encode_32((uchar *) outhash, outhash_base58);
#endif

            fd_poh_mixin(&state->poh, outhash);
          } else
            fd_poh_append(&state->poh, micro_block->hdr.hash_cnt);

#ifdef _VHASH
          char block_hash[50];
          fd_base58_encode_32((uchar *) micro_block->hdr.hash, block_hash);

          char poh_state[50];
          fd_base58_encode_32((uchar *) state->poh.state, poh_state);

          FD_LOG_WARNING(( "poh at slot: %ld,  batch: %03ld,  entry: %03ld  hash_cnt: %03ld  block_hash: %s  poh_state: %s  mixin: %s", slot, blob_idx, entry_idx, micro_block->hdr.hash_cnt, block_hash, poh_state, outhash_base58));
#endif

          if (memcmp(micro_block->hdr.hash, state->poh.state, sizeof(state->poh.state))) {
            if (boot_boh) {
              fd_memcpy(state->poh.state, micro_block->hdr.hash, sizeof(state->poh.state));
              boot_boh = 0;
            } else
              FD_LOG_ERR(( "poh missmatch at slot: %ld,  batch: %ld,  entry: %ld", slot, blob_idx, entry_idx));
          }

          fd_microblock_leave(micro_block);

          blob_ptr = (uchar *) fd_ulong_align_up((ulong)blob_ptr + fd_microblock_footprint( micro_block->hdr.txn_cnt ), FD_MICROBLOCK_ALIGN);
        
          cnt--;
        } // while (cnt > 0) 
        blob = *((uchar **) blob);
      } // while (NULL != blob) 

      // free the slot data...
      fd_slot_blocks_destroy(slot_data, freef, state->alloc);
      freef(state->alloc, slot_data);
    } while (0);

    if (state->txn_exe == 2) 
      fd_funk_commit(state->funk, &funk_txn);

    ret = fd_rocksdb_root_iter_next ( &iter, &m, allocf, state->alloc);
    if (ret < 0) {
      FD_LOG_ERR(("fd_rocksdb_root_iter_seek returned %d", ret));
    }
  } while (1);

  fd_executor_delete(fd_executor_leave(executor));
  free(fd_executor_raw);

  FD_TEST( fd_rng_leave( rng )==shrng );

  return 0;
}

int main(int argc, char **argv) {
  fd_boot( &argc, &argv );

  global_state_t state;
  fd_memset(&state, 0, sizeof(state));

  state.argc = argc;
  state.argv = argv;

  state.end_slot = 73;
  state.start_slot = 0;

  state.name                = fd_env_strip_cmdline_cstr ( &argc, &argv, "--wksp",         NULL, NULL );
  state.ledger              = fd_env_strip_cmdline_cstr ( &argc, &argv, "--ledger",       NULL, NULL);
  state.db                  = fd_env_strip_cmdline_cstr ( &argc, &argv, "--db",           NULL, NULL);
  state.end_slot_opt        = fd_env_strip_cmdline_cstr ( &argc, &argv, "--end-slot",     NULL, NULL);
  state.start_slot_opt      = fd_env_strip_cmdline_cstr ( &argc, &argv, "--start-slot",   NULL, NULL);
  state.manifest            = fd_env_strip_cmdline_cstr ( &argc, &argv, "--manifest",     NULL, NULL);
  state.accounts            = fd_env_strip_cmdline_cstr ( &argc, &argv, "--accounts",     NULL, NULL);
  state.cmd                 = fd_env_strip_cmdline_cstr ( &argc, &argv, "--cmd",          NULL, NULL);
  state.txn_exe_opt         = fd_env_strip_cmdline_cstr ( &argc, &argv, "--txn-exe",     NULL, NULL);
  state.pages_opt           = fd_env_strip_cmdline_cstr ( &argc, &argv, "--pages",        NULL, NULL);
  const char *index_max_opt = fd_env_strip_cmdline_cstr ( &argc, &argv, "--index-max",    NULL, NULL);
  const char *validate_db   = fd_env_strip_cmdline_cstr ( &argc, &argv, "--validate",     NULL, NULL);

  if ((NULL == state.ledger) || (NULL == state.db)) {
    usage(argv[0]);
    exit(1);
  }

  if (state.txn_exe_opt) {
    state.txn_exe = (strcmp(state.txn_exe_opt, "skip") == 0) ? 1 : 0;
    state.txn_exe = (strcmp(state.txn_exe_opt, "sim") == 0) ? 2 : 0;
  }

  if (state.pages_opt) 
    state.pages = (ulong) atoi(state.pages_opt);
  else
    state.pages = 2;

  if( state.name ) {
    FD_LOG_NOTICE(( "Attaching to --wksp %s", state.name ));
    state.wksp = fd_wksp_attach( state.name );
    FD_LOG_NOTICE(("attach complete"));
  } else {
    FD_LOG_NOTICE(( "--wksp not specified, using an anonymous local workspace" ));
    state.wksp = fd_wksp_new_anonymous( FD_SHMEM_GIGANTIC_PAGE_SZ, state.pages, 0, "wksp", 0UL );
    FD_LOG_NOTICE(("attach complete"));
  } 

  if( FD_UNLIKELY( !state.wksp ) ) FD_LOG_ERR(( "Unable to attach to wksp" ));

  void * shmem = fd_wksp_alloc_laddr( state.wksp, fd_alloc_align(), fd_alloc_footprint(), 1 );

  FD_LOG_NOTICE(("fd_wksp_alloc_laddr complete"));

  if( FD_UNLIKELY( !shmem ) ) FD_LOG_ERR(( "Unable to allocate wksp memory for fd_alloc" ));

  void * shalloc = fd_alloc_new ( shmem, 1 ); 

  state.alloc = fd_alloc_join( shalloc, 0UL );

  ulong index_max = 1000000;    // Maximum size (count) of master index

  if (index_max_opt) 
    index_max = (ulong) atoi((char *) index_max_opt);

  ulong xactions_max = 100;     // Maximum size (count) of transaction index
  ulong cache_max = 10000;      // Maximum number of cache entries

  FD_LOG_NOTICE(("opening fd_funk db"));

  state.funk = fd_funk_new(state.db, state.wksp, 2, index_max, xactions_max, cache_max);

  if ((validate_db != NULL) && (strcmp(validate_db, "true") == 0)) {
    FD_LOG_WARNING(("starting validating %ld records", fd_funk_num_records(state.funk)));
    fd_funk_validate(state.funk);
    FD_LOG_WARNING(("finishing validate"));
  } else
    FD_LOG_WARNING(("found %ld records", fd_funk_num_records(state.funk)));

  if (NULL != state.end_slot_opt)
    state.end_slot = (ulong) atoi(state.end_slot_opt);
  if (NULL != state.start_slot_opt) 
    state.start_slot = (ulong) atoi(state.start_slot_opt);

  // Eventually we will have to add support for reading compressed genesis blocks...
  char genesis[128];
  sprintf(genesis, "%s/genesis.bin", state.ledger);

  char db_name[128];
  sprintf(db_name, "%s/rocksdb", state.ledger);

  {
    struct stat sbuf;
    stat(genesis, &sbuf);
    int fd = open(genesis, O_RDONLY);
    if (fd < 0) {
      FD_LOG_ERR(("Cannot open %s", genesis));
    }
    uchar *buf = malloc((ulong) sbuf.st_size);
    ssize_t n = read(fd, buf, (ulong) sbuf.st_size);
    close(fd);

    // TODO: Richie? . review?
    fd_sha256_t sha;
    fd_sha256_init( &sha );
    fd_sha256_append( &sha, buf, (ulong) n );
    fd_sha256_fini( &sha, state.genesis_hash );

    //DDaHhm7PCCf6a2s2YxvD5mBcp2NfDkiWr61sBW4nuN7
    char hash[100];
    fd_base58_encode_32((uchar *) state.genesis_hash, NULL, hash);
    
    void *data = buf;
    void *dataend = &buf[n];
    fd_memset(&state.gen, 0, sizeof(state.gen));
    fd_genesis_solana_decode(&state.gen, ( void const** )&data, dataend, allocf, state.alloc);

    free(buf);
  }

  // Jam all the accounts into the database....  (gen.accounts)

  /* Initialize the account manager */
  struct fd_funk_xactionid const* xroot = fd_funk_root(state.funk);

  void *fd_acc_mgr_raw = allocf(state.alloc, FD_ACC_MGR_ALIGN, FD_ACC_MGR_FOOTPRINT);
  state.acc_mgr = fd_acc_mgr_join(fd_acc_mgr_new(fd_acc_mgr_raw, state.funk, xroot, FD_ACC_MGR_FOOTPRINT));

  FD_LOG_WARNING(("loading genesis account into funk db"));

  for (ulong i = 0; i < state.gen.accounts_len; i++) {
    fd_pubkey_account_pair_t *a = &state.gen.accounts[i];

    fd_acc_mgr_write_structured_account(state.acc_mgr, 0, &a->key, &a->account);
  }

  for (ulong i = 0; i < state.gen.native_instruction_processors_len; i++) {
    fd_string_pubkey_pair_t * ins = &state.gen.native_instruction_processors[i];

    char pubkey[50];

    fd_base58_encode_32((uchar *) ins->pubkey.key, NULL, pubkey);

    FD_LOG_WARNING(("native program:  %s <= %s", ins->string, pubkey));
  }

  //  we good?
//  FD_LOG_WARNING(("validating funk db"));
//  fd_funk_validate(state.funk);

  // Initialize the rocksdb 
  char *err = fd_rocksdb_init(&state.rocks_db, db_name);

  if (err != NULL) {
    FD_LOG_ERR(("fd_rocksdb_init returned %s", err));
  }

  ulong last_slot = fd_rocksdb_last_slot(&state.rocks_db, &err);
  if (err != NULL) {
    FD_LOG_ERR(("fd_rocksdb_last_slot returned %s", err));
  }

  if (state.end_slot > last_slot) {
    state.end_slot = last_slot;
    FD_LOG_WARNING(("setting the end_slot to %ld since that is the last slot we see in the rocksdb", state.end_slot));
  }

  ulong first_slot = fd_rocksdb_first_slot(&state.rocks_db, &err);
  if (err != NULL) {
    FD_LOG_ERR(("fd_rocksdb_first_slot returned %s", err));
  }

  if (state.start_slot < first_slot) {
    state.start_slot = first_slot;
    FD_LOG_WARNING(("setting the start_slot to %ld since that is the first slot we see in the rocksdb", state.start_slot));
  }

  if (strcmp(state.cmd, "replay") == 0)
    replay(&state);
  if (strcmp(state.cmd, "manifest") == 0)
    manifest(&state);
  if (strcmp(state.cmd, "ingest") == 0)
    ingest(&state);
  if (strcmp(state.cmd, "validate") == 0)
    validate_bank_hashes(&state);
  if (strcmp(state.cmd, "accounts") == 0)
    slot_dump(&state);

  fd_acc_mgr_delete(fd_acc_mgr_leave(state.acc_mgr));
  freef(state.alloc, fd_acc_mgr_raw);

  fd_genesis_solana_destroy(&state.gen, freef, state.alloc);

  // The memory management model is odd...  how do I know how to destroy this
  fd_rocksdb_destroy(&state.rocks_db);

//  fd_alloc_fprintf( state.alloc, stdout );

//  fd_alloc_free(state.alloc, fd_funk_raw);

  fd_funk_delete(state.funk);

  // ??
  // ulong       wksp_tag = 1UL;
  //fd_wksp_tag_free(state.wksp, &wksp_tag, 1UL);

  fd_wksp_free_laddr( shmem );

  // dump wksp state

  if( state.name ) 
    fd_wksp_detach( state.wksp );
  else  
    fd_wksp_delete_anonymous( state.wksp );

  FD_LOG_NOTICE(( "pass" ));

  fd_halt();

  return 0;
}
