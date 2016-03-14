/**
 * @file coro_service.cpp
 * @author Wolfhead
 */
#include "coro_service.h"
#include "../service/service.h"
#include "../common/logger.h"
#include "coro_all.h"
#include <iostream>

namespace ade {

void CoroService::Run() {
  service_->Serve(RecieveMail()); 
}

} //namespace ade
