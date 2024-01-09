#!/bin/bash
set -euxo pipefail
IFS=$'\n\t'

# create temporary files in the user's home directory because it's likely to be on a large disk
TMPDIR=$(mktemp --directory --tmpdir="$HOME" tmp-test-tvu.XXXXXX)
cd $TMPDIR

cleanup() {
  sudo killall solana-validator || true
  sudo killall test_tvu || true
  rm -rf "$TMPDIR"
}

trap cleanup EXIT SIGINT SIGTERM

SOLANA_BIN_DIR="$HOME/code/solana/target/release"
FD_DIR="$HOME/code/firedancer-private"

sudo killall solana-validator || true

# if solana is not on path then use the one in the home directory
if ! command -v solana > /dev/null; then
  PATH=$SOLANA_BIN_DIR:$PATH
fi

# if fd_frank_ledger is not on path then use the one in the home directory
if ! command -v fd_frank_ledger > /dev/null; then
  PATH="$FD_DIR/build/native/gcc/bin":$PATH
fi
# if test_tvu is not on path then use the one in the home directory
if ! command -v test_tvu > /dev/null; then
  PATH="$FD_DIR/build/native/gcc/unit-test":$PATH
fi


echo "Creating mint and stake authority keys..."
solana-keygen new --no-bip39-passphrase -o faucet.json > /dev/null
solana-keygen new --no-bip39-passphrase -o authority.json > /dev/null

# Create bootstrap validator keys
echo "Creating keys for Validator"
solana-keygen new --no-bip39-passphrase -o id.json > id.seed
solana-keygen new --no-bip39-passphrase -o vote.json > vote.seed
solana-keygen new --no-bip39-passphrase -o stake.json > stake.seed

# Genesis
echo "Running Genesis..."

GENESIS_OUTPUT=$(solana-genesis \
    --cluster-type mainnet-beta \
    --ledger ledger \
    --enable-warmup-epochs \
    --bootstrap-validator id.json vote.json stake.json \
    --bootstrap-stake-authorized-pubkey id.json \
    --bootstrap-validator-lamports 2399348000000000 \
    --bootstrap-validator-stake-lamports 32282880 \
    --faucet-pubkey faucet.json --faucet-lamports 5000000000000000)

# Start the bootstrap validator
GENESIS_HASH=$(echo $GENESIS_OUTPUT | grep -o -P '(?<=Genesis hash:).*(?=Shred version:)' | xargs)
SHRED_VERSION=$(echo $GENESIS_OUTPUT | grep -o -P '(?<=Shred version:).*(?=Ticks per slot:)' | xargs)
_PRIMARY_INTERFACE=$(ip route show default | awk '/default/ {print $5}')
PRIMARY_IP=$(ip addr show $_PRIMARY_INTERFACE | awk '/inet / {print $2}' | cut -d/ -f1)

RUST_LOG=trace solana-validator \
    --identity id.json \
    --ledger ledger \
    --limit-ledger-size 100000000 \
    --no-genesis-fetch \
    --no-snapshot-fetch \
    --no-poh-speed-test \
    --no-os-network-limits-test \
    --vote-account $(solana-keygen pubkey vote.json) \
    --expected-shred-version $SHRED_VERSION \
    --expected-genesis-hash $GENESIS_HASH \
    --no-wait-for-vote-to-start-leader \
    --incremental-snapshots \
    --full-snapshot-interval-slots 200 \
    --rpc-port 8899 \
    --gossip-port 8001 \
    --gossip-host $PRIMARY_IP \
    --full-rpc-api \
    --log validator.log &

sleep 90

while [ $(solana -u localhost epoch-info --output json | jq .blockHeight) -le 250 ]; do
  sleep 1
done

wget --trust-server-names http://localhost:8899/snapshot.tar.bz2

sudo "$FD_DIR/build/native/gcc/bin/fd_shmem_cfg" alloc 100 gigantic 0

timeout 30 test_tvu \
    --rpc-port 9999 \
    --gossip-peer-addr $PRIMARY_IP:8001 \
    --repair-peer-addr $PRIMARY_IP:8008 \
    --repair-peer-id $(solana-keygen pubkey id.json) \
    --snapshot snapshot* \
    --page-cnt 100 \
    --log-level-logfile 0 \
    --log-level-stderr 0 >test_tvu.log 2>&1 || true
    
grep -q "verified block successfully" test_tvu.log

