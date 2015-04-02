#ifndef _MINOTAUR_COMMON_XML_MACRO_H_
#define _MINOTAUR_COMMON_XML_MACRO_H_
/**
 * @file xml_macro.h
 * @author Wolfhead
 */

#define XML_LOAD_STRING(xml, name, value, exit)                       \
  do {                                                                \
    const char * __str__ = xml->Attribute(name);                      \
    if (!__str__) {                                                   \
      LOG_ERROR(logger, "can not get attribute: " << name);           \
      return exit;                                                    \
    } else {                                                          \
      value = __str__;                                                \
    }                                                                 \
  } while(0);                                                         \

#define XML_LOAD_STRING_DEFAULT(xml, name, value, default_value)      \
  do {                                                                \
    const char * __str__ = xml->Attribute(name);                      \
    if (!__str__) {                                                   \
      LOG_DEBUG(logger, "can not get attribute: " << name);           \
      value = default_value;                                          \
    } else {                                                          \
      value = __str__;                                                \
    }                                                                 \
  } while(0);      

#define XML_LOAD_INT(xml, name, value, exit)                          \
  do {                                                                \
    if (0 != xml->QueryIntAttribute(name, value)) {                   \
      LOG_ERROR(logger, "can not get attribute: " << name);           \
      return exit;                                                    \
    }                                                                 \
  } while (0);                                                        \

#define XML_LOAD_INT_DEFAULT(xml, name, value, default_value)         \
  do {                                                                \
    if (0 != xml->QueryIntAttribute(name, value)) {                   \
      LOG_DEBUG(logger, "can not get attribute: " << name);           \
      *value = default_value;                                         \
    }                                                                 \
  } while (0);                                                        \

#define XML_LOAD_UINT(xml, name, value, exit)                         \
  do {                                                                \
    if (0 != xml->QueryUnsignedAttribute(name, value)) {              \
      LOG_ERROR(logger, "can not get attribute: " << name);           \
      return exit;                                                    \
    }                                                                 \
  } while (0);                                                        \

#define XML_LOAD_UINT_DEFAULT(xml, name, value, default_value)        \
  do {                                                                \
    if (0 != xml->QueryUnsignedAttribute(name, value)) {              \
      LOG_DEBUG(logger, "can not get attribute: " << name);           \
      *value = default_value;                                         \
    }                                                                 \
  } while (0);                                                        \

#define XML_LOAD_BOOL(xml, name, value, exit)                         \
  do {                                                                \
    if (0 != xml->QueryBoolAttribute(name, value)) {                  \
      LOG_ERROR(logger, "can not get attribute: " << name);           \
      return exit;                                                    \
    }                                                                 \
  } while (0);                                                        \

#define XML_LOAD_DOUBLE(xml, name, value, exit)                       \
  do {                                                                \
    if (0 != xml->QueryDoubleAttribute(name, value)) {                \
      LOG_ERROR(logger, "can not get attribute: " << name);           \
      return exit;                                                    \
    }                                                                 \
  } while (0);                                                        \

#define XML_LOAD_FLOAT(xml, name, value, exit)                        \
  do {                                                                \
    if (0 != xml->QueryFloatAttribute(name, value)) {                 \
      LOG_ERROR(logger, "can not get attribute: " << name);           \
      return exit;                                                    \
    }                                                                 \
  } while (0);                                                        \

#endif //_MINOTAUR_COMMON_XML_MACRO_H_
