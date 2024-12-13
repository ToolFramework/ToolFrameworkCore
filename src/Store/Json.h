#ifndef TOOLFRAMEWORK_JSON_H
#define TOOLFRAMEWORK_JSON_H

#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace ToolFramework {

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
json_encode(std::ostream& output, T datum) {
  output << datum;
  return true;
}

bool json_encode(std::ostream& output, const std::string& datum);

template <typename T>
bool json_encode(std::ostream& output, const std::vector<T>& data) {
  output << '[';
  bool comma = false;
  for (const T& datum : data) {
    if (comma)
      output << ',';
    else
      comma = true;
    json_encode(output, datum);
  };
  output << ']';
  return true;
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
    json_encode(output, datum.first);
    output << ':';
    json_encode(output, datum.second);
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
