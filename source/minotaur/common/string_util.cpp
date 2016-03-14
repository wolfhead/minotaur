/**
 * @file string_util.cpp
 * @author Wolfhead
 */
#include "string_util.h" 
#include <string.h>
#include <stdlib.h>
#include <cctype>
#include "../3rd-party/md5/md5.h"
#include "../3rd-party/urlsafe64/urlsafe64.h"
#include "../3rd-party/crc/crc.h"

namespace ade {

namespace {
// URL-decode input buffer into destination buffer.
// http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt
static int url_decode(
    const char *src, int src_len, 
    char *dst, int dst_len) {
  int i, j, a, b;
  if (src_len <= 0) src_len = strlen(src);

#define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')

  for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++) {
    if (src[i] == '%' &&
        std::isxdigit(* (unsigned char *) (src + i + 1)) &&
        std::isxdigit(* (unsigned char *) (src + i + 2))) {
      a = std::tolower(* (unsigned char *) (src + i + 1));
      b = std::tolower(* (unsigned char *) (src + i + 2));
      dst[j] = (char) ((HEXTOI(a) << 4) | HEXTOI(b));
      i += 2;
    } else if (src[i] == '+') {
      dst[j] = ' ';
    } else {
      dst[j] = src[i];
    }
  }

#undef HEXTOI
  dst[j] = '\0'; /* Null-terminate the destination */
  return j;
}

// Refer to rfc3986 (2005): http://en.wikipedia.org/wiki/Percent-encoding
static int url_encode(const char *src, char *dst, int dst_len) {
  static const char *dont_escape = "-_.~";
  static const char *hex = "0123456789abcdef";
  const char *begin = dst;
  const char *end = dst + dst_len - 1;

  for (; *src != '\0' && dst < end; src++, dst++) {
    if (isalnum(*(unsigned char *) src) ||
        strchr(dont_escape, * (unsigned char *) src) != NULL) {
      *dst = *src;
    }
    else if (dst + 2 < end) {
      dst[0] = '%';
      dst[1] = hex[(* (unsigned char *) src) >> 4];
      dst[2] = hex[(* (unsigned char *) src) & 0xf];
      dst += 2;
    }
  }
  *dst = '\0';
  return (dst - begin);
}

// in must be at least len bytes
// len must be 1, 2, or 3
// buf must be a buffer of at least 4 bytes
static void base64_encode(
    const uint8_t *in, uint32_t len, uint8_t *buf) {
  static const uint8_t *kEncodeTable = (const uint8_t *)
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef"
    "ghijklmnopqrstuvwxyz0123456789+/";
  buf[0] = kEncodeTable[(in[0] >> 2) & 0x3F];
  if (len == 3) {
    buf[1] = kEncodeTable[((in[0] << 4) + (in[1] >> 4)) & 0x3f];
    buf[2] = kEncodeTable[((in[1] << 2) + (in[2] >> 6)) & 0x3f];
    buf[3] = kEncodeTable[in[2] & 0x3f];
  } else if (len == 2) {
    buf[1] = kEncodeTable[((in[0] << 4) + (in[1] >> 4)) & 0x3f];
    buf[2] = kEncodeTable[(in[1] << 2) & 0x3f];
  } else  { // len == 1
    buf[1] = kEncodeTable[(in[0] << 4) & 0x3f];
  }
}

// buf must be a buffer of at least 4 bytes
// buf will be changed to contain output bytes
// len is number of bytes to consume from input (must be 2, 3, or 4)
static int base64_decode(const uint8_t *buf, uint32_t len, uint8_t* dst) {
  static const uint8_t kDecodeTable[256] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
    0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,
    0x3c,0x3d,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
    0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
    0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
    0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
    0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,
    0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,
    0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
  };

  int ret = 0;
  dst[ret++] = (kDecodeTable[buf[0]] << 2)
      | (kDecodeTable[buf[1]] >> 4);
  if (len > 2 && buf[2] != '=') {
    dst[ret++] = ((kDecodeTable[buf[1]] << 4) & 0xf0)
      | (kDecodeTable[buf[2]] >> 2);
    if (len > 3 && buf[3] != '=') {
      dst[ret++] = ((kDecodeTable[buf[2]] << 6) & 0xc0)
        | (kDecodeTable[buf[3]]);
    }
  }
  return ret;
}

static void base64_encode(
    const void* src_buf,
    int src_len, 
    uint8_t* dst, 
    int dst_len) {
  const uint8_t* src = (const uint8_t*)src_buf;
  while (src_len >= 3) {
    base64_encode(src, 3, dst);
    src += 3;
    src_len -= 3;
    dst += 4;
    dst_len -= 4;
  }
  if (src_len > 0) {
    base64_encode(src, src_len, dst);
    if (src_len == 1) {
      dst += 2;
      *dst++ = '=';
      *dst++ = '=';
    } else {
      dst += 3;
      *dst++ = '=';
    }
  }
}

/**
 * @return used bytes in dst
 */
static int base64_decode(
    const void* src_buf, 
    int src_len, 
    uint8_t* dst, 
    int dst_len) {
  const uint8_t* src = (const uint8_t*)src_buf;
  int ret = 0;

  while (src_len >= 4) {
    ret += base64_decode(src, 4, dst);
    src += 4;
    src_len -= 4;
    dst += 3;
    dst_len -= 3;
  }
  if (src_len > 0) {
    ret += base64_decode(src, src_len, dst);
  }
  return ret;
}

size_t base64_encode_buf_size(size_t src_size) {
  return (src_size + 2) / 3 * 4;
}

size_t base64_decode_buf_size(size_t src_size) {
  return (src_size + 3) / 4 * 3;
}

} //namespace

std::string strutil::ToHex(const void* buf, size_t size, bool upper /*=false*/) {
  static const char hex_lower[16] = {
    '0','1','2','3','4','5','6','7',
    '8','9','a','b','c','d','e','f'};
  static const char hex_upper[16] = {
    '0','1','2','3','4','5','6','7',
    '8','9','A','B','C','D','E','F'};

  const char* hex = upper ? hex_upper : hex_lower;

  std::string result;
  result.resize(size * 2);
  char *str = GetMutableBuffer(result);
  const uint8_t* input_data = (const uint8_t*)buf;

  for (size_t i = 0, j = 0; i < size; i++) {
    uint8_t x = input_data[i];
    str[j++] = hex[x>>4];
    str[j++] = hex[x&15];
  }
  return result;
}

std::string strutil::UrlDecode(const char *url, size_t len) {
  size_t dst_len = len + 1;
  std::string result;
  result.resize(dst_len);
  char *dst = GetMutableBuffer(result);
  int j = url_decode(url, len, dst, dst_len);
  result.resize(j);
  return result;
}

std::string strutil::UrlEncode(const char *url, size_t len) {
  size_t dst_len = (3 * len) + 1;
  std::string result;
  result.resize(dst_len);
  char *dst = GetMutableBuffer(result);
  int j = url_encode(url, dst, dst_len);
  result.resize(j);
  return result;
}

std::string strutil::Md5(const void* buf, size_t size) {
  md5_state_t ms;
  md5_byte_t digest[16] = {0};

  md5_init(&ms);
  md5_append(&ms, (const md5_byte_t *)buf, size);
  md5_finish(&ms, digest);
  return std::string((char *)digest, sizeof(digest));
}

std::string strutil::Base64Encode(const void *src, size_t len) {
  std::string result;
  result.resize(base64_encode_buf_size(len));
  uint8_t* dst = (uint8_t*)GetMutableBuffer(result);
  base64_encode(src, len, dst, result.size());
  return result;
}

std::string strutil::Base64Decode(const void *src, size_t len) {
  std::string result;
  result.resize(base64_decode_buf_size(len));
  uint8_t* dst = (uint8_t*)GetMutableBuffer(result);
  int ret = base64_decode(src, len, dst, result.size());
  result.resize(ret);
  return result;
}

std::string strutil::UrlSafe64Decode(const void *str, size_t len) {
  std::string result;
  result.resize(len);
  uint8_t *dst = (uint8_t *) GetMutableBuffer(result);
  int j = urlsafe64_decode((const char*)str, len, dst);
  if (j < 0) return std::string();
  result.resize(j);
  return result;
}

std::string strutil::UrlSafe64Encode(const void *src, size_t len) {
  std::string result;
  result.resize(urlsafe64_get_encoded_size(len));

  char *dst = GetMutableBuffer(result);
  int j = urlsafe64_encode((const uint8_t *)src, len, dst);
  if (j < 0) return std::string();
  result.resize(j);
  return result;
}

std::string strutil::RandomString(size_t n) {
  std::string result;
  result.resize(n);
  char *str = GetMutableBuffer(result);

  static const char char_list[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678900";

  for (size_t i=0; i<n; i++) {
    str[i] = char_list[rand() & (64 - 1)];
  }
  return result;
}

uint32_t strutil::CRC32(const void* buf, size_t size) {
  return (crc32_ieee_le(0 ^ 0xFFFFFFFF, (const uint8_t*)buf, size) ^ 0xFFffFFff);
}

} //namespace ade
