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
};

template <typename T, size_t N>
bool json_encode(std::ostream& output, const std::array<T, N>& array) {
  return json_encode(output, array.data(), array.size());
};

template <typename T>
bool json_encode(std::ostream& output, const std::vector<T>& vector) {
  return json_encode(output, vector.data(), vector.size());
};

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

}

#endif
