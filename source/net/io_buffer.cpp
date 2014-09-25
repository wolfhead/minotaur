/**
 * @file io_buffer.cpp
 * @author Wolfhead
 */
#include "io_buffer.h"
#include <stdlib.h>
#include <string.h>

namespace minotaur {

IOBuffer::IOBuffer() 
    : buffer_(NULL)
    , buffer_size_(0)
    , write_offset_(0)
    , read_offset_(0) {
 
}

IOBuffer::~IOBuffer() {
  if (buffer_) {
    free(buffer_);
    buffer_ = NULL;
  }
}

void IOBuffer::Write(const char* buffer, uint32_t size) {
  memcpy(EnsureWrite(size), buffer, size);
  Produce(size);
}

char* IOBuffer::EnsureWrite(uint32_t size) {
  uint32_t remain_bytes = buffer_size_ - write_offset_ ;
  if (remain_bytes >= size) {
    return buffer_ + write_offset_;
  }

  uint32_t data_bytes = write_offset_ - read_offset_;
  if (read_offset_ >= size) {
    if (data_bytes == 0) {
      read_offset_ = write_offset_ = 0;
      return buffer_ + write_offset_;
    } else {
      memmove(buffer_, buffer_ + read_offset_, data_bytes);
      write_offset_= read_offset_;
      read_offset_ = 0;
      return buffer_ + write_offset_;
    }
  }

  while (buffer_size_ < (write_offset_ + size)) {
    buffer_size_ = buffer_size_ ? buffer_size_ * 2 : 1024;
  }

  buffer_ = (char*)realloc(buffer_, buffer_size_);
  return buffer_ + write_offset_;
}



} //namespace minotaur
