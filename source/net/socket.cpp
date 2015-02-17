/**
 * @file socket.cpp
 * @author Wolfhead
 */
#include "socket.h"
#include <sys/socket.h>
#include "../event/event_loop_data.h"
#include "../event/event_loop_stage.h"
#include "../common/system_error.h"
#include "../io_service.h"

namespace minotaur {

LOGGER_CLASS_IMPL_NAME(logger, Socket, "net.Socket");

Socket::Socket(IOService* io_service) 
    : IODescriptor(io_service, -1, -1) {
}

Socket::Socket(IOService* io_service, int fd) 
    : IODescriptor(io_service, fd, fd) {
}

void Socket::Dump(std::ostream& os) const {
  os << "[Socket fd:" << in_ << "]";
}

} //namespace minotaur
