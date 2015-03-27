/**
 * @file coro_service.cpp
 * @author Wolfhead
 */
#include "coro_service.h"
#include "../service/service.h"
#include <iostream>

namespace minotaur {

void CoroService::Run() {
  service_->Serve(RecieveMail()); 
}

} //namespace minotaur
