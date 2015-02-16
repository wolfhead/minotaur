/**
 * @file io_service.cpp
 * @author Wolfhead
 */
#include "io_service.h"
#include "net/channel.h"

namespace minotaur {

IOService::IOService() 
    : channel_pool_(new ChannelPool(65536)) {
}

IOService::~IOService() {
  delete channel_pool_;
}

Channel* IOService::CreateChannel(int fd) {
  uint64_t channel_id = 0;
  Channel* channel = channel_pool_->Alloc(this, fd, &channel_id);
  if (channel) channel->SetChannelId(channel_id);
  return channel;
}

Channel* IOService::GetChannel(uint64_t channel_id) {
  return channel_pool_->GetKey(channel_id);
}

bool IOService::DestoryChannel(uint64_t channel_id) {
  return channel_pool_->DestroyKey(channel_id);
}

} //namespace minotaur
