/**
 * @file stdio_channel.cpp
 * @author Wolfhead
 */
#include "stdio_channel.h"
#include <stdio.h>
#include "socket_op.h"


namespace ade {

LOGGER_CLASS_IMPL(logger, StdioChannel);

StdioChannel::StdioChannel(IOService* io_service, Service* service) 
    : IODescriptor(io_service, STDIN_FILENO, STDOUT_FILENO, true) {
  
}

int StdioChannel::Start() {
  if (0 != SocketOperation::SetNonBlocking(GetIN())) {
    MI_LOG_ERROR(logger, "StdioChannel::Start SetNonBlocking, failed with:" 
        << SystemError::FormatMessage());
    return -1;
  }

  if (0 != RegisterRead()) {
    MI_LOG_ERROR(logger, "StdioChannel::Start RegisterRead failed");
    return -1;
  }

  return 0;
}

int StdioChannel::Stop() {
  assert("no implement");
  return -1;
}

void StdioChannel::Close() {
  //DoNothing
}

void StdioChannel::OnClose() {
  Destroy();
}

} //namesapce ade
