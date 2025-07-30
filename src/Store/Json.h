#ifndef TOOLFRAMEWORK_JSON_H
#define TOOLFRAMEWORK_JSON_H

#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace ToolFramework {

template <typename T, size_t N>
bool json_encode(std::ostream&, const std::array<T, N>&);

template <typename T>
bool json_encode(std::ostream&, const std::vector<T>&);

template <typename T>
bool json_encode(std::ostream&, const std::map<std::string, T>&);

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
json_encode(std::ostream& output, T datum) {
  output << datum;
  return true;
}

bool json_encode(std::ostream& output, const char* datum);
bool json_encode(std::ostream& output, const std::string& datum);

template <typename T>
bool json_encode(std::ostream& output, const T* data, size_t size) {
  output << '[';
  bool comma = false;
  for (size_t i = 0; i < size; ++i) {
    if (comma)
      output << ',';
    comma = true;
    if (!json_encode(output, data[i])) return false;
  };
  output << ']';
  return true;
}

template <typename T, size_t N>
bool json_encode(std::ostream& output, const std::array<T, N>& array) {
  return json_encode(output, array.data(), array.size());
}

template <typename T>
bool json_encode(std::ostream& output, const std::vector<T>& vector) {
  return json_encode(output, vector.data(), vector.size());
}

template <typename T>
bool json_encode(std::ostream& output, const std::map<std::string, T>& data) {
  output << '{';
  bool comma = false;
  for (auto& datum : data) {
    if (comma)
      output << ',';
    else
      comma = true;
    if (!json_encode(output, datum.first)) return false;
    output << ':';
    if (!json_encode(output, datum.second)) return false;
  };
  output << '}';
  return true;
}

template <typename T>
bool json_encode(std::string& output, T data) {
  std::stringstream ss;
  if (!json_encode(ss, data)) return false;
  output = ss.str();
  return true;
}

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
    if (!json_encode(output, slot)) return false;
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

} // json_internal


// A helper function to write fixed-size objects
// Example: call `json_encode_object(output, "x", 42, "a", false)`
// to produce `{"x":42,"a":false}`
template <typename... Args>
bool json_encode_object(std::ostream& output, Args... args) {
  output << '{';
  bool comma = false;
  if (!json_internal::json_encode_object_slots(output, comma, args...))
    return false;
  output << '}';
  return true;
}

} // ToolFramework

#endif
