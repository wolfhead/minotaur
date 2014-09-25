#ifndef _MINOTAUR_NET_IO_BUFFER_H_
#define _MINOTAUR_NET_IO_BUFFER_H_
/**
 * @file io_buffer.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <algorithm>

namespace minotaur {

class IOBuffer {
 public:
  IOBuffer();
  ~IOBuffer();
  
  void Write(const char* buffer, uint32_t size);
  char* EnsureWrite(uint32_t size);
  void Produce(uint32_t size) {write_offset_ += size;}

  char* GetRead() const {return buffer_ + read_offset_;}
  void Consume(uint32_t size) {read_offset_ += std::min(size, GetReadSize());}
  uint32_t GetReadSize() const {return write_offset_ - read_offset_;}

 private:
  char* buffer_;
  uint32_t buffer_size_;
  uint32_t write_offset_;
  uint32_t read_offset_;
};

} //namespace minotaur

#endif // _MINOTAUR_NET_IO_BUFFER_H_
