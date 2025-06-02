#include <iostream>
#include <functional>

#include <string.h>

#include <Store.h>

using ToolFramework::Store;

class Tester {
  public:
    bool success = true;

    void operator()(bool result) {
      success = success && result;
    };

    operator bool() { return success; };
};

template <>
struct std::equal_to<Store> {
  bool operator()(const Store& a, const Store& b) {
    auto ia = a.begin();
    auto ib = b.begin();
    while (ia != a.end()) if (ib == b.end() || *ia++ != *ib++) return false;
    return true;
  };
};

// Compare two JSON strings neglecting whitespace
// Skips whitespace and compares further characters. May produce false equals
// when comparing strings within strings differing in whitespace, e.g.,
// "{\"a\":\"q w\"}" "{\"a\":\"q  w\"}", but we don't expect those.
static bool json_eq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) {
      while (isspace(*a)) ++a;
      while (isspace(*b)) ++b;
      if (*a != *b) return false;
    };
    ++a;
    ++b;
  };
  while (isspace(*a)) ++a;
  while (isspace(*b)) ++b;
  return *a == *b;
};

template <typename T>
std::ostream& operator<<(std::ostream&, const std::map<std::string, T>&);

template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vector) {
  stream << "[ ";
  bool comma = false;
  for (auto& x : vector) {
    if (comma) stream << ", ";
    comma = true;
    stream << x;
  };
  return stream << " ]";
};

template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::map<std::string, T>& map) {
  stream << "{ ";
  bool comma = false;
  for (auto& kv : map) {
    if (comma) stream << ", ";
    comma = true;
    stream << kv.first << " => " << kv.second;
  };
  return stream << " }";
};

// Custom user class
struct User {
  std::string field;

  User() {};
  User(const char* f): field(f) {};
  virtual ~User() {};

  bool operator==(const User& u) const {
    return field == u.field;
  };
};

static void print(std::ostream& stream, const User& user) {
  stream << "user " << user.field;
};

// Custom user class with implemented json encode/decode
struct User1: public User {
  User1() {};
  User1(const char* f): User(f) {};
};

namespace ToolFramework {
  static bool json_encode_r(std::ostream& stream, const User1& user, adl_tag tag) {
    return json_encode_r(stream, user.field, tag);
  };

  static bool json_decode_r(const char*& input, User1& user, adl_tag tag) {
    const char* i = input;
    if (!json_decode_r(i, user.field, tag)) return false;
    input = i;
    return true;
  };
};

// Custom user class with implemented input/output operators
struct User2: public User {
  User2() {};
  User2(const char* f): User(f) {};
};

static std::ostream& operator<<(std::ostream& output, const User2& user) {
  return output << user.field;
};

static std::istream& operator>>(std::istream& input, User2& user) {
  return input >> user.field;
};

// Custom user class with implemented both json encode/decode and input/output
// operators
struct User3: public User {
  User3() {};
  User3(const char* f): User(f) {};
};

namespace ToolFramework {
  static bool json_encode_r(
      std::ostream& stream, const User3& user, adl_tag tag
  ) {
    return json_encode_r(stream, user.field, tag);
  };

  static bool json_decode_r(
      const char*& input, User3& user, adl_tag tag
  ) {
    const char* i = input;
    if (!json_decode_r(i, user.field, tag)) return false;
    input = i;
    return true;
  };
};

static std::ostream& operator<<(std::ostream& output, const User3& user) {
  return output << user.field;
};

static std::istream& operator>>(std::istream& input, User3& user) {
  return input >> user.field;
};

// Custom user class with json encode/decode implemented through inheritance
struct User4: public User1 {
  User4() {};
  User4(const char* f): User1(f) {};
};

// Custom user class with both json encode/decode and input/output operators
// implemented through inheritance
struct User5: public User3 {
  User5() {};
  User5(const char* f): User3(f) {};
};

// Custom user class with json encode/decode implemented through inheritance
// and custom input/output operators
struct User6: public User1 {
  User6() {};
  User6(const char* f): User1(f) {};
};

static std::ostream& operator<<(std::ostream& output, const User6& user) {
  return output << user.field;
};

static std::istream& operator>>(std::istream& input, User6& user) {
  return input >> user.field;
};

template <typename T, typename Eq = std::equal_to<T>>
static bool test_var(
    const char* key,
    const T& value,
    const char* json = nullptr,
    Eq eq = std::equal_to<T>(),
    const std::function<void (std::ostream&, const T&)>& printer
      = [](std::ostream& stream, const T& value) { stream << value; }
) {
  auto log = [&]() -> std::ostream& {
    std::cout
      << "test_var<"
      << typeid(T).name()
      << "> `"
      << key
      << '\'';
    if (printer) {
      std::cout << " `";
      printer(std::cout, value);
      std::cout << '\'';
    };
    return std::cout << ": ";
  };

  Store store;
  store.Set(key, value);

  T value2;
  store.Get(key, value2);
  if (!eq(value, value2)) {
    log() << "Get returned `";
    printer(std::cout, value2);
    std::cout << "'. Store contents:\n";
    store.Print();
    return false;
  };

  std::string json2;
  store >> json2;
  if (json && !json_eq(json, json2.c_str())) {
    log()
      << "JSON mismatch: expected `"
      << json
      << "', got `"
      << json2
      << "'. Store contents:\n";
    store.Print();
    return false;
  };

  Store store2;
  if (!store2.JsonParser(json2)) {
    log() << "JsonParser failed on `" << json2 << "'\n";
    return false;
  };

  store2.Get(key, value2);
  if (!eq(value, value2)) {
    log() << "Get after deserialization returned `";
    printer(std::cout, value2);
    std::cout << "'\n";
    return false;
  };

  return true;
};

static bool test_json(
    const char* json,
    bool valid = true,
    const char* expected = nullptr
) {
  if (!expected) expected = json;

  auto log = [&]() -> std::ostream& {
    return std::cout << "test_json `" << json << "': ";
  };

  Store store;
  if (store.JsonParser(json) != valid) {
    log()
      << "JsonParser "
      << (valid ? "failed" : "unexpectedly succeeded")
      << '\n';
    return false;
  };

  std::string json2;
  store >> json2;

  if (!json_eq(expected, json2.c_str())) {
    log() << "JSON mismatch: got `" << json2 << "'\n";
    return false;
  };

  return true;
};

int main(int argc, char** argv) {
  Tester t;

  t(test_var("a", 1, "{\"a\":1}"));
  t(test_var("b", static_cast<short>(2), "{\"b\":2}"));
  t(test_var("c", 3L, "{\"c\":3}"));
  t(test_var("d", 4.4f, "{\"d\":4.4}"));
  t(test_var("e", 5.5, "{\"e\":5.5}"));
  t(test_var("f", true, "{\"f\":1}"));
  t(test_var("g", 'h', "{\"g\":\"h\"}"));
  t(test_var<std::string>("h", "hello world", "{\"h\":\"hello world\"}"));

  // TODO: make it work with const char*
  t(test_var<std::string>("i", "q \" w", "{\"i\":\"q \\\" w\"}"));

  {
    Store s;
    s.Set("a", "q } w");
    t(test_var("j", s, "{\"j\":{\"a\":\"q } w\"}}"));
  };

  t(test_var<std::string>("k", "{\"a\":{\"q } w\"}}", "{\"k\":\"{\\\"a\\\":{\\\"q } w\\\"}}\"}"));
  t(test_var("l", std::vector<int> { 42, 11 }, "{\"l\":[42,11]}"));

  t(
      test_var(
        "m",
        std::vector<float> { -2.5e-10, -0.1, 0.1, 2.5e10 },
        "{\"m\":[-2.5e-10,-0.1,0.1,2.5e+10]}"
      )
  );

  {
    Store s;
    s.Set("x", std::vector<int> { -5, 5 });
    s.Set("y", std::vector<float> { 0, 1.333 });
    t(test_var("n", s, "{\"n\":{\"x\":[-5,5],\"y\":[0,1.333]}}"));
  };

  t(
      test_var(
        "o",
        std::vector<std::string> { "qwe , asd", "q \"}] w" },
        "{\"o\":[\"qwe , asd\",\"q \\\"}] w\"]}"
      )
  );

  t(
      test_var(
        "p",
        std::vector<std::vector<int>> { { 1, 2 }, { -1, -2 } },
        "{\"p\":[[1,2],[-1,-2]]}"
      )
  );

  t(
      test_var(
        "q",
        std::map<std::string, std::string> {
          { "a", "qwe" },
          { "q \" w", "x } z" }
        },
        "{\"q\":{\"a\":\"qwe\",\"q \\\" w\":\"x } z\"}}"
      )
  );

  t(
      test_var(
        "r",
        std::vector<std::map<std::string, std::vector<int>>> {
          {
            { "a", { 1, 2 } },
            { "b", { -1, -2 } }
          },
          {
            { "q \" w", { 0, 42 } },
            { "x ] z", { 15, 11 } }
          }
        },
        "{\"r\":[{\"a\":[1,2],\"b\":[-1,-2]},{\"q \\\" w\":[0,42],\"x ] z\":[15,11]}]}"
      )
  );

  t(
      test_var(
        "user1",
        User1("user1"),
        "{\"user1\":\"user1\"}",
        std::equal_to<User1>(),
        std::function<void (std::ostream&, const User1&)>(print)
      )
  );

  t(
      test_var(
        "user2",
        User2("user2"),
        "{\"user2\":\"user2\"}",
        std::equal_to<User2>(),
        std::function<void (std::ostream&, const User2&)>(print)
      )
  );

  t(
      test_var(
        "user3",
        User3("user3"),
        "{\"user3\":\"user3\"}",
        std::equal_to<User3>(),
        std::function<void (std::ostream&, const User3&)>(print)
      )
  );

  t(
      test_var(
        "user4",
        User4("user4"),
        "{\"user4\":\"user4\"}",
        std::equal_to<User4>(),
        std::function<void (std::ostream&, const User4&)>(print)
      )
  );

  t(
      test_var(
        "user5",
        User5("user5"),
        "{\"user5\":\"user5\"}",
        std::equal_to<User5>(),
        std::function<void (std::ostream&, const User5&)>(print)
      )
  );

  t(
      test_var(
        "user6",
        User6("user6"),
        "{\"user6\":\"user6\"}",
        std::equal_to<User6>(),
        std::function<void (std::ostream&, const User6&)>(print)
      )
  );

  t(
      test_json(
        "{\"key\":[{\"a\":{\"b\":[1,false]}},{\"a\":{\"b\":[null,-2.5e-2]}}]}"
      )
  );

  // adapted from https://github.com/briandfoy/json-acceptance-tests, pass1
  {
    const char* json =
      "{\n"
"        \"integer\": 1234567890,\n"
"        \"real\": -9876.543210,\n"
"        \"e\": 0.123456789e-12,\n"
"        \"E\": 1.234567890E+34,\n"
"        \"\":  23456789012E66,\n"
"        \"zero\": 0,\n"
"        \"one\": 1,\n"
"        \"space\": \" \",\n"
"        \"quote\": \"\\\"\",\n"
"        \"backslash\": \"\\\\\",\n"
"        \"controls\": \"\\b\\f\\n\\r\\t\",\n"
"        \"slash\": \"/ & \\/\",\n"
"        \"alpha\": \"abcdefghijklmnopqrstuvwyz\",\n"
"        \"ALPHA\": \"ABCDEFGHIJKLMNOPQRSTUVWYZ\",\n"
"        \"digit\": \"0123456789\",\n"
"        \"0123456789\": \"digit\",\n"
"        \"special\": \"`1~!@#$%^&*()_+-={':[,]}|;.</>?\",\n"
"        \"hex\": \"\\u0123\\u4567\\u89AB\\uCDEF\\uabcd\\uef4A\",\n"
"        \"true\": true,\n"
"        \"false\": false,\n"
"        \"null\": null,\n"
"        \"array\":[  ],\n"
"        \"object\":{  },\n"
"        \"address\": \"50 St. James Street\",\n"
"        \"url\": \"http://www.JSON.org/\",\n"
"        \"comment\": \"// /* <!-- --\",\n"
"        \"# -- --> */\": \" \",\n"
"        \" s p a c e d \" :[1,2 , 3\n"
"\n"
",\n"
"\n"
"4 , 5        ,          6           ,7        ],\"compact\":[1,2,3,4,5,6,7],\n"
"        \"jsontext\": \"{\\\"object with 1 member\\\":[\\\"array with 1 element\\\"]}\",\n"
"        \"quotes\": \"&#34; \\u0022 %22 0x22 034 &#x22;\",\n"
"        \"\\/\\\\\\\"\\uCAFE\\uBABE\\uAB98\\uFCDE\\ubcda\\uef4A\\b\\f\\n\\r\\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?\"\n"
": \"A key can be any string\"\n"
"    }"
    ;

    const char* expected =
"      {\n"
"              \"\":  23456789012E66,\n"
"              \" s p a c e d \" :[1,2 , 3\n"
"\n"
"      ,\n"
"\n"
"      4 , 5        ,          6           ,7        ],\n"
"              \"# -- --> */\": \" \",\n"
"              \"/\\\\\\\"\\ucafe\\ubabe\\uab98\\ufcde\\ubcda\\uef4a\\b\\f\\n\\r\\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?\"\n"
"      : \"A key can be any string\",\n"
"              \"0123456789\": \"digit\",\n"
"              \"ALPHA\": \"ABCDEFGHIJKLMNOPQRSTUVWYZ\",\n"
"              \"E\": 1.234567890E+34,\n"
"              \"address\": \"50 St. James Street\",\n"
"              \"alpha\": \"abcdefghijklmnopqrstuvwyz\",\n"
"              \"array\":[  ],\n"
"              \"backslash\": \"\\\\\",\n"
"              \"comment\": \"// /* <!-- --\","
"\"compact\":[1,2,3,4,5,6,7],\n"
"              \"controls\": \"\\b\\f\\n\\r\\t\",\n"
"              \"digit\": \"0123456789\",\n"
"              \"e\": 0.123456789e-12,\n"
"              \"false\": 0,\n"
"              \"hex\": \"\\u0123Eg\\u89ab\\ucdef\\uabcd\\uef4a\",\n"
"              \"integer\": 1234567890,\n"
"              \"jsontext\": \"{\\\"object with 1 member\\\":[\\\"array with 1 element\\\"]}\",\n"
"              \"null\": 0,\n"
"              \"object\":{  },\n"
"              \"one\": 1,\n"
"              \"quote\": \"\\\"\",\n"
"              \"quotes\": \"&#34; \\\" %22 0x22 034 &#x22;\",\n"
"              \"real\": -9876.543210,\n"
"              \"slash\": \"/ & /\",\n"
"              \"space\": \" \",\n"
"              \"special\": \"`1~!@#$%^&*()_+-={':[,]}|;.</>?\",\n"
"              \"true\": 1,\n"
"              \"url\": \"http://www.JSON.org/\",\n"
"              \"zero\": 0\n"
"      }\n"
    ;

    t(test_json(json, true, expected));
  };

  if (t) std::cout << "ok\n";

  return 0;
};
