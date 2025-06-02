#ifndef STORE_H 
#define STORE_H

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

#include "Json.h"

namespace ToolFramework{

  /**
   * \class Store
   *
   * This class Is a dynamic data storeage class and can be used to store variables of any type listed by ASCII key. The storage of the varaible is in ASCII, so is inefficent for large numbers of entries.
   *
   * $Author: B.Richards $
   * $Date: 2019/05/28 10:44:00 $
   */
  
  class Store{
    
  public:
    
    Store(); ////< Sinple constructor
    
    bool Initialise(std::string filename); ///< Initialises Store by reading in entries from an ASCII text file, when each line is a variable and its value in key value pairs.  @param filename The filepath and name to the input file.
    bool JsonParser(const char* input);
    bool JsonParser(const std::string& input); ///<  Converts a flat JSON formatted string to Store entries in the form of key value pairs.  @param input The input flat JSON string.
    void Print(); ///< Prints the contents of the Store.
    void Delete(); ///< Deletes all entries in the Store.
    bool Has(std::string key); ///<Returns bool based on if store contains entry given by sting @param string key to comapre.
    std::vector<std::string> Keys(); //returns a vector of the keys
    bool Destring(std::string key); //convers an element from a string by stripping the speachmarks @param string key to comapre.
    bool Erase(std::string key);
    
    /**
       Templated getter function for store content. 
       @param name The ASCII key that the variable in the Store is stored with.
       @return Return value is default copiler costructed value if not true (note: no checking exists)
    */
    template<typename T> T Get(std::string name){
      
      T tmp {};
      if(!Get(name,tmp)) std::cout<<"\033[38;5;196mERROR: Store doesnt hold value \""<<name<<"\" default returned\033[0m"<<std::endl;
      
      return tmp;
      
    }
    
    /**
       Get a value from Store and put it into out.
       @param name The key name.
       @param out Variable to assign the value to.
       @return false if the key does not exit or the value could not be deserialized to this type. In the latter case, out may or may not be changed.
    */
    template <typename T>
    bool Get(const std::string& name, T& out) const {
      auto i = m_variables.find(name);
      if (i == m_variables.end()) return false;
      return deserialize(i->second, out);
    };

    /**
       Assign a value to a key.
       @param name The key name.
       @param value The value.
    */
    template <typename T>
    void Set(const std::string& name, const T& value) {
      m_variables[name] = serialize(value);
    };

    /**
       Returns string pointer to Store element.
       @param key The key of the string pointer to return.
       @return a pointer to the string version of the value within the Store.
    */
    std::string* operator[](std::string key){
      return &m_variables[key];
    }
    
    /**
       Allows streaming of a flat JASON formatted string of Store contents.
    */
    void operator>>(std::string&) const;
    
    std::map<std::string, std::string>::iterator begin() { return m_variables.begin(); }
    std::map<std::string, std::string>::iterator end()   { return m_variables.end(); }
    
    std::map<std::string, std::string>::const_iterator begin() const { return m_variables.begin(); };
    std::map<std::string, std::string>::const_iterator end()   const { return m_variables.end();   };

    std::map<std::string, std::string>::const_iterator cbegin() const { return m_variables.cbegin(); };
    std::map<std::string, std::string>::const_iterator cend()   const { return m_variables.cend();   };
    
    
  private:

    template <typename, typename = int>
    struct json_encode_exists : std::false_type {};

    template <typename T>
    struct json_encode_exists<
      T,
      decltype(
          json_encode_r(
            std::declval<std::ostream&>(),
            std::declval<T>(),
            std::declval<adl_tag>()
          ),
          0
      )
    > : std::true_type {};

    template <typename, typename = int>
    struct json_decode_exists : std::false_type {};

    template <typename T>
    struct json_decode_exists<
      T,
      decltype(
          json_decode_r(
            std::declval<const char*&>(),
            std::declval<T&>(),
            std::declval<adl_tag>()
          ),
          0
      )
    > : std::true_type {};
    
    std::map<std::string,std::string> m_variables;
    std::string StringStrip(std::string in);

    template <typename T>
    static
    typename std::enable_if<!json_decode_exists<T>::value, bool>::type
    deserialize(const std::string& value, T& out) {
      char c = 0;
      if (!value.empty()) {
        c = value[0];
        if (c == '[' || c == '{') return false;
      };

      std::stringstream ss;
      if (c == '"')
        ss.str(value.substr(1, value.size() - 2));
      else
        ss.str(value);

      ss >> out;
      return !ss.fail();
    }

    template <typename T>
    static
    typename std::enable_if<json_decode_exists<T>::value, bool>::type
    deserialize(const std::string& value, T& out) {
      const char* s = value.c_str();
      return json_decode(s, out);
    }

    template <typename T>
    static bool deserialize(const std::string& value, std::vector<T>& out) {
      if (value.empty() || value[0] != '[') return false;
      const char* v = value.c_str();
      return json_decode(v, out);
    }

    static bool deserialize(const std::string& value, std::string& out) {
      char c = value.empty() ? 0 : value[0];
      if (c == '{' || c == '[') return false;
      if (c == '"')
        out.assign(value.begin() + 1, value.end() - 1);
      else
        out = value;
      return true;
    }

    static bool deserialize(const std::string& value, char& out) {
      if (value.empty()) return false;
      switch (value[0]) {
        case '[':
        case '{':
          return false;
        case '"':
          if (value.size() < 2) return false;
          out = value[1];
          break;
        default:
          out = value[0];
      };
      return true;
    }

    static bool deserialize(const std::string& value, Store& out) {
      if (value.empty() || value[0] != '{') return false;
      // out.m_variables.clear() ?
      return out.JsonParser(value);
    }

    template <typename T>
    static
    typename std::enable_if<!json_encode_exists<T>::value, std::string>::type
    serialize(const T& value) {
      std::stringstream ss;
      ss << value;
      std::string result = ss.str();
      if (result.empty()) return result;
      char r = result[0];
      if (r == '"' || r == '[' || r == '{')
        json_encode(result, ss.str());
      return result;
    }

    template <typename T>
    static
    typename std::enable_if<json_encode_exists<T>::value, std::string>::type
    serialize(const T& value) {
      std::string result;
      json_encode(result, value);
      return result;
    }

    template <typename T>
    static std::string serialize(const std::vector<T>& value) {
      std::string result;
      json_encode(result, value);
      return result;
    }

    static std::string serialize(const char* value) {
      std::stringstream ss;
      ss << '"' << value << '"';
      return ss.str();
    }

    static std::string serialize(const std::string& value) {
      return serialize(value.c_str());
    }

    static std::string serialize(char value) {
      if (value == '"') return "\"\\\"\"";
      if (value == '[') return "\"[\"";
      if (value == '{') return "\"{\"";
      return std::string(1, value);
    }

    static std::string serialize(const Store& value) {
      std::string result;
      value >> result;
      return result;
    }

  };

  std::ostream& operator<<(std::ostream&, const Store&);

} // end ToolFramework namespace
  

#endif
