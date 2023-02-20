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
  long int Btell();
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
  long int m_pos;

  enum_mode m_mode; //0=READ, 1==WRITE, 2==READ/APPEND
  //  0=READ, 1==WRITE, 2==APPEND, 3 READ/WRITE, 4 READ/APPEND/ , 5 READ/OVEWRITE
  //  READ   NEW     APPEND        UPDATE    READ/APPEND  , NEW/READ

  //gz read r, write w, append a
  //fopen read r, write w, append a, read/write r+ (file must exist), read/write w+ (file doesnt exist/overwrite), read/append a+

  //operator overloads

  bool operator<<(std::string& rhs) {
    if (m_mode == READ) return false;

    unsigned int len = rhs.length();
    if (!(*this << len)) return false;

    if (len == 0) return true;
    return Bwrite(rhs.data(), len);
  };

  bool operator<<(const std::string& rhs) {
    if (m_mode == READ) return false;

    unsigned int len = rhs.length();
    if (!(*this << len)) return false;

    if (len == 0) return true;
    return Bwrite(rhs.data(), len);
  };

  bool operator>>(std::string& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int len = 0;
    if (!(*this >> len)) return false;

    rhs.resize(len);
    if (len == 0) return true;
    return Bread(rhs.data(), len);
  };

  bool operator&(std::string& rhs){
    if(m_write) return (*this) << rhs;
    else return (*this) >> rhs;
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
  
  template<typename T> bool operator<<(std::vector<T>& rhs) {
    if (m_mode == READ) return false;

    unsigned int size = rhs.size();
    if (!(*this << size)) return false;

    if (size == 0) return true;

    if (!check_base<SerialisableObject, T>::value)
      return Bwrite(rhs.data(), size * sizeof(T));

    for (typename std::vector<T>::iterator it = rhs.begin(); it != rhs.end(); ++it)
      if (!(*this << *it)) return false;
    return true;
  }

  template<typename T> bool operator>>(std::vector<T>& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int size = 0;
    if (!(*this >> size)) return false;

    rhs.resize(size);
    if (size == 0) return true;

    if (!check_base<SerialisableObject, T>::value)
      return Bread(rhs.data(), size * sizeof(T));

    for (typename std::vector<T>::iterator it = rhs.begin(); it != rhs.end(); ++it)
      if (!(*this >> *it)) return false;
    return true;
  }
  
  template<typename T> bool operator&(std::vector<T>& rhs){
    
    if(m_write) return (*this) << rhs;
    else return (*this) >> rhs;
  }

  bool operator<<(std::vector<std::string>& rhs) {
    if (m_mode == READ) return false;

    unsigned int size = rhs.size();
    if (!(*this << size)) return false;

    for (unsigned int i = 0; i < size; ++i)
      if (!(*this << rhs[i])) return false;
    return true;
  }

  bool operator>>(std::vector<std::string>& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int size = 0;
    if (!(*this >> size)) return false;

    rhs.resize(size);
    for (unsigned int i = 0; i < size; ++i)
      if (!(*this >> rhs[i])) return false;
    return true;
  }
  
  bool operator&(std::vector<std::string>& rhs){   
    if(m_write) return (*this) << rhs;
    else return (*this) >> rhs;
  }

  template<typename T, typename U> bool operator<<(std::map<T, U>& rhs) {
    if (m_mode == READ) return false;

    unsigned int size = rhs.size();
    if (!(*this << size)) return false;

    for (typename std::map<T, U>::iterator it = rhs.begin(); it != rhs.end(); ++it) {
      T key = it->first;
      U value = it->second;
      if (!(*this << key && *this << value)) return false;
    }
    return true;
  }

  template<typename T, typename U> bool operator>>(std::map<T, U>& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int size = 0;
    if (!(*this >> size)) return false;

    for (unsigned int i = 0; i < size; ++i) {
      T key;
      U value;
      if (!(*this >> key && *this >> value)) return false;
      rhs[key] = value;
    };
    return true;
  }
  
  template<typename T, typename U> bool operator&(std::map<T,U>& rhs){
    if(m_write) return (*this) << rhs;
    else return (*this) >> rhs;
  } 

  template<typename T> bool operator<<(std::deque<T>& rhs) {
    if (m_mode == READ) return false;

    unsigned int size = rhs.size();
    if (!(*this << size)) return false;

    if (size == 0) return true;

    for (typename std::deque<T>::iterator it = rhs.begin();
	 it != rhs.end();
	 ++it)
      if (check_base<SerialisableObject, T>::value) {
	if (!(*this << *it))
	  return false;
      } else if (!Bwrite(&*it, sizeof(T)))
	return false;
    return true;
  }

  template<typename T> bool operator>>(std::deque<T>& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int size = 0;
    if (!(*this >> size)) return false;

    rhs.resize(size);
    if (size == 0) return true;

    for (typename std::deque<T>::iterator it = rhs.begin();
	 it != rhs.end();
	 ++it)
      if (check_base<SerialisableObject, T>::value) {
	if (!(*this >> *it))
	  return false;
      } else if (!Bread(&*it, sizeof(T)))
	return false;
    return true;
  }

  template<typename T> bool operator&(std::deque<T>& rhs){
    if(m_write) return(*this) << rhs;
    else return(*this) >> rhs;    
  }

  bool operator<<(std::deque<std::string>& rhs) {
    if (m_mode == READ) return false;

    unsigned int len = rhs.size();
    if (!(*this << len)) return false;

    for (unsigned int i = 0; i < len; ++i)
      if (!(*this << rhs[i])) return false;
    return true;
  };

  bool operator>>(std::deque<std::string>& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int size = 0;
    if (!(*this >> size)) return false;

    rhs.resize(size);
    for (unsigned int i = 0; i < size; ++i)
      if (!(*this >> rhs[i])) return false;
    return true;
  };
  
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


#endif
