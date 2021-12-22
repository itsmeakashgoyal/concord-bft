#!/usr/bin/env sh
################################################################################
#                              run-codecoverage                                #
#                                                                              #
# This script is used to run prepare-code-coverage-artifact.py python file to  #
# to generate code coverage report for apollo tests.                           #
#                                                                              #
# Help:                                                                        #
#  If do not want to preserve raw profile data with the coverage report.       #
#  > python3 $dir_path$py_file $raw_file_path_arg                              #
#                                                                              #
#  If want to preserve raw profile data with the coverage report.              #
#  > python3 $dir_path$py_file $raw_file_path_arg $optional_arg                #
#                                                                              #
################################################################################

echo "Usage: run-codecoverage.sh"

dir_path="scripts"
py_file="/prepare-code-coverage-artifact.py"
optional_arg="--preserve-profiles"
profile_data_dir="build/"
binaries="build/thin-replica-server/test/thin_replica_server_test
build/thin-replica-server/test/trs_sub_buffer_test
build/thin-replica-server/test/replica_state_snapshot_service_test
build/util/test/thread_pool_test
build/util/test/metric_server
build/util/test/mt_tests
build/util/test/serializable_test
build/util/test/simple_memory_pool_test
build/util/test/callback_registry_test
build/util/test/utilization_test
build/util/test/RawMemoryPool_test
build/util/test/synchronized_value_test
build/util/test/crypto_utils_test
build/util/test/openssl_crypto_wrapper_test
build/util/test/sha_hash_tests
build/util/test/metric_tests
build/util/test/RollingAvgAndVar_test
build/util/test/scope_exit_test
build/util/test/sliver_test
build/util/test/lru_cache_test
build/util/test/hex_tools_test
build/util/test/timers_tests
build/bftclient/test/bft_client_api_tests
build/bftclient/test/bft_client_test
build/diagnostics/test/histogram_tests
build/diagnostics/test/diagnostics_tests
build/diagnostics/test/diagnostics_server_main
build/ccron/test/ccron_ticks_generator_test
build/ccron/test/ccron_table_test
build/threshsign/bin/bench/BenchLagrange
build/threshsign/bin/bench/BenchMultiExp
build/threshsign/bin/bench/BenchRelic
build/threshsign/bin/bench/BenchThresholdBls
build/threshsign/bin/test/TestRelicSerialization
build/threshsign/bin/test/TestGroupElementSizes
build/threshsign/bin/test/TestRelic
build/threshsign/bin/test/TestThresholdBls
build/threshsign/bin/test/TestLagrange
build/threshsign/bin/test/TestBlsBatchVerifier
build/threshsign/bin/test/TestMisc
build/threshsign/bin/test/TestVectorOfShares
build/kvbc/test/resources-manager/resource_manager_test
build/kvbc/test/pruning_test
build/kvbc/test/replica_resources_test
build/kvbc/test/multiIO_test
build/kvbc/test/replica_state_sync_test
build/kvbc/test/immutable_kv_category_unit_test
build/kvbc/test/categorized_kv_blockchain_unit_test
build/kvbc/test/kvbc_filter_test
build/kvbc/test/categorized_blocks_unit_test
build/kvbc/test/sparse_merkle_storage_serialization_unit_test
build/kvbc/test/sparse_merkle_internal_node_test
build/kvbc/test/kvbc_dbadapter_test
build/kvbc/test/order_test
build/kvbc/test/kv_blockchain_db_editor_test
build/kvbc/test/categorized_blockchain_unit_test
build/kvbc/test/sparse_merkle_tree_test
build/kvbc/test/sparse_merkle_base_types_test
build/kvbc/test/versioned_kv_category_unit_test
build/kvbc/test/sparse_merkle_internal_node_property_test
build/kvbc/test/sparse_merkle_storage_db_adapter_property_test
build/kvbc/test/block_merkle_latest_ver_cf_migration_test
build/kvbc/test/block_merkle_category_unit_test
build/kvbc/test/sparse_merkle_storage_db_adapter_unit_test
build/kvbc/test/blockchain_view_test
build/kvbc/test/pruning_reserved_pages_client_test
build/kvbc/benchmark/kvbcbench/kvbcbench
build/kvbc/benchmark/state_snapshot_benchmarks/hash_state_benchmark
build/kvbc/tools/migrations/block_merkle_latest_ver_cf_migration_tool
build/kvbc/tools/db_editor/kv_blockchain_db_editor
build/client/concordclient/test/cc_basic_update_queue_tests
build/client/clientservice/clientservice
build/client/clientservice/test/clientservice-test-yaml_parsing
build/client/reconfiguration/test/cre_test_api
build/client/reconfiguration/test/poll_based_state_client_test
build/client/thin-replica-client/test/thin_replica_client_tests
build/client/thin-replica-client/test/trc_rpc_use_tests
build/client/thin-replica-client/test/trc_byzantine_tests
build/client/thin-replica-client/test/trc_hash_tests
build/client/thin-replica-client/test/replica_stream_snapshot_client_tests
build/storage/test/s3_client_test
build/storage/test/native_rocksdb_client_test
build/storage/s3_integrity_check
build/tests/simpleTest/server
build/tests/simpleTest/persistency_test
build/tests/simpleTest/config
build/tests/simpleTest/client
build/bftengine/src/preprocessor/tests/preprocessor_test
build/bftengine/src/preprocessor/tests/messages/PreProcessResultMsg_test
build/bftengine/src/preprocessor/tests/messages/ClientPreProcessRequestMsg_test
build/bftengine/src/preprocessor/tests/messages/PreProcessReplyMsg_test
build/bftengine/tests/testSequenceWithActiveWindow/SequenceWithActiveWindow_tests
build/bftengine/tests/KeyStore/KeyStore_test
build/bftengine/tests/bcstatetransfer/source_selector_test
build/bftengine/tests/bcstatetransfer/bcstatetransfer_tests
build/bftengine/tests/controllerWithSimpleHistory/ControllerWithSimpleHistory_test
build/bftengine/tests/incomingMsgsStorage/incomingMsgsStorage_test
build/bftengine/tests/SigManager/SigManager_test
build/bftengine/tests/testRequestThreadPool/RequestThreadPool_test
build/bftengine/tests/timeServiceResPageClient/TimeServiceResPageClient_test
build/bftengine/tests/testMsgsCertificate/msgsCertificate_test
build/bftengine/tests/metadataStorage/metadataStorage_test
build/bftengine/tests/testSerialization/test_serialization
build/bftengine/tests/testSeqNumForClientRequest/seqNumForClientRequest_test
build/bftengine/tests/timeServiceManager/TimeServiceManager_test
build/bftengine/tests/testViewChange/views_manager_test
build/bftengine/tests/testViewChange/ViewChange_tests
build/bftengine/tests/clientsManager/ClientsManager_test
build/bftengine/tests/messages/ViewChangeMsg_test
build/bftengine/tests/messages/ReplicaRestartReadyMsg_test
build/bftengine/tests/messages/FullCommitProofMsg_test
build/bftengine/tests/messages/PrePrepareMsg_test
build/bftengine/tests/messages/CheckpointMsg_test
build/bftengine/tests/messages/ReqMissingDataMsg_test
build/bftengine/tests/messages/AskForCheckpointMsg_test
build/bftengine/tests/messages/ReplicaAsksToLeaveViewMsg_test
build/bftengine/tests/messages/StartSlowCommitMsg_test
build/bftengine/tests/messages/ClientRequestMsg_test
build/bftengine/tests/messages/ReplicaStatusMsg_test
build/bftengine/tests/messages/NewViewMsg_test
build/bftengine/tests/messages/PartialCommitProofMsg_test
build/bftengine/tests/messages/SignedShareBase_test
build/secretsmanager/test/secrets_manager_test"

python3 $dir_path$py_file $profile_data_dir $binaries "--unified-report"
