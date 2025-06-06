#ifndef TOOLFRAMEWORK_JSON_H
#define TOOLFRAMEWORK_JSON_H

#include <functional>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>


/**
   This file provides functions working with JSON.

   Three families of functions are provided: json_encode for JSON
   serialization, json_scan for JSON validation, and json_decode for JSON
   parsing.
*/

namespace ToolFramework {

/**
   JSON serialization functions

   json_encode comes in two flavours: json_encode(std::ostream&, const T&)
   serializes a value into a stream, json_encode(std::string&, const T&)
   serializes a value into a string. The latter calls the former, and the
   former calls json_encode_r to encode specific objects. Provide overloads of
   json_encode_r function or template to extend for custom classes.

   json_encode returns false if encoding has failed; for the function working
   with a stream it can only happen in user extensions, functions in this file
   always return true. json_encode(std::string&, const T&) checks the stream
   flags.
*/

template <typename T> bool json_encode(std::ostream& output, const T& datum);
template <typename T> bool json_encode(std::string& output, const T& datum);

// Encode a part of a string as a string
bool json_encode(
    std::ostream& output,
    std::string::const_iterator begin,
    std::string::const_iterator end
);

// A helper function to write fixed-size objects
// Example: call `json_encode_object(output, "x", 42, "a", false)`
// to produce `{"x":42,"a":false}`
template <typename... Fields>
bool json_encode_object(std::ostream& output, Fields... fields);

/*
   JSON decoder function
   Use this if you know the structure of JSON data in advance

   Returns false if the JSON string is invalid.
   If the object was decoded successfully, input is set one character past it,
   otherwise it is left unchanged.
*/
template <typename T> bool json_decode(const char*& input, T& value);

// Expects an array in input, calls decode_item(item) on each array item
bool json_decode_array(
    const char*& input,
    const std::function<bool (const char*& /* input */)>& decode_item
);

// Expects an object in input, calls decode_value(key, value) on each key-value pair
bool json_decode_object(
    const char*& input,
    const std::function<bool (const char*& /* input */, std::string /* key */)>&
      decode_value
);

// A class to trigger argument-dependent lookup (ADL) in json_encode_r and
// json_decode_r functions. It is required for the compiler to find overloads
// declared after this file. See
// https://akrzemi1.wordpress.com/2016/01/16/a-customizable-framework/
// for caveats.
struct adl_tag {};


/**
   JSON scanner functions

   Scanner functions find the end of an object without further interpretation.

   Each function returns nullptr if the JSON string is invalid.
   It is okay to pass nullptr as input.
*/
const char* json_scan_whitespace(const char* input);
const char* json_scan_token(const char* input, char token);
const char* json_scan_token(const char* input, const char* token);
const char* json_scan_number(const char* input);
const char* json_scan_string(const char* input);
const char* json_scan_object(const char* input);
const char* json_scan_array(const char* input);
const char* json_scan(const char* input); // generic

bool json_valid(const char* input);


// json_encode_r implements encoding of specific objects. Add overloads to this
// function to support custom classes.

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
json_encode_r(std::ostream& output, T datum, adl_tag);

bool json_encode_r(std::ostream& output, const std::string& datum, adl_tag);

bool json_encode_r(std::ostream& output, const char* datum, adl_tag);

template <typename T>
bool json_encode_r(std::ostream& output, const T* data, size_t size, adl_tag);

template <typename T, size_t N>
bool json_encode_r(std::ostream&, const std::array<T, N>&, adl_tag);

template <typename T>
bool json_encode_r(std::ostream&, const std::vector<T>&, adl_tag);

template <typename T>
bool json_encode_r(std::ostream&, const std::map<std::string, T>&, adl_tag);

// json_decode_r implements decoding of specific objects. Add overloads to this
// function to support custom classes.

bool json_decode_r(const char*& input, bool& value, adl_tag);

template <typename T>
typename std::enable_if<
  std::is_integral<T>::value && std::is_signed<T>::value,
  bool
>::type
json_decode_r(const char*& input, T& value, adl_tag);

template <typename T>
typename std::enable_if<
  std::is_integral<T>::value && !std::is_signed<T>::value,
  bool
>::type
json_decode_r(const char*& input, T& value);

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
json_decode_r(const char*& input, T& value, adl_tag);

bool json_decode_r(const char*& input, std::string& value, adl_tag);

template <typename T>
bool json_decode_r(const char*& input, std::vector<T>& value, adl_tag);

template <typename T>
bool json_decode_r(
    const char*& input,
    std::map<std::string, T>& value,
    adl_tag
);

template <typename T>
bool json_decode_r(
    const char*& input,
    std::unordered_map<std::string, T>& value,
    adl_tag
);


// Implementation

namespace json_internal {

  inline bool json_encode_object_slots(std::ostream& output, bool& comma) {
    return true;
  }

  template <typename Slot, typename... Rest>
  bool json_encode_object_slots(
      std::ostream& output,
      bool& comma,
      const char* name,
      const Slot& slot,
      Rest... rest
  ) {
    if (comma) output << ',';
    comma = true;
    output << '"' << name << '"' << ':';
    if (!json_encode_r(output, slot, adl_tag {})) return false;
    return json_encode_object_slots(output, comma, rest...);
  }

  template <typename Slot, typename... Rest>
  bool json_encode_object_slots(
      std::ostream& output,
      bool& comma,
      const std::string& name,
      const Slot& slot,
      Rest... rest
  ) {
    return json_encode_object_slots(output, comma, name.c_str(), slot, rest...);
  }

  bool json_decode_string(
      const char*& input,
      std::string& value,
      bool keep_quotes
  );

} // json_internal

template <typename T>
bool json_encode(std::ostream& output, const T& datum) {
  return json_encode_r(output, datum, adl_tag {});
}

template <typename T>
bool json_encode(std::string& output, const T& datum) {
  std::stringstream ss;
  if (!json_encode(ss, datum) || !ss) return false;
  output = ss.str();
  return true;
}

template <typename... Args>
bool json_encode_object(std::ostream& output, Args... args) {
  output << '{';
  bool comma = false;
  if (!json_internal::json_encode_object_slots(output, comma, args...))
    return false;
  output << '}';
  return true;
}

template <typename T>
bool json_decode(const char*& input, T& value) {
  return json_decode_r(input, value, adl_tag {});
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
json_encode_r(std::ostream& output, T datum, adl_tag) {
  output << datum;
  return true;
}

template <typename T>
bool json_encode_r(std::ostream& output, const T* data, size_t size, adl_tag tag) {
  output << '[';
  bool comma = false;
  for (size_t i = 0; i < size; ++i) {
    if (comma)
      output << ',';
    comma = true;
    if (!json_encode_r(output, data[i], tag)) return false;
  };
  output << ']';
  return true;
}

template <typename T, size_t N>
bool json_encode_r(
    std::ostream& output,
    const std::array<T, N>& array,
    adl_tag tag
) {
  return json_encode_r(output, array.data(), array.size(), tag);
}

template <typename T>
bool json_encode_r(
    std::ostream& output,
    const std::vector<T>& vector,
    adl_tag tag
) {
  return json_encode_r(output, vector.data(), vector.size(), tag);
}

template <typename Map, typename T>
bool json_encode_r_map(std::ostream& output, const Map& data, adl_tag tag) {
  output << '{';
  bool comma = false;
  for (auto& datum : data) {
    if (comma)
      output << ',';
    else
      comma = true;
    if (!json_encode_r(output, datum.first, tag)) return false;
    output << ':';
    if (!json_encode_r(output, datum.second, tag)) return false;
  };
  output << '}';
  return true;
}

template <typename T>
bool json_encode_r(
    std::ostream& output,
    const std::map<std::string, T>& data,
    adl_tag tag
) {
  return json_encode_r_map<std::map<std::string, T>, T>(output, data, tag);
}

template <typename T>
bool json_encode_r(
    std::ostream& output,
    const std::unordered_map<std::string, T>& data,
    adl_tag tag
) {
  return json_encode_r_map<std::unordered_map<std::string, T>, T>(
      output, data, tag
  );
}

template <typename T>
typename std::enable_if<
  std::is_integral<T>::value && std::is_signed<T>::value,
  bool
>::type
json_decode_r(const char*& input, T& value, adl_tag) {
  char* i;
  value = strtol(input, &i, 10);
  if (isalnum(*i)) return false;
  input = i;
  return true;
}

template <typename T>
typename std::enable_if<
  std::is_integral<T>::value && !std::is_signed<T>::value,
  bool
>::type
json_decode_r(const char*& input, T& value, adl_tag) {
  char* i;
  value = strtoul(input, &i, 10);
  if (isalnum(*i)) return false;
  input = i;
  return true;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
json_decode_r(const char*& input, T& value, adl_tag) {
  char* i;
  value = strtod(input, &i);
  if (isalnum(*i)) return false;
  input = i;
  return true;
}

template <typename T>
bool json_decode_r(const char*& input, std::vector<T>& value, adl_tag tag) {
  value.clear();
  return json_decode_array(
      input,
      [&](const char*& i) -> bool {
        T item;
        if (!json_decode_r(i, item, tag)) return false;
        value.push_back(std::move(item));
        return true;
      }
  );
}

template <typename Map, typename T>
bool json_decode_r_map(const char*& input, Map& value, adl_tag tag) {
  value.clear();
  return json_decode_object(
      input,
      [&](const char*& i, std::string key) -> bool {
        T item;
        if (!json_decode_r(i, item, tag)) return false;
        value[std::move(key)] = std::move(item);
        return true;
      }
  );
}

template <typename T>
bool json_decode_r(
    const char*& input,
    std::map<std::string, T>& value,
    adl_tag tag
) {
  return json_decode_r_map<std::map<std::string, T>, T>(input, value, tag);
}

template <typename T>
bool json_decode_r(
    const char*& input,
    std::unordered_map<std::string, T>& value,
    adl_tag tag
) {
  return json_decode_r_map<std::unordered_map<std::string, T>, T>(
      input, value, tag
  );
}

} // ToolFramework

#endif
