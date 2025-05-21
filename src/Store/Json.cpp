#include <Json.h>

namespace ToolFramework {

bool json_encode(std::ostream& output, const char* datum) {
  output << '"';
  while (*datum) {
    if (*datum == '"' || *datum == '\\') output << '\\';
    output << *datum;
  };
  output << '"';
  return true;
}

bool json_encode(std::ostream& output, const std::string& datum) {
  return json_encode(output, datum.c_str());
}

}
