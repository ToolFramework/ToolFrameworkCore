#include <Json.h>

namespace ToolFramework {

bool json_encode(std::ostream& output, const std::string& datum) {
  output << '"';
  for (char c : datum) {
    if (c == '"' || c == '\\') output << '\\';
    output << c;
  };
  output << '"';
  return true;
}

}
