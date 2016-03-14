/**
 * @file redis_parser.h
 * @author Wolfhead
 */
#include "redis_parser.h"

namespace ade { namespace redis {

const std::string RedisParser::k_command_sep = "\r\n";
LOGGER_CLASS_IMPL(logger, RedisParser);

std::ostream& operator << (std::ostream& os, const RESP& resp) {
  resp.Dump(os);
  return os;
}

} //namespace redis 
} //namespace rade
