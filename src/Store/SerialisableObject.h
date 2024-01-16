#ifndef SERIALISABLEOBJECT_H
#define SERIALISABLEOBJECT_H

#include <string>

namespace ToolFramework{

  /**
   * \class SerialisableObject
   *
   * An abstract base class for sustom calsses to inherit from to ensure version and type information are present, as well as a Print function and some form of sereialisation.
   *
   * $Author: B.Richards $
   * $Date: 2019/05/28 10:44:00 $
   */
  
  //class archive;
  class BinaryStream;
  
  class SerialisableObject{
    
    
  public:
    virtual ~SerialisableObject(){};
    bool SerialiseWrapper(BinaryStream &bs);
    virtual bool Serialise(BinaryStream &bs)=0;
    virtual std::string GetVersion()=0;
    virtual bool Print()=0; ///< Simple virtual Pritn function to ensure inhereted classes have one
    // virtual ~SerialisableObject(){}; ///< Destructor
    //bool m_serialise; ///< Denotes if the calss should be serialised or not when added to a BoostStore. 
    //protected:
    
    //std::string type; ///< String to store type of Tool
    //  std::string m_version; ///< String to store version of Tool
    
    /**
       Simple Boost serialise method to serialise the membervariables of a custom class. This shuld be expanded to include the custom classes variables
       @param ar Boost archive.
       @param version of the archive.
    */
    /*    template<class Archive> void serialize(Archive & ar, const unsigned int version){  
	  if(serialise){
	  ar & type;
	  ar & version;
	  }
	  }
    */
    
    
    
  };
  
}


#endif
