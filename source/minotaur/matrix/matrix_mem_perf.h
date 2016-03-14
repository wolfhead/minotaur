#ifndef MINOTAUR_MATRIX_MEM_PERF_H
#define MINOTAUR_MATRIX_MEM_PERF_H
/**
 * @file matrix_mem_perf.h
 * @author Wolfhead
 */

#include <type_traits>
#include <iostream>
#include "matrix.h"

#define __MATRIX_TRACE(file, line) file":"#line
#define _MATRIX_TRACE(file, line) __MATRIX_TRACE(file, line)
#define MATRIX_TRACE() _MATRIX_TRACE(__FILE__, __LINE__)

#ifdef MATRIX_MEM_PERF 
  #define ADE_NEW(T, ...) \
    ade::matrix::AllocPerf<T>(MATRIX_TRACE(), ## __VA_ARGS__)
  #define ADE_DELETE(P) \
    ade::matrix::DestroyPerf(P)
#else
  #define ADE_NEW(T, ...) \
    new T(__VA_ARGS__)
  #define ADE_DELETE(P) \
    delete P
#endif //MATRIX_MEM_PERF

namespace ade { namespace matrix {

namespace __detail__ {

template<bool pod>
struct alloc_triats {};

template<>
struct alloc_triats<true> {
  template<typename T>
  static void construct(T*) {}

  template<typename T, typename... Args>
  static void construct(T* buffer, const Args&... args) {
    new (buffer) T(args...);
  }

  template<typename T>
  static void destruct(T*) {}
};

template<>
struct alloc_triats<false> {
  template<typename T, typename... Args>
  static void construct(T* buffer, const Args&... args) {
    new (buffer) T(args...);
  }

  template<typename T>
  static void destruct(T* buffer) {
    buffer->~T();
  }
};

template<typename T>
struct construct_traits {
  static void construct(T* buffer) {
    alloc_triats<std::is_pod<T>::value>::construct(buffer);
  }

  template<typename... Args>
  static void construct(T* buffer, const Args&... args) {
    alloc_triats<std::is_pod<T>::value>::construct(buffer, args...);
  }

  static void destruct(T* buffer) {
    alloc_triats<std::is_pod<T>::value>::destruct(buffer);
  }
};

} //namespace __detail__


struct PerfHeader {
  const char* message;
  uint64_t mem_size;
  char data[0];
};

template <typename T, typename ...Args>
T* AllocPerf(const char* message, const Args&... args) {
  uint8_t* p = (uint8_t*)malloc(sizeof(PerfHeader) + sizeof(T));
  PerfHeader* header = (PerfHeader*)p;

  if (GlobalMatrix::Ready()) {
    header->message = message;
    header->mem_size = sizeof(T);
    GlobalMatrix::Instance().PersistentAdd(message, sizeof(T));
  }

  __detail__::construct_traits<T>::construct((T*)(&header->data), args...);
  return (T*)&header->data;
}

template <typename T>
void DestroyPerf(T* p) {
  PerfHeader* header = (PerfHeader*)((uint8_t*)p - sizeof(PerfHeader));
  if (header->message && GlobalMatrix::Ready()) {
    GlobalMatrix::Instance().PersistentSub(header->message, header->mem_size);
  }
  
  __detail__::construct_traits<T>::destruct(p);
  free((uint8_t*)p - sizeof(PerfHeader));
}

} //namespace matrix
} //namespace ade

#endif // MINOTAUR_MATRIX_MEM_PERF_H
