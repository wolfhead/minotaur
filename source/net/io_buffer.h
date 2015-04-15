#ifndef _MINOTAUR_NET_IO_BUFFER_H_
#define _MINOTAUR_NET_IO_BUFFER_H_
/**
 * @file io_buffer.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <iostream>
#include <algorithm>

namespace minotaur {

class IOBuffer {
 public:
  IOBuffer();
  ~IOBuffer();
  
  void Write(const char* buffer, uint32_t size);
  char* EnsureWrite(uint32_t size);
  char* GetWrite() {return buffer_ + write_offset_;}
  void Produce(uint32_t size) {write_offset_ += size;}

  char* GetRead() const {return buffer_ + read_offset_;}
  void Consume(uint32_t size) {read_offset_ += std::min(size, GetReadSize());}
  uint32_t GetReadSize() const {return write_offset_ - read_offset_;}
  void EnsureCStyle() {*(GetRead() + GetReadSize()) = 0;}

  void Reset() {write_offset_ = read_offset_ = 0;}

  void Dump(std::ostream& os) const;

 private:
  char* buffer_;
  uint64_t buffer_size_;
  uint32_t write_offset_;
  uint32_t read_offset_;
};

std::ostream& operator << (std::ostream& os, const IOBuffer& buffer);

} //namespace minotaur

#endif // _MINOTAUR_NET_IO_BUFFER_H_
