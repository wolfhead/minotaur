#ifndef _MINOTAUR_NET_IO_BUFFER_H_
#define _MINOTAUR_NET_IO_BUFFER_H_
/**
 * @file io_buffer.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>

namespace ade {

class IOBuffer {
 public:
  IOBuffer();
  ~IOBuffer();
  
  char* EnsureWrite(uint32_t size);

  inline void Write(const char* buffer, uint32_t size) {
    memcpy(EnsureWrite(size), buffer, size);
    Produce(size); 
  }

  inline void WriteString(const std::string& data) {
    Write(data.data(), data.size());
  }

  inline char* GetWrite() {
    return buffer_ + write_offset_;
  }

  inline void Produce(uint32_t size) {
    write_offset_ += size;
  }

  inline char* GetRead() const {
    return buffer_ + read_offset_;
  }

  inline void Consume(uint32_t size) {
    read_offset_ += std::min(size, GetReadSize());
    if (read_offset_ == write_offset_) {
      read_offset_ = write_offset_ = 0;
    }
  }

  inline uint32_t GetReadSize() const {
    return write_offset_ - read_offset_;
  }

  inline void EnsureCStyle() {
    *(GetRead() + GetReadSize()) = 0;
  }

  inline char* GetCStyle() {
    EnsureCStyle();
    return GetRead();
  }

  inline void Reset() {
    write_offset_ = read_offset_ = 0;
  }

  void Dump(std::ostream& os) const;

 private:
  char* buffer_;
  uint64_t buffer_size_;
  uint32_t write_offset_;
  uint32_t read_offset_;
};

std::ostream& operator << (std::ostream& os, const IOBuffer& buffer);

} //namespace ade

#endif // _MINOTAUR_NET_IO_BUFFER_H_
