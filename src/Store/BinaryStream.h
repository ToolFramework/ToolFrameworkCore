#ifndef BINARYSTREAM_H
#define BINARYSTREAM_H

#include <string.h>
#include <string>
#include <iostream>
#include <unistd.h> //for lseek
#include <vector>
#include <map>
#include <deque>
#include <SerialisableObject.h>
#include <stdio.h>
#include <sys/stat.h>
#ifdef ZLIB
#include <zlib.h>
#endif

#include <assert.h> //needed for inflate deflate

namespace ToolFramework{
  
#define CHUNK 16384 // dito
  
  enum enum_endpoint { RAM , UNCOMPRESSED , POST_PRE_COMPRESS, COMPRESSED };
  enum enum_mode { READ , NEW , APPEND, UPDATE, READ_APPEND, NEW_READ };
  
  class BinaryStream : public SerialisableObject{ 
    
  public:
    
    BinaryStream(enum_endpoint endpoint=RAM);
    ~BinaryStream();
    bool Bopen(std::string filename, enum_mode method=UPDATE, enum_endpoint endpoint=POST_PRE_COMPRESS);
    bool Bclose(bool Ignore_Post_Pre_compress=false);
    bool Bwrite(const void* in, unsigned int size);
    bool Bread(void* out, unsigned int size);
    unsigned long int Btell();
    bool Bseek(unsigned int pos, int whence);
    bool Print();
    bool Serialise(BinaryStream &bs);  
    std::string GetVersion();
    
    enum_endpoint m_endpoint;
    FILE* pfile;
#ifdef ZLIB
    gzFile* gzfile;
#endif
    std::string buffer;
    bool m_write;
    std::string m_file_name;
    unsigned long int m_pos;
    
    enum_mode m_mode; //0=READ, 1==WRITE, 2==READ/APPEND
    //  0=READ, 1==WRITE, 2==APPEND, 3 READ/WRITE, 4 READ/APPEND/ , 5 READ/OVEWRITE
    //  READ   NEW     APPEND        UPDATE    READ/APPEND  , NEW/READ
    
    //gz read r, write w, append a
    //fopen read r, write w, append a, read/write r+ (file must exist), read/write w+ (file doesnt exist/overwrite), read/append a+
    
    //operator overloads
    
    bool operator<<(std::string& rhs){
      if(m_mode!=READ){
	bool ret=true;    
	unsigned int tmp=rhs.length();
	ret*=(*this) << tmp;
	if(tmp) ret*=Bwrite(&(rhs[0]), tmp);
	return ret;
      }
      else return false;
    }
    
    bool operator>>(std::string& rhs){
      if(m_mode!=NEW && m_mode!=APPEND){
	bool ret=true;
	unsigned int tmp=0;
	ret*=(*this) >> tmp;
	rhs.resize(tmp);
	if(tmp) ret*=Bread(&(rhs[0]), tmp);
	return ret;
      }
      else return false;
    }
    
    bool operator&(std::string& rhs){
      if(m_write) return (*this) << rhs;
      else return (*this) >> rhs;
    }
    
    bool operator<<(const std::string& rhs){
      if(m_mode!=READ){
	bool ret=true;
	unsigned int tmp=rhs.length();
	ret*=(*this) << tmp;
	if(tmp) ret*=Bwrite(&(rhs[0]), tmp);
	return ret;
      }
      else return false;
    }
    
    bool operator&(const std::string& rhs){
      if(m_write) return (*this) << rhs;
      return false;
    }
    
    
    template<typename T> bool operator<<(T& rhs){
      if(m_mode!=READ){
	if(check_base<SerialisableObject,T>::value){
	  SerialisableObject* tmp=reinterpret_cast<SerialisableObject*>(&rhs);
	  m_write=true;
	  return tmp->SerialiseWrapper(*this);
	}
	else return Bwrite(&rhs, sizeof(T));
      }
      else return false;  
    }
    
    
    template<typename T> bool operator>>(T& rhs){
      if(m_mode!=NEW && m_mode!=APPEND){
	if(check_base<SerialisableObject,T>::value){
	  SerialisableObject* tmp=reinterpret_cast<SerialisableObject*>(&rhs);
	  m_write=false;
	  return tmp->SerialiseWrapper(*this);
	}
	else return Bread(&rhs, sizeof(T)); 
      }
      return false;
    }
    
    
    template<typename T> bool operator&(T& rhs){
      if(m_write)  return (*this) << rhs;
      else return (*this) >> rhs; 
    }
    
    template<typename T> bool operator<<(const T& rhs){
      if(m_mode!=READ){
	if(check_base<SerialisableObject,T>::value){
	  SerialisableObject* tmp=reinterpret_cast<SerialisableObject*>(&rhs);
	  m_write=true;
	  return tmp->SerialiseWrapper(*this);
	}
	return Bwrite(&rhs, sizeof(T));
      }
      else return false;
    }
    
    template<typename T> bool operator&(const T& rhs){
      if(m_write) return (*this) << rhs;
      return false;
    }
    
    
    template<typename T> bool operator<<(std::vector<T>& rhs){
      
      if(m_mode!=READ){
	bool ret=true;
	unsigned int tmp=rhs.size();
	ret*=(*this) << tmp;
	if(tmp){
	  if(check_base<SerialisableObject,T>::value){
	    for(typename std::vector<T>::iterator it=rhs.begin(); it!=rhs.end(); it++) ret*=(*this) << (*it);
	  }
	  else ret*=Bwrite(&(rhs[0]), tmp*sizeof(T));
	}
	return ret; 
      }
      else return false;
    }
    
    template<typename T> bool operator>>(std::vector<T>& rhs){
      
      if(m_mode!=NEW && m_mode!=APPEND){
	bool ret=true;
	unsigned int tmp=0;
	ret*=(*this) >> tmp;
	rhs.resize(tmp);
	if(tmp){
	  if(check_base<SerialisableObject,T>::value){
	    for(typename std::vector<T>::iterator it=rhs.begin(); it!=rhs.end(); it++) ret*=(*this) >> (*it);
	  }
	  else ret*=Bread(&(rhs[0]), tmp*sizeof(T));
	}
	return ret;
      }
      else return false;
    }
    
    template<typename T> bool operator&(std::vector<T>& rhs){
      
      if(m_write) return (*this) << rhs;
      else return (*this) >> rhs;
    }
    
    bool operator<<(std::vector<std::string>& rhs){
      if(m_mode!=READ){
	bool ret=true;
	unsigned int tmp=rhs.size();
	ret*=(*this) << tmp;
	for(unsigned int i=0; i<tmp; i++){
	  ret*=(*this) << rhs.at(i);
	}
	return ret;
      }
      else return false;
    }
    
    bool operator>>(std::vector<std::string>& rhs){
      if(m_mode!=NEW && m_mode!=APPEND){
	bool ret=true;
	unsigned int tmp=0;
	ret*=(*this) >> tmp;
	rhs.resize(tmp);
	for(unsigned int i=0; i<tmp; i++){
	  ret*=(*this) >> rhs.at(i);
	}
	return ret;
      }
      else return false;
    }
    
    bool operator&(std::vector<std::string>& rhs){   
      if(m_write) return (*this) << rhs;
      else return (*this) >> rhs;
    }
    
    template<typename T, typename U> bool operator<<(std::map<T,U>& rhs){
      if(m_mode!=READ){
	bool ret=true;
	unsigned int tmp=rhs.size();
	ret*=(*this) << tmp;
	for (typename std::map<T,U>::iterator it=rhs.begin(); it!=rhs.end(); ++it){
	  T key=it->first;
	  U value=it->second;
	  ret*=(*this) << key;
	  ret*=(*this) << value;
	}
	return ret;
      }
      else return false;
    }
    
    template<typename T, typename U> bool operator>>(std::map<T,U>& rhs){
      if(m_mode!=NEW && m_mode!=APPEND){
	bool ret=true;
	unsigned int tmp=0;      
	ret*=(*this) >> tmp;
	for (unsigned int i=0; i<tmp; i++){
	  T key;
	  U value;
	  ret*=(*this) >> key;
	  ret*=(*this) >> value;
	  rhs[key]=value;
	}
	return ret;
      }
      else return false;
    }
    
    template<typename T, typename U> bool operator&(std::map<T,U>& rhs){
      if(m_write) return (*this) << rhs;
      else return (*this) >> rhs;
    } 
    
    template<typename T> bool operator<<(std::deque<T>& rhs){
      if(m_mode!=READ){
	bool ret=true;
	unsigned int tmp=rhs.size(); 
	ret*=(*this) << tmp;
	if(tmp){
	  if(check_base<SerialisableObject,T>::value){
	    for(typename std::deque<T>::iterator it=rhs.begin(); it!=rhs.end(); it++) ret*=(*this) << (*it);	
	  }
	  else ret*=Bwrite(&(rhs[0]), tmp*sizeof(T));
	}
	return ret;
      }
      else return false;
    }
    
    template<typename T> bool operator>>(std::deque<T>& rhs){
      if(m_mode!=NEW && m_mode!=APPEND){
	bool ret=true;
	unsigned int tmp=0;
	ret*=(*this) >> tmp;
	rhs.resize(tmp);
	if(tmp){
	  if(check_base<SerialisableObject,T>::value){
	    for(typename std::deque<T>::iterator it=rhs.begin(); it!=rhs.end(); it++) ret*=(*this) >> (*it);
	  }
	  else ret*=Bread(&(rhs[0]), tmp*sizeof(T));
	}
	return ret;
      }
      else return false;
    }
    
    template<typename T> bool operator&(std::deque<T>& rhs){
      if(m_write) return(*this) << rhs;
      else return(*this) >> rhs;    
    }
    
    bool operator<<(std::deque<std::string>& rhs){
      if(m_mode!=READ){
	bool ret=true;
	unsigned int tmp=rhs.size();
	ret*=(*this) << tmp;
	for(unsigned int i=0; i<tmp; i++){
	  ret*=(*this) << rhs.at(i);
	}
	return ret;
      }
      else return false;
    }
    
    bool operator>>(std::deque<std::string>& rhs){
      if(m_mode!=NEW && m_mode!=APPEND){
	bool ret=true;
	unsigned int tmp=0;
	ret*=(*this) >> tmp;
	rhs.resize(tmp);
	for(unsigned int i=0; i<tmp; i++){
	  ret*=(*this) >> rhs.at(i);
	}
	return ret;
      }
      else return false;
    }
    
    bool operator&(std::deque<std::string>& rhs){
      if(m_write) return (*this) << rhs;
      else return (*this) >> rhs;    
    }     
    
    
  private:
    
    int def(FILE *source, FILE *dest, int level);
    int inf(FILE *source, FILE *dest);
    void zerr(int ret);
    
    template <typename B, typename D> struct Host{
      
      operator B*() const;
      operator D*();
      
    };
    
    
    template <typename B, typename D> struct check_base {
      template <typename T> 
      static short check(D*, T);
      static char check(B*, int);
      
      static const bool value = sizeof(check(Host<B,D>(), int())) == sizeof(short);
    };
    
    
  
  /*
derived:

  yes D*(Host<B,D>(),T)   = D*(B* const, T);  not allowed
                          = D*(D*, T); ----------------------------------------------------------------------------------->   preferred as D*->D* better than D*->B* : answer yes

  no B*(Host<B,D>(), int) =  B*(B* const, int); 
                          =  B*(D*, int);  preferred as not const and object called on is not const----------------------->


not derrived:

  yes D*(Host<B,D>(),T)   =  D*(B* const, T); not allowed
                          =  D*(D*, T); ------------------------------>

  no B*(Host<B,D>(), int) =  B*(B* const, int);----------------------->  preffered as not templated : answer no
                          =  B*(D*, int); not allowed


   */


  };

}

#endif
