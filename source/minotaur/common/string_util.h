#ifndef MINOTAUR_COMMON_STRING_UTIL_H
#define MINOTAUR_COMMON_STRING_UTIL_H
/**
 * @file string_util.h
 * @author Wolfhead
 */
#include <string>

namespace ade {

class strutil {
 public:
  static inline char* GetMutableBuffer(std::string &str) {
    std::string::iterator it = str.begin();
    return str.empty() ? NULL : &(*it);
  }

  static std::string ToHex(const void* buf, size_t len, bool upper = false);
  inline static std::string ToHex(const std::string &str, bool upper = false) {
    return ToHex(str.data(), str.size(), upper);
  }

  /// URL-decode/encode, see RFC 1866
  static std::string UrlDecode(const char *url, size_t len);
  static std::string UrlEncode(const char *url, size_t len);

  inline static std::string UrlDecode(const std::string& str) {
    return UrlDecode(str.data(), str.size());
  }
  inline static std::string UrlEncode(const std::string& str) {
    return UrlEncode(str.data(), str.size());
  }

  /// base64 algorithm
  static std::string Base64Decode(const void *src, size_t len);
  static std::string Base64Encode(const void *src, size_t len);

  inline static std::string Base64Decode(const std::string& src) {
    return Base64Decode(src.data(), src.size()); 
  }
  inline static std::string Base64Encode(const std::string& src) {
    return Base64Encode(src.data(), src.size()); 
  }

  /// decode/encode to 64 bits string (url-safe, make RFC-1866 happy)
  static std::string UrlSafe64Decode(const void *src, size_t len);
  static std::string UrlSafe64Encode(const void *src, size_t len);

  inline static std::string UrlSafe64Decode(const std::string& src) {
    return UrlSafe64Decode(src.data(), src.size());
  }
  inline static std::string UrlSafe64Encode(const std::string& src) {
    return UrlSafe64Encode(src.data(), src.size());
  }

  /// randomize a string with letters and digitals
  static std::string RandomString(size_t n);

  /// MD5
  static std::string Md5(const void* buf, size_t size);
  inline static std::string Md5(const std::string& input) {
    return Md5(input.data(), input.size());
  }

  /// CRC32
  static uint32_t CRC32(const void* buf, size_t size);
  inline static uint32_t CRC32(const std::string& input) {
    return CRC32(input.data(), input.size());
  }

  static inline bool StartsWith(const std::string &str, const std::string &sub) {
    return (str.find(sub) == 0);
  }

  static inline bool EndsWith(const std::string &str, const std::string &sub) {
    size_t i = str.rfind(sub);
    return (i != std::string::npos) && (i == (str.length() - sub.length()));
  }
};

} //namespace ade

#endif //MINOTAUR_COMMON_STRING_UTIL_H
