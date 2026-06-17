#ifndef TAGGED_SERIALISABLE_OBJECT_H
#define TAGGED_SERIALISABLE_OBJECT_H

#include <BinaryStream.h>
#include <SerialisableObject.h>



//fixme use zie_t for sizes but have todo in binary stream as well;

namespace ToolFramework{

  class Tag : public SerialisableObject{

  public:

    //unsigned int size = 0;
    size_t size = 0;
    std::string type = "";
    unsigned int version = 0;
    
    bool Print(){
      
      std::cout<<"size:type:version = "<<size<<":"<<type<<":"<<version<<std::endl;
      
      return true;
    }     

    bool Serialise(BinaryStream &bs){

      if(!(bs & type)) return false;
      if(!(bs & version)) return false;
      if(!(bs & size)) return false;

      return true;
    }
    
    void Skip(BinaryStream &bs){
      
      bs.Bseek(size, SEEK_CUR);
      
    }

    
  };


  class TaggedSerialisableObject : public SerialisableObject{

  public:

    virtual std::string GetType()=0;
    unsigned int GetSize(){

      BinaryStream bs;
      Serialise(bs);
      return bs.Size();
    }
    virtual unsigned int GetVersion()=0;
    bool SerialiseWrapper(BinaryStream &bs){

      if(bs.m_write){
	//size_t size= GetSize();
	unsigned int version = GetVersion();
	if(!(bs << GetType())) return false;    
	if(!(bs << version)) return false;
	//if(!(bs << size)) return false;
	BinaryStream tmp_bs;
	Serialise(tmp_bs);
	return bs << tmp_bs;
      }

      return Serialise(bs);
      
      
    }
    


  private:
    
    
    
    
  };
  
}

#endif
