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
#ifdef MINOTAUR_MEM_CHECK
  Dump(std::cout);
#endif
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
  size += 1; // we need the extra byte to make a safe c-style string
  uint32_t remain_bytes = buffer_size_ - write_offset_ ;
  if (remain_bytes >= size) {
    return buffer_ + write_offset_;
  }

  uint32_t data_bytes = write_offset_ - read_offset_;
  if (read_offset_ >= size) {
    if (data_bytes == 0) {
      read_offset_ = write_offset_ = 0;
      return buffer_ + write_offset_;
    } else if (read_offset_ >= 1024 * 1024) {
      memmove(buffer_, buffer_ + read_offset_, data_bytes);
      write_offset_= data_bytes;
      read_offset_ = 0;
      return buffer_ + write_offset_;
    }
  }

  while (buffer_size_ < (write_offset_ + size)) {
    buffer_size_ = buffer_size_ ? buffer_size_ * 2 : 1024;
  }

  if (buffer_size_ > 0xFFFFFFFF) {
    buffer_size_ /= 2;
    return NULL;
  }

  buffer_ = (char*)realloc(buffer_, buffer_size_);
  return buffer_ + write_offset_;
}


void IOBuffer::Dump(std::ostream& os) const {
  os << "IOBuffer buffer_size:" << buffer_size_
     << ", write_offset_:" << write_offset_
     << ", read_offset_:" << read_offset_
     << ", size:" << GetReadSize()
     << std::endl;
}

std::ostream& operator << (std::ostream& os, const IOBuffer& buffer) {
  buffer.Dump(os);
  return os;
}



} //namespace minotaur
