#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <mutex>
#include <SerialisableObject.h>
#include <BinaryStream.h>

namespace ToolFramework{
  
  template<class T> class Buffer: SerialisableObject{
    
  public:
    
    Buffer(){;}
    void Add(T &in){
      std::lock_guard<std::mutex> lock(mtx);    
      data.push_back(in);
    };
    void Swap(std::vector<T> &in){
      std::lock_guard<std::mutex> lock(mtx);
      if(data.size()) std::swap (data, in);
    
    }
    size_t Size(){
      
      std::lock_guard<std::mutex> lock(mtx);
      return data.size();
      
    }
    void Clear(){
      std::lock_guard<std::mutex> lock(mtx);
      data.clear();
    }
  
    
    std::string GetVersion(){ return "1";}
    
    bool Print(){

      return true;
    }
    
    bool Serialise(BinaryStream& bs){
      
      bs & data;
      return true;
    }
    
  
  private:
    std::vector<T> data;
    std::mutex mtx;
    
  };
  
}

#endif
