#include <iomanip>

#include <cstdint>
#include <cstring>

#include <Json.h>

namespace ToolFramework {

static bool json_encode(
    std::ostream& output,
    const char* begin,
    const char* end
) {
  output << '"';
  for (auto c = begin; c < end; ++c)
    switch (*c) {
      case '\a':
        output << "\\a";
        break;
      case '\b':
        output << "\\b";
        break;
      case '\f':
        output << "\\f";
        break;
      case '\n':
        output << "\\n";
        break;
      case '\r':
        output << "\\r";
        break;
      case '\t':
        output << "\\t";
        break;
      case '\\':
      case '"':
        output << '\\' << *c;
        break;
      default:
        {
          uint8_t byte = static_cast<uint8_t>(*c);
          if (byte < 0x20 || byte > 0x7F) {
            uint16_t codepoint = byte;
            if (++c != end)
              codepoint = codepoint << 8 | static_cast<uint8_t>(*c);
            output
              << "\\u"
              << std::setw(4)
              << std::setfill('0')
              << std::hex
              << codepoint
              << std::dec;
          } else
            output << byte;
        };
    };
  output << '"';
  return true;
}

bool json_encode(
    std::ostream& output,
    std::string::const_iterator begin,
    std::string::const_iterator end
) {
  return json_encode(output, &*begin, &*end);
}

bool json_encode_r(std::ostream& output, const std::string& datum, adl_tag) {
  return json_encode(output, datum.begin(), datum.end());
}

bool json_encode_r(std::ostream& output, const char* datum, adl_tag) {
  return json_encode(output, datum, datum + strlen(datum));
}

const char* json_scan_whitespace(const char* input) {
  if (!input) return nullptr;
  while (isspace(*input)) ++input;
  return input;
}

const char* json_scan_token(const char* input, char token) {
  input = json_scan_whitespace(input);
  return input && *input == token ? input + 1 : nullptr;
}

const char* json_scan_token(const char* input, const char* token) {
  input = json_scan_whitespace(input);
  if (!input) return nullptr;
  while (*token) if (*input++ != *token++) return nullptr;
  return isalnum(*input) ? nullptr : input;
}

const char* json_scan_number(const char* input) {
  input = json_scan_whitespace(input);
  if (!input) return nullptr;

  if (*input == '-' || *input == '+') ++input;
  while (isdigit(*input)) ++input;
  if (*input == '.') {
    ++input;
    while (isdigit(*input)) ++input;
  };
  if (tolower(*input) == 'e') {
    ++input;
    if (*input == '-' || *input == '+') ++input;
    while (isdigit(*input)) ++input;
  };

  return isalnum(*input) ? nullptr : input;
}

const char* json_scan_string(const char* input) {
  input = json_scan_token(input, '"');
  if (!input) return nullptr;

  bool escaped = false;
  for (; *input; ++input)
    if (escaped)
      escaped = false;
    else
      switch (*input) {
        case '\\':
          escaped = true;
          break;
        case '"':
          return input + 1;
          break;
        default:
          break;
      };

  return nullptr;
}

static const char* json_scan_list(
    const char* input,
    char start,
    char end,
    const char* (*scan_item)(const char*)
) {
  bool comma = false;
  input = json_scan_token(input, start);
  while (input) {
    input = json_scan_whitespace(input);
    if (*input == end) return input + 1;

    if (comma) {
      if (*input++ != ',') return nullptr;
    } else
      comma = true;

    input = scan_item(input);
  };
  return nullptr;
}

const char* json_scan_object(const char* input) {
  return json_scan_list(
      input, '{', '}',
      [](const char* i) -> const char* {
        i = json_scan_string(i);
        i = json_scan_token(i, ':');
        return json_scan(i);
    }
  );
}

const char* json_scan_array(const char* input) {
  return json_scan_list(input, '[', ']', json_scan);
}

const char* json_scan(const char* input) {
  input = json_scan_whitespace(input);
  if (!input) return nullptr;
  switch (*input) {
    case 'f':
      return json_scan_token(input, "false");
    case 't':
      return json_scan_token(input, "true");
    case 'n':
      return json_scan_token(input, "null");
    case '-':
    case '+':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return json_scan_number(input);
    case '"':
      return json_scan_string(input);
    case '{':
      return json_scan_object(input);
    case '[':
      return json_scan_array(input);
    default:
      return nullptr;
  };
}

bool json_valid(const char* input) {
  input = json_scan(input);
  input = json_scan_whitespace(input);
  return input && !*input;
};

bool json_decode_r(const char*& input, bool& value, adl_tag) {
  const char* i = json_scan_whitespace(input);
  if (!i) return false;

  switch (*i) {
    case 'f':
      i = json_scan_token(i, "false");
      if (!i) return false;
      value = false;
      break;
    case 't':
      i = json_scan_token(i, "true");
      if (!i) return false;
      value = true;
      break;
    case '0':
      if (isalnum(*++i)) return false;
      value = false;
      break;
    case '1':
      if (isalnum(*++i)) return false;
      value = true;
      break;
    default:
      return false;
  };

  input = i;
  return true;
}

namespace json_internal {

bool json_decode_string(
    const char*& input,
    std::string& value,
    bool keep_quotes
) {
  const char* i = json_scan_token(input, '"');
  if (!i) return false;

  std::stringstream ss;
  if (keep_quotes) ss << '"';

  bool escaped = false;
  for (; *i; ++i) {
    if (escaped) {
      escaped = false;
      switch (*i) {
        case '\\':
          ss << '\\';
          break;
        case 'a':
          ss << '\a';
          break;
        case 'b':
          ss << '\b';
          break;
        case 'f':
          ss << '\f';
          break;
        case 'n':
          ss << '\n';
          break;
        case 'r':
          ss << '\r';
          break;
        case 't':
          ss << '\t';
          break;
        case 'u':
          {
            uint16_t codepoint = 0;
            for (int b = 0; b < 2; ++b) {
              for (int d = 0; d < 2; ++d) {
                char digit = *++i;
                if (digit >= '0' && digit <= '9')
                  digit -= '0';
                else if (digit >= 'a' && digit <= 'f')
                  digit = digit - 'a' + 10;
                else if (digit >= 'A' && digit <= 'F')
                  digit = digit - 'A' + 10;
                else
                  return false; // TODO: invalid string
                codepoint = codepoint << 4 | digit;
              };
            };
            if (codepoint > 0xFF) ss << static_cast<char>(codepoint >> 8);
            ss << static_cast<char>(codepoint & 0xFF);
          };
          break;
        default:
          // TODO: invalid escape sequence. Store anyway?
          ss << *i;
      };
    } else {
      switch (*i) {
        case '\\':
          escaped = true;
          continue;
        case '"':
          if (keep_quotes) ss << '"';
          input = ++i;
          value = ss.str();
          return true;
        default:
          ss << *i;
      };
    };
  };

  return false;
}

}

bool json_decode_r(const char*& input, std::string& value, adl_tag) {
  return json_internal::json_decode_string(input, value, false);
}

static bool json_decode_list(
    const char*& input,
    char start,
    char end,
    const std::function<bool (const char*&)> decode_item
) {
  const char* i = json_scan_token(input, start);
  if (!i) return false;

  bool comma = false;
  while (true) {
    i = json_scan_whitespace(i);
    if (*i == end) break;

    if (comma) {
      if (*i++ != ',') return false;
    } else
      comma = true;

    if (!decode_item(i)) return false;
  };

  input = i + 1;
  return true;
}

bool json_decode_array(
    const char*& input,
    const std::function<bool (const char*&)>& decode_item
) {
  return json_decode_list(input, '[', ']', decode_item);
}

bool json_decode_object(
    const char*& input,
    const std::function<bool (const char*&, std::string)>& decode_value
) {
  return json_decode_list(
      input, '{', '}',
      [&](const char*& input_) -> bool {
        const char* i = input_;
        std::string key;
        if (!json_decode_r(i, key, adl_tag {})) return false;
        i = json_scan_token(i, ':');
        if (!i) return false;
        if (!decode_value(i, std::move(key))) return false;
        input_ = i;
        return true;
      }
  );
}

}
