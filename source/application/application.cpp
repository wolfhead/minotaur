/**
 * @file application.cpp
 * @author Wolfhead
 */
#include "application.h"
#include <unistd.h>
#include <boost/program_options.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h> 
#include "config_manager.h"
#include "../net/acceptor.h"
#include "../net/io_descriptor_factory.h"
#include "../service/service.h"
#include "../client/client_manager.h"

namespace minotaur {

LOGGER_CLASS_IMPL(logger, Application);

Application::Application() 
    : logger_watcher_(NULL)
    , config_manager_(NULL)
    , client_manager_(NULL)
    , help_mode_(false)
    , daemon_mode_(false) {
}

Application::~Application() {
  if (logger_watcher_) {
    delete logger_watcher_;
    logger_watcher_ = NULL;
  }
  if (config_manager_) {
    delete config_manager_;
    config_manager_ = NULL;
  }
  if (client_manager_) {
    delete client_manager_;
    client_manager_ = NULL;
  }
}

int Application::Run(int argc, char* argv[]) {
  if (0 != ParseCmd(argc, argv)) {
    return -1;
  }

  if (help_mode_) {
    return 0; 
  }

  if (0 != InitApplication()) {
    return -1;
  }

  if (0 != StartLogger()) {
    return -1;
  }

  if (0 != LoadConfig()) {
    return -1;
  }

  DumpStatus(std::cout);

  if (0 != StartIOService()) {
    return -1;
  }

  if (0 != StartClientManager()) {
    return -1;
  }

  if (0 != OnStart()) {
    return -1;
  }

  if (0 != StartServiceManager()) {
    return -1;
  }

  if (0 != RunIOService()) {
    return -1;
  }

  OnStop();

  StopServiceManager();

  StopClientManager();

  StopIOService();
    
  OnFinish();

  return 0;
}

void Application::DumpStatus(std::ostream& os) const {
  os << "version_info: " << version_info_ << std::endl
     << "logger_config: " << logger_config_ << std::endl 
     << "application_config: " << application_config_ << std::endl
     << "help_mode: " << (help_mode_ ? "true" : "false") << std::endl
     << "daemon_mode: " << (daemon_mode_ ? "true" : "false") << std::endl;
  if (config_manager_) {
    os << "========== ConfigManager ========" << std::endl;
    config_manager_->Dump(os);
  }
}

int Application::ParseCmd(int argc, char* argv[]) {
  namespace po = boost::program_options;
  po::options_description desc("command line option");
  desc.add_options()
      ("logger,l", po::value<std::string>(), "log4cplus config file path")
      ("config,c", po::value<std::string>(), "application config file path")
      ("daemon,d", "use daemon mode")
      ("help,h", "produce help message");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    help_mode_ = true;
    return 0;
  }

  if (vm.count("daemon")) {
    daemon_mode_ = true;
  }

  if (vm.count("logger")) {
    logger_config_ = vm["logger"].as<std::string>();
  }

  if (vm.count("config")) {
    application_config_ = vm["config"].as<std::string>(); 
  }

  return 0;
}

int Application::InitApplication() {
  if (daemon_mode_) {
    daemon(1, 0);
  }
  return 0;
}

int Application::StartLogger() {
  if (logger_config_.empty()) {
    std::cout << "Initialize logger with default configuration" << std::endl;

    log4cplus::SharedAppenderPtr appender (new log4cplus::ConsoleAppender());
    std::auto_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout("%D{[%Y/%m/%d-%H:%M:%S]} (%t) %-5p%x - %m [%l]%n"));
    appender->setLayout(layout);
    log4cplus::Logger logger = log4cplus::Logger::getRoot();
    logger.addAppender(appender);
    logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);  
  } else {
    logger_watcher_ = new log4cplus::ConfigureAndWatchThread(logger_config_, 5000);
  }
  return 0;
}

int Application::LoadConfig() {
  if (!application_config_.empty()) {
    ConfigManager* config_manager = CreateConfigManager();
    if (0 != config_manager->LoadConfig(application_config_)) {
      return -1;
    }
    io_service_config_ = config_manager->GetIOServiceConfig();
    config_manager_ = config_manager;
  } else {
    MI_LOG_INFO(logger, "Load with default configuration");
    io_service_config_.event_loop_worker = 1;
    io_service_config_.fd_count = 65535;
    io_service_config_.io_queue_size = 128 * 1024;
    io_service_config_.io_worker = 1;
    io_service_config_.service_queue_size = 128 * 1024;
    io_service_config_.service_worker = 1;
    io_service_config_.stack_size = 2048;
  }

  io_service_config_.service_handler_prototype = CreateServiceHandler();

  return 0;
}

int Application::StartIOService() {
  io_service_.HandleSignal();

  if (0 != io_service_.Init(io_service_config_)) {
    MI_LOG_ERROR(logger, "Application::StartIOService Init fail");
    return -1;
  }

  if (0 != io_service_.Start()) {
    MI_LOG_ERROR(logger, "Application::StartIOService Start fail");
    return -1;
  }

  return 0;
}

int Application::StartClientManager() {
  if (!config_manager_) {
    MI_LOG_DEBUG(logger, "Application::StartClientManager no config set, ignore");
    return 0;
  }

  ClientManager* client_manager = new ClientManager(GetIOService());
  for (const auto& router_config : config_manager_->GetClientRoutersConfig()) {
    for (const auto& client_config : router_config.clients) {
      for (int i = 0; i != client_config.count; ++i) {
        if (0 != client_manager->AddClient(router_config.name, client_config.address, client_config.timeout)) {
          LOG_ERROR(logger, "Application::StartClientManager AddClient fail"
              << ", name:" << router_config.name
              << ", address:" << client_config.address
              << ", timeout:" << client_config.timeout);
          delete client_manager;
          return -1;
        }
      }
    }
  }

  if (0 != client_manager->Start()) {
    LOG_ERROR(logger, "Application::StartClientManager Start fail");
    delete client_manager;
    return -1;
  }

  client_manager_ = client_manager;
  return 0;
}

int Application::StartServiceManager() {
  if (!config_manager_) {
    MI_LOG_DEBUG(logger, "Application::StartServiceManager no config set, ignore");
    return 0;
  }

  for (const auto& service_config : config_manager_->GetServicesConfig()) {
    Acceptor* acceptor = IODescriptorFactory::Instance()
      .CreateAcceptor(GetIOService(), service_config.address, service_config.name);
    if (!acceptor) {
      MI_LOG_ERROR(logger, "Application::StartServiceManager CreateAcceptor fail"
          << ", address:" << service_config.address
          << ", name:" << service_config.name);
      return -1;
    }

    if (0 != acceptor->Start()) {
      MI_LOG_ERROR(logger, "Application::StartServiceManager Start Acceptor fail"
          << ", address:" << service_config.address
          << ", name:" << service_config.name);
      IODescriptorFactory::Instance().Destroy(acceptor);
      return -1;
    } else {
      MI_LOG_INFO(logger, "Application::StartServiceManager"
          << ", address:" << service_config.address
          << ", service:" << service_config.name);
    }

    acceptor_.push_back(acceptor);
  }
  return 0;
}

int Application::RunIOService() {
  io_service_.Run();
  return 0;
}

int Application::StopServiceManager() {
  for (Acceptor* acceptor : acceptor_) {
    IODescriptorFactory::Instance().Destroy(acceptor);
  }
  return 0;
}

int Application::StopClientManager() {
  if (client_manager_) {
    return client_manager_->Stop();
  }
  return 0;
}

int Application::StopIOService() {
  io_service_.CleanUp();
  return 0;
}

} //namespace minotaur

