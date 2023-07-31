#!/bin/bash -f

set -e

# this assumes the test_runtime has already been built

LEDGER="test-ledger-4"
VERBOSE=NO
POSITION_ARGS=()

while [[ $# -gt 0 ]]; do
  case $1 in
    -l|--ledger)
       LEDGER="$2"
       shift
       shift
       ;;
    -v|--verbose)
       VERBOSE=YES
       shift
       ;;
    -*|--*)
       echo "unknown option $1"
       exit 1
       ;;
    *)
       POSITION_ARGS+=("$1")
       shift
       ;;
  esac
done

if [ ! -e $LEDGER ]; then
  curl -o - -L -q https://github.com/firedancer-io/firedancer-testbins/raw/main/$LEDGER.tar.gz | tar zxf -
fi

# We determine these values by
#  1) Checkout https://github.com/firedancer-io/solana.git
#  2) switch to the debug branch
#  3) build using podman (podman build --no-cache -t solana-builder2 -f Dockerfile -v `pwd`:/solana:rw  .)
#  4) ./target/debug/solana-test-validator --reset
#         Do stuff
#  5) grep 'bank frozen:' test-ledger/validator.log | grep 'solana_runtime::bank'

# we could ALWAYS run it with logging except when I run this from the command line, I want less noise...

# sudo build/native/gcc/bin/fd_shmem_cfg fini

# sudo build/native/gcc/bin/fd_shmem_cfg init 0777 jsiegel ""
# sudo build/native/gcc/bin/fd_shmem_cfg alloc 64 gigantic 0
# sudo build/native/gcc/bin/fd_shmem_cfg alloc 512 huge 0

set -x

if [ $VERBOSE == "YES" ]; then
  set -x
fi

build/native/gcc/bin/fd_frank_ledger --rocksdb $LEDGER/rocksdb --genesis $LEDGER/genesis.bin --cmd ingest --indexmax 10000 --txnmax 100 --backup test_ledger_backup

build/native/gcc/unit-test/test_runtime --load test_ledger_backup --cmd replay --end-slot 25 --confirm_hash AsHedZaZkabNtB8XBiKWQkKwaeLy2y4Hrqm6MkQALT5h --confirm_parent CvgPeR54qpVRZGBuiQztGXecxSXREPfTF8wALujK4WdE --confirm_account_delta 7PL6JZgcNy5vkPSc6JsMHET9dvpvsFMWR734VtCG29xN  --confirm_signature 2  --confirm_last_block G4YL2SieHDGNZGjiwBsJESK7jMDfazg33ievuCwbkjrv --validate true >& /tmp/ledger_log$$

status=$?

if [ $status -ne 0 ]
then
  tail -20 /tmp/ledger_log$$
  echo 'ledger test failed:'
  echo /tmp/ledger_log$$
  exit $status
fi

build/native/gcc/unit-test/test_native_programs --filter 'vote|system|config' >& native.log

status=$?

if [ $status -ne 0 ]
then
  echo 'native test failed'
  grep "Failed" native.log | tail -20
  exit $status
fi

grep "Progress" native.log

echo 'simple tests passed'
