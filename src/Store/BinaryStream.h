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
 private:
  // Equivalent to std::is_baseof<Derived, Base> in C++11
  template<typename Derived, typename Base>
  class is_base_of {
    private:
      static short check(const Base*);
      static char  check(...);
    public:
      static const bool value
	= sizeof(check(static_cast<const Derived*>(0))) == sizeof(short);
  };

  // Equivalent to std::enable_if<B, T> in C++11
  template <bool B, typename T = void> struct enable_if {};

  template <typename T>
  struct enable_if<true, T> {
    typedef T type;
  };
  
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

  template <typename T> bool operator&(const T& rhs) {
    if (!m_write) return false;
    return *this << rhs;
  }

  template <typename T> bool operator&(T& rhs) {
    if (m_write) return *this << rhs;
    return *this >> rhs;
  }

  template <typename T>
  typename enable_if<is_base_of<T, SerialisableObject>::value, bool>::type
  operator<<(T& rhs) {
    if (m_mode == READ) return false;
    m_write = true;
    return rhs.SerialiseWrapper(*this);
  }

  template <typename T>
  typename enable_if<is_base_of<T, SerialisableObject>::value, bool>::type
  operator>>(T& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;
    m_write = false;
    return rhs.SerialiseWrapper(*this);
  }

  template <typename T>
  typename enable_if<!is_base_of<T, SerialisableObject>::value, bool>::type
  operator<<(const T& rhs) {
    if (m_mode == READ) return false;
    return Bwrite(&rhs, sizeof(T));
  }

  template <typename T>
  typename enable_if<!is_base_of<T, SerialisableObject>::value, bool>::type
  operator>>(T& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;
    return Bread(&rhs, sizeof(T));
  }

  bool operator<<(const std::string& rhs) {
    if (m_mode == READ) return false;

    unsigned int len = rhs.length();
    if (!(*this << len)) return false;

    if (len == 0) return true;
    return Bwrite(rhs.data(), len);
  }

  bool operator>>(std::string& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int len = 0;
    if (!(*this >> len)) return false;

    rhs.resize(len);
    if (len == 0) return true;
    return Bread(rhs.data(), len);
  }

  template <typename T>
  typename enable_if<is_base_of<T, SerialisableObject>::value, bool>::type
  operator<<(std::vector<T>& rhs) {
    return serialise_container(rhs);
  }

  template <typename T>
  typename enable_if<is_base_of<T, SerialisableObject>::value, bool>::type
  operator>>(std::vector<T>& rhs) {
    return deserialise_container(rhs);
  }

  template <typename T>
  typename enable_if<!is_base_of<T, SerialisableObject>::value, bool>::type
  operator<<(const std::vector<T>& rhs) {
    if (m_mode == READ) return false;

    unsigned int size = rhs.size();
    if (!(*this << size)) return false;

    if (size == 0) return true;
    return Bwrite(rhs.data(), size * sizeof(T));
  }

  template <typename T>
  typename enable_if<!is_base_of<T, SerialisableObject>::value, bool>::type
  operator>>(std::vector<T>& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int size = 0;
    if (!(*this >> size)) return false;

    rhs.resize(size);
    if (size == 0) return true;
    return Bread(rhs.data(), size * sizeof(T));
  }

  bool operator<<(const std::vector<std::string>& rhs) {
    return serialise_container(rhs);
  };

  bool operator>>(std::vector<std::string>& rhs) {
    return deserialise_container(rhs);
  };

  template <typename First, typename Second>
  bool operator<<(std::pair<First, Second>& rhs) {
    if (m_mode == READ) return false;
    return *this << rhs.first && *this << rhs.second;
  }

  template <typename First, typename Second>
  bool operator>>(std::pair<First, Second>& rhs) {
    if (m_mode == NEW || m_mode == APPEND) return false;
    return *this >> rhs.first && *this >> rhs.second;
  }

  template <typename T, typename U> bool operator<<(std::map<T, U>& rhs) {
    return serialise_container(rhs);
  }

  template <typename T, typename U> bool operator>>(std::map<T, U>& rhs) {
    // std::map has no resize, so we cannot use deserialise_container
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

  template <typename T> bool operator<<(std::deque<T>& rhs) {
    return serialise_container(rhs);
  }

  template <typename T> bool operator>>(std::deque<T>& rhs) {
    return deserialise_container(rhs);
  }

 private:

  int def(FILE *source, FILE *dest, int level);
  int inf(FILE *source, FILE *dest);
  void zerr(int ret);

  template <typename Container>
  bool serialise_container(const Container& container) {
    if (m_mode == READ) return false;

    unsigned int size = container.size();
    if (!(*this << size)) return false;

    if (size == 0) return true;

    for (typename Container::const_iterator i = container.begin();
	 i != container.end();
	 ++i)
      if (!(*this << *i)) return false;
    return true;
  }

  template <typename Container>
  bool deserialise_container(Container& container) {
    if (m_mode == NEW || m_mode == APPEND) return false;

    unsigned int size = 0;
    if (!(*this >> size)) return false;

    if (size == 0) return true;

    container.resize(size);
    for (typename Container::iterator i = container.begin();
	 i != container.end();
	 ++i)
      if (!(*this >> *i)) return false;
    return true;
  }
};

#endif
