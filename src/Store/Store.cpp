#include <iomanip>
#include <memory>

#include "Store.h"

namespace ToolFramework {
  
  Store::Store(){}
  
  
  bool Store::Initialise(std::string filename){
    
    std::ifstream file(filename.c_str());
    std::string line;
    
    if(file.is_open()){
      
      while (getline(file,line)){
	if (line.size()>0){
	  if (line.at(0)=='#')continue;
	  std::string key="";
	  std::string value="";
	  std::stringstream stream(line);
	  stream>>key>>value;
	  std::string tmp;
	  stream>>tmp;
	  value='"'+value;
	  
	  while(tmp.length() && tmp[0]!='#'){
	    value+=" "+tmp;
	    tmp="";
	    stream>>tmp;
	  }
	  value+="\"";
	  
	  if(value!="") m_variables[key]=value;
	}
	
      }
      file.close();
    }
    else{
      std::cout<<"\033[38;5;196m WARNING!!!: Config file "<<filename<<" does not exist no config loaded \033[0m"<<std::endl;
      return false;
    }
  
    return true;
  }
  
  void Store::Print(){
  
    for (std::map<std::string,std::string>::iterator it=m_variables.begin(); it!=m_variables.end(); ++it){
      
      std::cout<< it->first << " => " << it->second <<std::endl;
      
    }
    
  }
  
  
  void Store::Delete(){
    
    m_variables.clear();
    
    
  }

  bool Store::JsonParser(const char* input) {
    bool result = json_decode_object(
        input,
        [this](const char*& input_, std::string key) -> bool {
          const char* i = input_;
          i = json_scan_whitespace(i);
          switch (*i) {
            case '"':
              {
                std::string value;
                if (!json_internal::json_decode_string(i, value, true))
                  return false;
                m_variables[std::move(key)] = std::move(value);
              };
              break;
            case 'f':
              i = json_scan_token(i, "false");
              if (!i) return false;
              m_variables[std::move(key)] = "0";
              break;
            case 't':
              i = json_scan_token(i, "true");
              if (!i) return false;
              m_variables[std::move(key)] = "1";
              break;
            case 'n':
              i = json_scan_token(i, "null");
              if (!i) return false;
              m_variables[std::move(key)] = "0";
              break;
            default:
              const char* value = i;
              i = json_scan(i);
              if (!i) return false;
              m_variables[std::move(key)] = std::string(value, i);
          };
          input_ = i;
          return true;
        }
    );
    if (!result) return false;

    input = json_scan_whitespace(input);
    if (!input || *input) return false; // junk at the end
    return true;
  };
  
  bool Store::JsonParser(const std::string& input) {
    return JsonParser(input.c_str());
  }

  bool Store::Has(std::string key){
    
    return (m_variables.count(key)!=0);
    
  }
  
  
  std::vector<std::string> Store::Keys(){
    
    std::vector<std::string> ret;
    
    for(std::map<std::string, std::string>::iterator it= m_variables.begin(); it!= m_variables.end(); it++){
      
      ret.push_back(it->first);
      
    }
    
    return ret;
    
  }
  
  
  std::string Store::StringStrip(std::string in){
    
    if(in.length() && in[0]=='"' && in[in.length()-1]=='"') return in.substr(1,in.length()-2);
    return in;
    
  }
  
  bool Store::Destring(std::string key){
    
    if(!m_variables.count(key)) return false;
    m_variables[key]=StringStrip(m_variables[key]);
    return true;
    
  }

  void Store::operator>>(std::string& out) const {
    std::stringstream ss;
    ss << *this;
    out = ss.str();
  }

  std::ostream& operator<<(std::ostream& stream, const Store& store) {
    bool comma = false;
    stream << '{';
    for (const auto& kv : store) {
      if (comma)
        stream << ',';
      else
        comma = true;

      json_encode(stream, kv.first);
      stream << ':';
      char c = kv.second.empty() ? 0 : kv.second[0];
      if (c == '{' || c == '[')
        stream << kv.second;
      else if (c == '"')
        json_encode(stream, kv.second.begin() + 1, kv.second.end() - 1);
      else {
        const char* s = json_scan_number(kv.second.c_str());
        s = json_scan_whitespace(s);
        if (s && !*s)
          stream << kv.second;
        else
          json_encode(stream, kv.second);
      };
    };
    stream << '}';
    return stream;
  }
  
  bool Store::Erase(std::string key){
    
    return m_variables.erase(key);
  
  }

}
