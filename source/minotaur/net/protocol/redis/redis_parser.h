#ifndef MINOTAUR_PROTOCOL_REDIS_PARSER_H
#define MINOTAUR_PROTOCOL_REDIS_PARSER_H
/**
 * @file redis_parser.h
 * @author Wolfhead
 */
#include <stdint.h>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include "../../../common/logger.h"
#include "../../../common/slice.h"

namespace ade { namespace redis {

class RESP {
 public:
  typedef int64_t IntType;
  typedef std::string StringType;
  typedef std::vector<RESP> ArrayType;

  enum {
    kNull = 0,
    kString,
    kBulkString,
    kError,
    kInteger,
    kArray,
  };

  RESP() : type_(kNull) {}
  RESP(IntType data) {SetInteger(data);}
  RESP(uint8_t type, const std::string& data) {
    type_ = type;
    PlacementNewString(data);
  }
  RESP(const RESP& resp) {
    memcpy(this, &resp, sizeof(RESP));
    // there will be only one resp holding ref
    resp.type_ = kNull;
  }

  ~RESP() {
    Reset();
  }

  inline void SetInteger(int64_t data) {
    type_ = kInteger;
    *IntBuffer() = data;
  }

  inline void SetString(const std::string& data) {
    type_ = kString;
    PlacementNewString(data);
  }

  inline void SetBulkString(const std::string& data) {
    type_ = kBulkString;
    PlacementNewString(data);
  }

  inline void SetError(const std::string& data) {
    type_ = kError;
    PlacementNewString(data);
  }

  inline void SetArray(int32_t length) {
    type_ = kArray;
    PlacementNewArray();
    ArrayBuffer()->resize(length);
  }

  inline void AppendInteger(int64_t data) {
    if (type_ == kNull) {
      type_ = kArray;
      PlacementNewArray();
    }

    ArrayBuffer()->push_back(RESP(data));
  }

  inline void AppendString(uint8_t type, const std::string& data) {
    if (type_ == kNull) {
      type_ = kArray;
      PlacementNewArray();
    }

    ArrayBuffer()->push_back(RESP(type, data));
  }

  inline RESP& AddRESP() {
    if (type_ == kNull) {
      type_ = kArray;
      PlacementNewArray();
    }
    ArrayBuffer()->resize(ArrayBuffer()->size() + 1);
    return ArrayBuffer()->at(ArrayBuffer()->size() - 1);    
  }

  inline bool IsInteger() const {return type_ == kInteger;}
  inline bool IsString() const {return type_ == kString || type_ == kBulkString;}
  inline bool IsError() const {return type_ == kError;}
  inline bool IsArray() const {return type_ == kArray;}
  inline bool IsNull() const {return type_ == kNull;}

  IntType GetInteger() const {return *IntBuffer();}
  const StringType& GetString() const {return *StringBuffer();}
  const StringType& GetError() const {return *StringBuffer();}

  uint32_t Size() const {return ArrayBuffer()->size();}
  const RESP& At(uint32_t index) const {return ArrayBuffer()->at(index);}
  RESP& At(uint32_t index) {return ArrayBuffer()->at(index);}

  void Reset() {
    switch (type_) {
      case kString:
      case kBulkString:
      case kError:
        PlacementDeleteString();
        break;
      case kArray:
        PlacementDeleteArray();
        break;
      default:
        break;
    }
    type_ = kNull;
  }

  void Dump(std::ostream& os, uint8_t level = 0) const {
    std::string level_str(level * 2, ' ');
    if (type_ == kNull) {
      os << level_str << " -- Null" << std::endl;
    } else if (type_ == kString) {
      os << level_str << "+ -- " << *StringBuffer() << std::endl;
    } else if (type_ == kBulkString) {
      os << level_str << "$" << StringBuffer()->size() << " -- " << *StringBuffer() << std::endl;
    } else if (type_ == kError) {
      os << level_str << "- -- " << *StringBuffer() << std::endl;
    } else if (type_ == kInteger) {
      os << level_str << ": -- " << *IntBuffer() << std::endl;
    } else if (type_ == kArray) {
      os << level_str << "*" << ArrayBuffer()->size() << std::endl;
      for (const auto& resp : *ArrayBuffer()) {
        resp.Dump(os, level + 1);
      }
    }
  }

 private:
  RESP& operator = (const RESP&);

  inline IntType* IntBuffer() const {return (IntType*)(&buffer_.int_buffer);}
  inline StringType* StringBuffer() const {return (StringType*)(&buffer_.str_buffer);}
  inline ArrayType* ArrayBuffer() const {return (ArrayType*)(&buffer_.array_buffer);}

  inline void PlacementNewString(const std::string& data) {
    new (StringBuffer()) StringType(data);
  }

  inline void PlacementDeleteString() {
    StringBuffer()->~StringType();
  }

  inline void PlacementNewArray() {
    new (ArrayBuffer()) ArrayType();
  }

  inline void PlacementDeleteArray() {
    ArrayBuffer()->~ArrayType();
  }

  mutable uint8_t type_;
  union {
    char int_buffer[sizeof(IntType)];
    char str_buffer[sizeof(StringType)];
    char array_buffer[sizeof(ArrayType)];
  } buffer_;
};

class RedisParser {
 public:
  static const int64_t kMaxBulkLength = 16 * 1024;
  static const int64_t kMaxArrayLength = 4 * 1024;
  static const std::string k_command_sep;

  inline static void EncodeRaw(const std::string& data, std::string* buffer) {
    buffer->append(data);
  }

  inline static void EncodeString(const std::string& data, std::string* buffer) {
    buffer->reserve(buffer->size() + data.size() + 3);
    buffer->append(1, '+').append(data).append(k_command_sep);
  }

  inline static void EncodeError(const std::string& data, std::string* buffer) {
    buffer->reserve(buffer->size() + data.size() + 3);
    buffer->append(1, '-').append(data).append(k_command_sep);
  }

  inline static void EncodeInteger(int64_t data, std::string* buffer) {
    buffer->reserve(buffer->size() + 20 + 3);
    buffer->append(1, ':').append(boost::lexical_cast<std::string>(data)).append(k_command_sep);
  }

  inline static void EncodeBulkString(const std::string& data, std::string* buffer) {
    buffer->reserve(buffer->size() + data.size() + 20);
    buffer
      ->append(1, '$')
      .append(boost::lexical_cast<std::string>(data.size())).append(k_command_sep)
      .append(data).append(k_command_sep);
  }

  inline static void EncodeArray(uint32_t size, std::string* buffer) {
    buffer->reserve(buffer->size() + 20 + 3);
    buffer->append(1, '*').append(boost::lexical_cast<std::string>(size)).append(k_command_sep);
  }

  /*
   * if command = "$6\r\nfoobar\r\n"
   * bulk_header = {"6",1}
   * bulk_buffer = {"forrbar\r\n", 8}
   * @return > 0 total consumed buffer, including bulk_header
   *         < 0 fail
   *         = 0 need more data 
   */
  static int ParseBulkString(
      const Slice& bulk_header,
      const Slice& bulk_buffer,
      RESP* resp) {
    int64_t length = 0;
    if (0 != SliceToInteger(bulk_header, &length)) {
      return -1;
    }    

    if (length == -1) {
      return bulk_header.size() + 3;
    }

    Slice bulk_command;
    int ret = GetBulkString(bulk_buffer, length, &bulk_command);
    if (ret < 0) {
      return -1;
    } else if (ret == 0) {
      return 0;
    }
    resp->SetBulkString(bulk_command.str());
    return bulk_header.size() + 3 + ret; 
  }

  static int ParseArray(
      const Slice& array_header,
      const Slice& array_buffer,
      RESP* resp) {
    int64_t length = 0;
    if (0 != SliceToInteger(array_header, &length)) {
      return -1;
    }

    if (length >= kMaxArrayLength) {
      LOG_ERROR(logger, "RedisParser::ParseArray length to large:" << length);
      return -1;
    }

    Slice current(array_buffer);
    int consume = array_header.size() + 3;
    resp->SetArray(length);

    for (int index = 0; index != length; ++index) {
      int ret = Parse(current, &resp->At(index));
      if (ret < 0) {
        return -1;
      } else if (ret == 0) {
        return 0;
      } else {
        consume += ret;
      }
      current.trim(ret);
    }
    return consume;
  }

#define HANDLE_RET(ret, consume) \
  if (ret < 0) {return -1;} \
  else if (ret == 0) {return 0;} \
  else {consume = ret;} \

  static int Parse(const Slice& buffer, RESP* resp) {
    Slice current(buffer);
    int ret = 0;
    int consume = 0;

    Slice command;
    uint8_t type = 0;

    if (current.empty()) {
      return 0;
    }

    ret = GetCommand(current, &type, &command);
    HANDLE_RET(ret, consume);

    switch (type) {
      case RESP::kString : resp->SetString(command.str()); break;
      case RESP::kError : resp->SetError(command.str()); break;
      case RESP::kInteger : { 
          int64_t value = 0;
          if (0 != SliceToInteger(command, &value)) {
            return -1;
          }
          resp->SetInteger(value);
          break;
      }
      case RESP::kBulkString : {
          ret = ParseBulkString(command, current.trim_copy(consume), resp);
          HANDLE_RET(ret, consume);
          break;
      }
      case RESP::kArray : {
          ret = ParseArray(command, current.trim_copy(consume), resp);
          HANDLE_RET(ret, consume);
          break;
      }
    }

    return consume;
  }

#undef HANDLE_RET

  inline static int SliceToInteger(const Slice& s, int64_t* v) {
    char* end;
    *v = strtoll(s.data(), &end, 10);
    if (end != s.data() + s.size()) {
      LOG_ERROR(logger, "RedisParser SliceToInteger fail:" << s);
      return -1;
    }
    return 0;
  }

  /**
   *  @return > 0 command extracted, return value is bytes consumed
   *          = 0 no command extracted, bufer not long enough
   *          < 0 parse error
   */
  inline static int GetCommand(
      const Slice& data, // should ensure this data is c-style string
      uint8_t* type, 
      Slice* command) {
    const char* current = data.data();
    const char* begin = data.data();
    switch (*current) {
      case '+': *type = RESP::kString; break;
      case '-': *type = RESP::kError; break;
      case ':': *type = RESP::kInteger; break;
      case '$': *type = RESP::kBulkString; break;
      case '*': *type = RESP::kArray; break;
      default:
        LOG_ERROR(logger, "RedisParser::GetCommand fail:" << data);
        *type = RESP::kNull; 
        return -1;
    }

    for (; *current != '\n' && *current != '\0'; ++current);

    if (*current) {
      command->set(begin + 1, current - begin - 2 /* for '\n' and leading char */);
      return current - begin + 1;
    } else {
      return 0;
    }
  }

  inline static int GetBulkString(
      const Slice& data,
      uint64_t length,
      Slice* bulk_string) {
    if (length >= kMaxBulkLength) {
      MI_LOG_ERROR(logger, "RedisParser GetBulkString length to large:" << length);
      return -1;
    }

    if (length + k_command_sep.size() > data.size()) {
      return 0;
    }

    bulk_string->set(data.data(), length);
    return length + k_command_sep.size();
  }

 private:
  LOGGER_CLASS_DECL(logger);
};

std::ostream& operator << (std::ostream& os, const RESP& resp);

} //namespace redis
} //namespace ade

#endif //MINOTAUR_PROTOCOL_REDIS_PARSER_H
