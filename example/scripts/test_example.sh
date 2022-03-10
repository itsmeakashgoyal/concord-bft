#!/bin/bash -e

cleanup() {
  killall -q test_replica || true
  killall -q minio || true
  rm -rf gen-sec.*
}

#trap 'cleanup' SIGINT

cleanup

echo "Generating new keys..."
../../tools/GenerateConcordKeys -f 1 -n 4 -o replica_keys_

# Generates num_participants number of key pairs
./create_concord_clients_transaction_signing_keys.sh -n 5 -o /tmp

# Generate TLS certificates
./create_tls_certs.sh 10 /tmp/certificates

# run 4 replica's with unique replica id's
echo "Running replica 1..."
../replica/test_replica -i 0 -a replica_conf &
echo "Running replica 2..."
../replica/test_replica -i 1 -a replica_conf &
echo "Running replica 3..."
../replica/test_replica -i 2 -a replica_conf &
echo "Running replica 4..."
../replica/test_replica -i 3 -a replica_conf &

# ../test_client -i 5 &

env MINIO_ROOT_USER=concordbft MINIO_ROOT_PASSWORD=concordbft ~/minio server minio_data_dir &

sleep 5

echo "Finished!"
#cleanup
