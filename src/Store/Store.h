#ifndef STORE_H 
#define STORE_H

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>
#include <sstream> 

/**
 * Replace all instances of @param from inside @param str by @param to.
 * https://stackoverflow.com/a/24315631
 */
static inline void ReplaceAll(std::string &str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
}

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
    void JsonParser(std::string input); ///<  Converts a flat JSON formatted string to Store entries in the form of key value pairs.  @param input The input flat JSON string.
    void Print(); ///< Prints the contents of the Store.
    void Delete(); ///< Deletes all entries in the Store.
    bool Has(std::string key); ///<Returns bool based on if store contains entry given by sting @param string key to comapre.
    std::vector<std::string> Keys(); //returns a vector of the keys
    bool Destring(std::string key); //convers an element from a string by stripping the speachmarks @param string key to comapre.
    
    /**
       Templated getter function for sore content. Assignment is templated and via reference.
       @param name The ASCII key that the variable in the Store is stored with.
       @param out The variable to fill with the value.
       @return Return value is true if varaible exists in the Store and correctly assigned to out and false if not.
    */
    template<typename T> bool Get(std::string name,T &out){
      
      if(m_variables.count(name)>0){
	
	std::stringstream stream(StringStrip(m_variables[name]));
	stream>>out;
	return !stream.fail();
      }
      
      return false;
      
    }

    /**
       Templated getter function for sore vector/array content. Assignment is templated and via reference.
       @param name The ASCII key that the variable in the Store is stored with.
       @param out The variable to fill with the value.
       @return Return value is true if varaible exists in the Store and correctly assigned to out and false if not.
    */
    template<typename T> bool Get(std::string name,std::vector<T> &out){

      if(m_variables.count(name)>0 && StringStrip(m_variables[name])[0]=='['){
	std::stringstream stream;
	out.clear();
	  
	for(unsigned int i=1; i<m_variables[name].length(); i++){
	  if(m_variables[name][i]!=',' && m_variables[name][i]!=']'){
	    if(m_variables[name][i]!='"') stream.put(m_variables[name][i]);
	  }
	  else {
	    T tmp;
	    stream>>tmp;
	    if(stream.fail()){
	      out.clear();
	      return false;
	    }
	    stream.str("");
	    stream.clear();
	    out.push_back(tmp);
	  }
	  
	}
	return true;
	
      }
      
      return false;
      
    }
    
    /**
       getter function for string content..
       @param name The ASCII key that the variable in the Store is stored with.
       @param out The variable to fill with the value.
       @return Return value is true if varaible exists in the Store and correctly assigned to out and false if not.
    */
    bool Get(std::string name, std::string &out);

        /**
       getter function for bool content..
       @param name The ASCII key that the variable in the Store is stored with.
       @param out The variable to fill with the value.
       @return Return value is true if varaible exists in the Store and correctly assigned to out and false if not.
    */
    bool Get(std::string name, bool &out);

        /**
       getter function for store content..
       @param name The ASCII key that the variable in the Store is stored with.
       @param out The variable to fill with the value.
       @return Return value is true if varaible exists in the Store and correctly assigned to out and false if not.
    */
    bool Get(std::string name, Store &out);

    
    /**
       Templated getter function for store content. 
       @param name The ASCII key that the variable in the Store is stored with.
       @return Return value is default copiler costructed value if not true (note: no checking exists)
    */
    template<typename T> T Get(std::string name){
      
      T tmp;
      if(!Get(name,tmp)) std::cout<<"\033[38;5;196mERROR: Store doesnt hold value \""<<name<<"\" default returned\033[0m"<<std::endl;
      
      return tmp;
      
    }

    
    
    /**
       Templated setter function to assign vairables in the Store.
       @param name The key to be used to store and reference the variable in the Store.
       @param in the varaible to be stored.
    */
    template<typename T> void Set(std::string name,T in){
      std::stringstream stream;
      stream<<in;
      m_variables[name]=stream.str();
    }

    /**
       string setter function to assign vairables in the Store.
       @param name The key to be used to store and reference the variable in the Store.
       @param in the varaible to be stored.
    */

    void Set(std::string name, std::string in);

    /**
       string setter function to assign vairables in the Store.
       @param name The key to be used to store and reference the variable in the Store.
       @param in the varaible to be stored.
    */

    void Set(std::string name, const char* in);

    /**
       Templated setter function to assign vairables in the Store from a vector.
       @param name The key to be used to store and reference the variable in the Store.
       @param in the varaible to be stored.
    */
    
    template<typename T> void Set(std::string name,std::vector<T> in){
     
      std::stringstream stream;
      std::string tmp="[";
      for(unsigned int i=0; i<in.size(); i++){
	stream<<in.at(i);
	tmp+=stream.str();
	if(i!=in.size()-1)tmp+=',';
	stream.str("");
	stream.clear();	
      }
      tmp+=']';
      m_variables[name]=tmp;
      
    }

    /**
       string setter function to assign vairables in the Store from a vector.
       @param name The key to be used to store and reference the variable in the Store.
       @param in the varaible to be stored.
    */
    
    void Set(std::string name,std::vector<std::string> in);
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
    template<typename T> void operator>>(T& obj){

      std::stringstream stream;
      stream<<"{";
      bool first=true;
      for (std::map<std::string,std::string>::iterator it=m_variables.begin(); it!=m_variables.end(); ++it){
	if (!first) stream<<", ";

	//Replace any occurances of "{ or }"
	// (e.g. if one of the it->second is a JSON dump of its own)
	// This should also work if there are multiple levels of JSON dump
	ReplaceAll(it->second, "\"{", "{");
	ReplaceAll(it->second, "}\"", "}");

	stream<<"\""<<it->first<<"\":"<< it->second;
	
	first=false;
      }
      stream<<"}";

      obj=stream.str();
    } 
    
    std::map<std::string, std::string>::iterator begin() { return m_variables.begin(); }
    std::map<std::string, std::string>::iterator end()   { return m_variables.end(); }
    
    
  private:
    
    
    std::map<std::string,std::string> m_variables;
    std::string StringStrip(std::string in);
    
  };

}

#endif
