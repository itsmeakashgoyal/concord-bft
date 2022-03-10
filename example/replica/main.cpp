#include "Setup.hpp"
#include "Replica.h"
#include "InternalCommandsHandler.hpp"
#include "replica_state_sync_imp.hpp"
#include "block_metadata.hpp"
#include "SimpleBCStateTransfer.hpp"
#include "secrets_manager_plain.h"
#include "bftengine/ControlStateManager.hpp"
#include "messages/ReplicaRestartReadyMsg.hpp"
#include "bftengine/ReconfigurationCmd.hpp"
#include "client/reconfiguration/cre_interfaces.hpp"
#include "assertUtils.hpp"
#include "Metrics.hpp"

#ifdef USE_ROCKSDB
#include "rocksdb/client.h"
#include "rocksdb/key_comparator.h"
#endif

#include <csignal>
#include <memory>
#include <unistd.h>

namespace concord::kvbc::example {
std::atomic_bool timeToExit = false;
std::shared_ptr<concord::kvbc::Replica> replica(nullptr);

void cronSetup(ReplicaSetup& setup, const Replica& replica) {
  if (!setup.GetCronEntryNumberOfExecutes()) {
    return;
  }
  const auto numberOfExecutes = *setup.GetCronEntryNumberOfExecutes();

  using namespace concord::cron;
  const auto cronTableRegistry = replica.cronTableRegistry();
  const auto ticksGenerator = replica.ticksGenerator();

  auto& cronTable = cronTableRegistry->operator[](ReplicaSetup::kCronTableComponentId);

  // Make sure these are available for rules and actions that are called in the main replica thread.
  static auto metricsComponent =
      std::make_shared<concordMetrics::Component>("cron_test", setup.GetMetricsServer().GetAggregator());
  static auto numberOfExecutesHandle = metricsComponent->RegisterGauge("cron_entry_number_of_executes", 0);
  static auto tickComponentIdHandle = metricsComponent->RegisterStatus("cron_ticks_component_id", "");
  static auto currentNumberOfExecutes = 0u;

  // Register the component.
  metricsComponent->Register();

  // Add a cron entry
  constexpr auto entryPos = 0;
  const auto rule = [numberOfExecutes](const Tick&) { return (currentNumberOfExecutes < numberOfExecutes); };
  const auto action = [](const Tick& tick) {
    ++currentNumberOfExecutes;
    numberOfExecutesHandle++;
    tickComponentIdHandle.Get().Set(std::to_string(tick.component_id));
    metricsComponent->UpdateAggregator();
  };
  cronTable.addEntry({entryPos, rule, action});

  // Start the ticks generator.
  ticksGenerator->start(ReplicaSetup::kCronTableComponentId, ReplicaSetup::kTickGeneratorPeriod);
}

void runReplica(int argc, char** argv) {
  const auto setup = ReplicaSetup::ParseArgs(argc, argv);
  logging::initLogger(setup->getLogPropertiesFile());
  auto logger = setup->GetLogger();
  MDC_PUT(MDC_REPLICA_ID_KEY, std::to_string(setup->GetReplicaConfig().replicaId));
  MDC_PUT(MDC_THREAD_KEY, "main");

  replica = std::make_shared<Replica>(setup->GetCommunication(),
                                      setup->GetReplicaConfig(),
                                      setup->GetStorageFactory(),
                                      setup->GetMetricsServer().GetAggregator(),
                                      setup->GetPerformanceManager(),
                                      std::map<std::string, categorization::CATEGORY_TYPE>{
                                          {VERSIONED_KV_CAT_ID, categorization::CATEGORY_TYPE::versioned_kv},
                                          {BLOCK_MERKLE_CAT_ID, categorization::CATEGORY_TYPE::block_merkle}},
                                      setup->GetSecretManager());
  bftEngine::ControlStateManager::instance().addOnRestartProofCallBack(
      [argv, &setup]() {
        setup->GetCommunication()->stop();
        setup->GetMetricsServer().Stop();
        execv(argv[0], argv);
      },
      static_cast<uint8_t>(ReplicaRestartReadyMsg::Reason::Scale));

  auto* blockMetadata = new BlockMetadata(*replica);

  if (!setup->GetReplicaConfig().isReadOnly) replica->setReplicaStateSync(new ReplicaStateSyncImp(blockMetadata));

  auto cmdHandler =
      std::make_shared<InternalCommandsHandler>(replica.get(),
                                                replica.get(),
                                                blockMetadata,
                                                logger,
                                                setup->AddAllKeysAsPublic(),
                                                replica->kvBlockchain() ? &replica->kvBlockchain().value() : nullptr);
  replica->set_command_handler(cmdHandler);
  replica->setStateSnapshotValueConverter(categorization::KeyValueBlockchain::kNoopConverter);
  replica->start();

  // Setup a test cron table, if requested in configuration.
  cronSetup(*setup, *replica);

  // Start metrics server after creation of the replica so that we ensure
  // registration of metrics from the replica with the aggregator and don't
  // return empty metrics from the metrics server.
  setup->GetMetricsServer().Start();
  while (replica->isRunning()) {
    if (timeToExit) {
      setup->GetMetricsServer().Stop();
      replica->stop();
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}
}  // namespace concord::kvbc::example

using namespace std;

namespace {
static void signal_handler(int signal_num) {
  LOG_INFO(GL, "Program received signal " << signal_num);
  concord::kvbc::example::timeToExit = true;
}
}  // namespace
int main(int argc, char** argv) {
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  try {
    concord::kvbc::example::runReplica(argc, argv);
  } catch (const std::exception& e) {
    LOG_FATAL(GL, "exception: " << e.what());
  }
  return 0;
}