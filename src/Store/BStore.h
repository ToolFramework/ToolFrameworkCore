#ifndef BSTORE_H
#define BSTORE_H

#include <iostream>
#include <stdio.h>
#include <map>
#include <string.h>
#include "zlib.h"
#include <unistd.h> //for lseek
#include <sstream>
#include <assert.h>
#include <typeinfo>
#include <SerialisableObject.h>
#include <PointerWrapper.h>

#include <BinaryStream.h>
#include <sys/stat.h>

namespace ToolFramework{
  
  enum enum_type {uncompressed, compressed, post_pre_compress, ram};
  
  
  /**
   * \class BStore
   *
   * This class Is a dynamic data storeage class and can be used to store variables of any type listed by ASCII key. The storage of the varaible is a binarystream.
   *
   * $Author: B.Richards $
   * $Date: 2022/01/23 10:44:00 $
   */
  
  
  
  class BStore: public SerialisableObject{
    
#define CHUNK 16384
    
  public:
    
    BStore(bool header=true, bool type_checking=false);
    BStore(const BStore &bs);
    ~BStore();
    //  void Init();
    // void Init2();
    bool Initnew(std::string filename, enum_type type=post_pre_compress, bool header=true, bool type_checking=false, unsigned int file_end=0);    
    bool Initnew(BinaryStream& bs, unsigned int position);    
    bool GetFlags(unsigned int file_end);
    bool GetFlags(std::string filename, unsigned int file_end);
    bool WriteHeader();
    bool WriteLookup();
    bool WriteFlags();
    bool Save(unsigned int entry=0);
    bool GetHeader();
    bool GetEntry(unsigned int entry_request);
    bool DeleteEntry(unsigned int entry_request);
    unsigned int NumEntries();
    bool Close();
    bool Rollback();
    
    std::string GetVersion();
    
    
    std::map<std::string,BinaryStream> m_variables;
    std::map<std::string,std::string> m_type_info;
    // std::map<std::string,BinaryStream> m_header;
    BStore* Header; 
    std::map<std::string,PointerWrapperBase*> m_ptrs;
    
    
    /////////importing
    
    void JsonParser(std::string input); ///< Converts a flat JSON formatted string to Store entries in the form of key value pairs.  @param input The input flat JSON string. 
    bool Print();
    void Print(bool values); ///< Prints the contents of the BoostStore. @param values If true values and keys are printed. If false just keys are printed
    void Delete(); ///< Deletes all entries in the BoostStore.
    void Remove(std::string key); ///< Removes a single entry from the BoostStore. @param key The key of the entry to remove. 
    std::string Type(std::string key); ///< Queries the type of an entry if type checking is turned on. @param key The key of the entry to check. @return A string encoding the type info.
    bool Has(std::string key); ///< Queries if entry exists in a BoostStore. @param key is the key of the varaible to look up. @return true if varaible is present in the store, false if not. 
    //  BoostStore *Header; ///< Pointer to header BoostStore (only available in multi event BoostStore). This can be used to access and assign header varaibles jsut like a standard non multi event store.
    
    /**
       Templated getter function for BoostStore content. Assignment is templated and via reference.
       @param name The ASCII key that the variable in the BoostStore is stored with.
       @param out The variable to fill with the value.
       @return Return value is true if varaible exists in the Store and false if not or wrong type.
    */
    template<typename T> bool Get(std::string name,T &out){
      
      if(m_variables.count(name)>0){
	
	if(m_type_info[name]==typeid(out).name() || !m_type_checking){
	  
	  m_variables[name].m_pos=0;
	  return m_variables[name] >> out;
	  
	}
	else return false;
      }
      
      else return false;
      
    }
    
    /**
       Templated getter function for pointer to BoostStore content. Assignment is templated and the object is created on first the heap and a pointer to it assigned. Latter requests will return a pointer to the first object. Bote the pointer is to an indipendant instance of the stored object and so changing its value without using a Setter will not effect the stored value. 
       @param name The ASCII key that the variable in the BoostStore is stored with.
       @param out The pointer to assign.
       @return Return value is true if varaible exists in the Store and false if not or wrong type.
    */
    template<typename T> bool Get(std::string name,T* &out){  
      
      if(m_variables.count(name)>0 || m_ptrs.count(name)>0){
	if((m_type_info[name]==typeid(T).name() || !m_type_checking ) || (m_ptrs.count(name)>0 && m_variables.count(name)==0)){
	  
	  bool ret=true;
	  if(m_ptrs.count(name)==0){
	    
	    T* tmp=new T;
	    m_ptrs[name]=new PointerWrapper<T>(tmp);
	    ret*=Get(name,*tmp);
	  }
	  
	  
	  PointerWrapper<T>* tmp=static_cast<PointerWrapper<T>* >(m_ptrs[name]); 
	  out=tmp->pointer;
	  
	  return ret;
	  
	}
	else return false;
	
      }
      
      else return false;
      
    }
    
    
    /**
       Templated setter function to assign vairables in the BoostStore.
       @param name The key to be used to store and reference the variable in the BoostStore.
       @param in the varaible to be stored.
    */
    template<typename T> bool Set(std::string name,T& in){
      //std::cout<<"in set"<<std::endl;
      m_variables[name].buffer.clear();
      m_variables[name].m_pos=0;
      //std::cout<<"set serialising"<<std::endl;
      bool ret=m_variables[name] << in;
      //std::cout<<"set serialised ="<<ret<<std::endl;
      if(m_type_checking) m_type_info[name]=typeid(in).name();
      
      return ret;
    }
    
    /**
       Returns string pointer to Store element.
       @param key The key of the string pointer to return.
       @return a pointer to the string version of the value within the Store.
    */  
    BinaryStream* operator[](std::string key){
      return &m_variables[key];
    }
    
    /**
       Templated setter function to assign vairables in the BoostStore from pointers.
       @param name The key to be used to store and reference the variable in the BoostStore.
       @param in A pointer to the object to be stored. The function will store the value the pointer poitns to in the archive and keep hold of the pointer to return to Get calls later.
       @param persist Indicates if the object that is being pointed to is stored or not. If not only the pointer is retained for later use but not in the archive and so will not be saved with the BoostStore on save calls.
    */
    template<typename T> bool Set(std::string name,T* in, bool persist=true){
      
      bool ret=true;
      
      if(m_ptrs.count(name)>0){
	PointerWrapper<T> *tmp = static_cast<PointerWrapper<T> *>(m_ptrs[name]);
	
      
	if(tmp->pointer!=in){
	  delete m_ptrs[name];
	  m_ptrs[name]=0;	
	  m_ptrs[name]= new PointerWrapper<T>(in);
	}
      }
      
      else if(m_ptrs.count(name)==0)  m_ptrs[name]= new PointerWrapper<T>(in);
      
      
      if(persist) ret*=Set(name,*in);
      
      return ret;
    }
    
  
    
    /**
       Allows streaming of a flat JASON formatted string of BoostStore contents.
    */
    template<typename T> void operator>>(T& obj){
      
      std::stringstream stream;
      stream<<"{";
      bool first=true;
      for (std::map<std::string,BinaryStream>::iterator it=m_variables.begin(); it!=m_variables.end(); ++it){
	if (!first) stream<<",";
	stream<<"\""<<it->first<<"\":\""<< it->second.buffer<<"\"";
	first=false;
      }
      stream<<"}";
      
      obj=stream.str();
      
    }
    
    bool Serialise(BinaryStream &bs);
    
    //    std::map<unsigned int, unsigned int> m_lookup; //why is this a map?? should change it when you ahve had more sleep
    
    std::vector<unsigned int> m_lookup;
    
    BinaryStream output;
    
  private:
    
    
    
    unsigned int m_file_end;
    std::string m_file_name;
    unsigned int m_open_file_end;
    unsigned int m_previous_file_end;
    enum_type m_type;
    bool m_type_checking;
    bool m_has_header;
    unsigned int m_header_start;
    
    unsigned int m_flags_start;
    
    
    unsigned int m_lookup_start;
    //unsigned int m_lookup_size;
    
    //  std::map<unsigned int, unsigned int> m_lookup;
    //unsigned int m_entry;
    // unsigned int m_current_loaded_entry;
    bool m_update;
    
    //int m_file_type; //0=gzopen, 1=fopen, 2=stringstream
    float m_version;
    
    
  };

}

#endif





  /////////////////////////////////////////////////////////////////
  ////////////////////// BStore Binary file schematic /////////////
  /////////////////////////////////////////////////////////////////
  /// entry 0                                      
  /// *entry 0 type_info
  /// entry 1 
  /// entry 1 
  /// entry 1 extra data for nesting 
  /// entry 2 
  /// *entry 2 type_info
  /// ..
  /// ..
  /// Header                                          :  m_header_start
  /// lookup 0                                        :  m_lookup_start
  /// lookup 1 
  /// ..
  /// ..
  /// m_version                                       :  m_flags_start    #here down always uncompressed
  /// m_header_start                                  
  /// m_has_header
  /// m_lookup_start
  /// m_type_checking
  /// m_type   
  /// m_previous_file_end
  //////////////////////////////////////////////////  :  m_file_end  m_open_file_end



