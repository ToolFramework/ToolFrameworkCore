#include <BinaryStream.h>

using namespace ToolFramework;

BinaryStream::BinaryStream(enum_endpoint endpoint){

  m_endpoint=endpoint;
  m_pos=0;
  m_write=true;
  m_file_name="";
  m_mode=UPDATE;  
  //  m_serialise=true;
  pfile=0;
#ifdef ZLIB
  gzfile=0;
#endif
}

BinaryStream::~BinaryStream(){

  if((m_endpoint==UNCOMPRESSED || m_endpoint==POST_PRE_COMPRESS) && pfile!=NULL) Bclose(true);
#ifdef ZLIB
  else if(m_endpoint==COMPRESSED && gzfile!=0) Bclose();
#endif
}


std::string BinaryStream::GetVersion(){return "mydata:1.0.0";}

//Todo:

//could put safety on close so cant be called twice, requires some way of recording state of gzfile as cant tell if open or closed (thinking either flag for state or better to use pointer if possible)

//need clear function delete buffer and reset m_pos
//need reset function for resetting m_pos

//posiblly pfile/gzfile direct initialise will need this for nested stores
//need direct initialisation form gxfile or pfile given a starting position for nested stores.

bool BinaryStream::Bopen(std::string filename, enum_mode method, enum_endpoint endpoint){//make methods auto from READ WRITE strings (maybe append in future)
  //auto to highest endpoint unless specified;

  enum_endpoint prev_endpoint=m_endpoint;
  m_endpoint=endpoint;
  m_file_name=filename;
  m_mode=method;
  
#ifndef ZLIB
  if(m_endpoint==COMPRESSED) m_endpoint=UNCOMPRESSED; // or give warning havent decided
  else if(m_endpoint==POST_PRE_COMPRESS) m_endpoint=UNCOMPRESSED;
#endif
 
  if(m_endpoint==RAM){
    //      endpoint=reinterpret_cast<int>(method);//how to check file type? maybe end control variable/ /or default to highest endpoint // nope read from serialise you dummy... but how think maybe go back to end of file control variable... wait could pass it on the method variable!?!?! good idea
    if(prev_endpoint!=RAM) m_endpoint=prev_endpoint;
    else{
      m_endpoint=UNCOMPRESSED;
#ifdef ZLIB
      m_endpoint=POST_PRE_COMPRESS;
#endif
    }
    
    //std::cout<<"m_endpoint before convert="<<m_endpoint<<std::endl;  
    if(m_endpoint==COMPRESSED) m_endpoint=POST_PRE_COMPRESS;
    //std::cout<<"m_endpoint after convert="<<m_endpoint<<std::endl;
    //std::cout<<"filename="<<filename<<std::endl;
    if(!Bopen(filename,READ,m_endpoint)) return false; 
    //std::cout<<"j1 "<<Btell()<<std::endl;
    if(!Bseek(0,SEEK_END)) return false;
    //std::cout<<"j2 "<<Btell()<<std::endl;  
    unsigned int end= Btell();
    //std::cout<<"j3 end="<<end<<std::endl;
    if(!Bseek(0,SEEK_SET)) return false;
    //std::cout<<"j4"<<std::endl;    
    buffer.resize(end);
    //std::cout<<"j5 end="<<end<<std::endl;
    if(!Bread(&(buffer[0]),end)) return false;
    //std::cout<<"j6"<<std::endl;    
    if(!Bclose()) return false;
    //std::cout<<"j7"<<std::endl;
    m_endpoint=RAM;
    return true;
  }
#ifdef ZLIB
  else if(m_endpoint== POST_PRE_COMPRESS){
    struct stat fbuffer;
    if(stat (m_file_name.c_str(), &fbuffer) == 0){ //if file exists
      FILE* source = fopen(m_file_name.c_str(), "rb");
      if(source==NULL) return false;
      //int fd = mkstemp(filename)
      m_file_name+=".tmp";
      //    tmp_file_name.resize(L_tmpnam);
      // tmpnam(&(tmp_file_name[0]));
      FILE* destination = fopen(m_file_name.c_str(), "wb");
      if(destination==NULL) return false;
      //    std::set_terminate([](){ std::cout << "Unhandled exception" << std::endl; remove(m_file_name.c_str()); std::abort();});
      inf(source, destination); // not sure of return type
      if(fclose(source)) return false;
      if(fclose(destination)) return false;
      //  m_recompressed_file_name=m_file_name;
      if(!Bopen(m_file_name.c_str(),method,UNCOMPRESSED)) return false;
    }
    else{
      // m_recompressed_file_name=m_file_name;
      m_file_name+=".tmp";
      if(!Bopen(m_file_name.c_str(),method,UNCOMPRESSED)) return false;
    }
    m_endpoint=POST_PRE_COMPRESS;
    return true; 
  }
#endif
 
  else if(m_endpoint==UNCOMPRESSED || m_endpoint==POST_PRE_COMPRESS){
    if(method==READ) pfile = fopen(m_file_name.c_str(), "rb");
    else if(method==NEW)  pfile = fopen(m_file_name.c_str(), "wb");
    else if(method==APPEND)  pfile = fopen(m_file_name.c_str(), "ab");
    else if(method==UPDATE){
      struct stat fbuffer;
      if(stat (filename.c_str(), &fbuffer) == 0) pfile = fopen(m_file_name.c_str(), "rb+"); //if file exists 
      else pfile = fopen(m_file_name.c_str(), "wb+"); //if it doesnt 
    }
    else if(method==READ_APPEND) pfile = fopen(m_file_name.c_str(), "ab+");
    else if(method==NEW_READ) pfile = fopen(m_file_name.c_str(), "wb+");
    else return false;
    if(pfile==NULL) return false; 
    else return true;
  }
  
#ifdef ZLIB
  else if(m_endpoint==COMPRESSED){
    gzfile=new gzFile;
    if(method==READ) *gzfile = gzopen(m_file_name.c_str(), "rb");
    else if (method==NEW) *gzfile = gzopen(m_file_name.c_str(), "wb");
    else if (method==APPEND) *gzfile = gzopen(m_file_name.c_str(), "ab");
    else {
      delete gzfile;
      gzfile=0;
      return false;
    }
    if(*gzfile==NULL) return false;
    return true;
  }
#endif
  else{
    m_file_name="";
    return false;
  }
}


bool BinaryStream::Bclose(bool Ignore_Post_Pre_compress){
  
  if(m_endpoint==RAM){
    buffer.clear();
    m_pos=0;
    return true;
  }
  else if(m_endpoint==UNCOMPRESSED){
    if(pfile!=0 && !fclose(pfile)){
      pfile=0;
      return true;
    }
    else return false;
  }   
#ifdef ZLIB 
  else if(m_endpoint==POST_PRE_COMPRESS){
    if(pfile!=0 && !fclose(pfile)) pfile=0;
    else return false;
    std::string tmpfile=m_file_name;
    m_file_name.erase(m_file_name.end()-4,m_file_name.end()); // check return
    if(!Ignore_Post_Pre_compress){
      FILE* source = fopen(tmpfile.c_str(), "rb");
      FILE* destination = fopen(m_file_name.c_str(), "wb");
      def(source, destination, 9); //not sure fo return
      if(fclose(source)) return false;
      if(fclose(destination)) return false;
    }
    remove(tmpfile.c_str()); // not sure of return
    return true; 
  
  }
  
  else if(m_endpoint==COMPRESSED){
    if(gzfile==0) return false;
    else{
      bool ret=(Z_OK==gzclose(*gzfile)); // strange return
      gzfile=0;
      return ret;
    }
  }
#endif
  return false;
  
}

bool BinaryStream::Bwrite(const void* in, unsigned int size){

  if(m_endpoint==RAM){
    buffer.resize(buffer.length()+size);
    memcpy(&(buffer[m_pos]), in, size);
    m_pos+=size;
    return true;
  }
  else if(m_endpoint==UNCOMPRESSED || m_endpoint==POST_PRE_COMPRESS){
    return fwrite(in , size, 1, pfile);
  } 
#ifdef ZLIB
  else if(m_endpoint==COMPRESSED) return size==gzwrite(*gzfile, in, size);
#endif
  else return false;

}

bool BinaryStream::Bread(void* out, unsigned int size){
  
  if(m_endpoint==RAM){
    memcpy(out, &(buffer[m_pos]), size);
    m_pos+=size;
    return true;
  }
  else if(m_endpoint==UNCOMPRESSED || m_endpoint==POST_PRE_COMPRESS) return fread(out , size, 1, pfile);
#ifdef ZLIB
  else if(m_endpoint==COMPRESSED) return size==gzread(*gzfile, out, size);
#endif
  else return false;
  
}


unsigned long int BinaryStream::Btell(){
  
  if(m_endpoint==RAM){
    return m_pos;
  }
  else if(m_endpoint==UNCOMPRESSED || m_endpoint==POST_PRE_COMPRESS){
    return (unsigned long) ftell(pfile);
  }
#ifdef ZLIB
  else if(m_endpoint==COMPRESSED){
    return gztell(*gzfile);
  }
#endif
  else return false;
}

bool BinaryStream::Bseek(unsigned int pos, int whence){

  if(m_endpoint==RAM){
    if(whence==SEEK_SET) m_pos=pos;
    else if(whence==SEEK_CUR) m_pos+=pos;
    else if(whence==SEEK_END) m_pos=buffer.length()+pos;
    return true;
  }
  else if(m_endpoint==UNCOMPRESSED || m_endpoint==POST_PRE_COMPRESS){
    if(whence==SEEK_END) return (-1!=lseek(fileno(pfile), pos, whence));
    else return !fseek(pfile, pos, whence);
  }
#ifdef ZLIB
  else if(m_endpoint==COMPRESSED){
    if(whence==SEEK_END) return false; // could rewrite to get end pos with fopen and lseek
    return (-1!=gzseek(*gzfile, pos, whence));
  }
#endif
  else return false;
}

bool BinaryStream::Print(){

  std::cout<<"endpoint="<<(int)m_endpoint<<std::endl;
  return true;
}

bool BinaryStream::Serialise(BinaryStream &bs){ // do return properly
  
  if(m_endpoint==RAM){
    bool ret= (bs & buffer);
    m_pos=0;
    return ret;
  }
  else{
    if(bs.m_write){
      //std::cout<<"w1"<<std::endl;
      if(!Bclose()) return false;
      //std::cout<<"w2"<<std::endl;
      if(!Bopen(m_file_name,READ,RAM)) return false;
      //std::cout<<"w3"<<std::endl;
      if(!(bs & buffer)) return false;
      //std::cout<<"w4"<<std::endl;
      if(!Bclose()) return false;
      //std::cout<<"w5"<<std::endl;
      return true;
    }
    else{
      enum_endpoint tmp=m_endpoint;
      if(!Bclose()) return false;
      if(!Bopen(m_file_name,NEW,tmp)) return false;
      if(!(bs & buffer)) return false;
      Bwrite(&(buffer[0]), buffer.length());
      if(!Bclose()) return false;
      if(!Bopen(m_file_name,READ,tmp)) return false;
      return true;
    }
    
  }
  
}

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int BinaryStream::def(FILE *source, FILE *dest, int level){
  #ifdef ZLIB
  int ret, flush;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];
  
  /* allocate deflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, level);
  if (ret != Z_OK)
    return ret;
  
  /* compress until end of file */
  do {
    strm.avail_in = fread(in, 1, CHUNK, source);
    if (ferror(source)) {
      (void)deflateEnd(&strm);
      return Z_ERRNO;
    }
    flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = in;
    
    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = deflate(&strm, flush);    /* no bad return value */
      assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
      have = CHUNK - strm.avail_out;
      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
	(void)deflateEnd(&strm);
	return Z_ERRNO;
      }
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);     /* all input will be used */
    
    /* done when last data in file processed */
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);        /* stream will be complete */
  
  /* clean up and return */
  (void)deflateEnd(&strm);
  return Z_OK;
#endif
  return 0;
}

int BinaryStream::inf(FILE *source, FILE *dest){

#ifdef ZLIB
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];
  
  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit(&strm);
  if (ret != Z_OK)
    return ret;
  
  /* decompress until deflate stream ends or end of file */
  do {
    strm.avail_in = fread(in, 1, CHUNK, source);
    if (ferror(source)) {
      (void)inflateEnd(&strm);
      return Z_ERRNO;
    }
    if (strm.avail_in == 0)
      break;
    strm.next_in = in;
    
    /* run inflate() on input until output buffer not full */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
      switch (ret) {
      case Z_NEED_DICT:
	ret = Z_DATA_ERROR;     /* and fall through */
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
	(void)inflateEnd(&strm);
	return ret;
      }
      have = CHUNK - strm.avail_out;
      if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
	(void)inflateEnd(&strm);
	return Z_ERRNO;
      }
    } while (strm.avail_out == 0);
    
    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);
  
  /* clean up and return */
  (void)inflateEnd(&strm);
  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;

#endif
  return 0;
}

/* report a zlib or i/o error */
void BinaryStream::zerr(int ret){
#ifdef ZLIB
  
  fputs("zpipe: ", stderr);
  switch (ret) {
  case Z_ERRNO:
    if (ferror(stdin))
      fputs("error reading stdin\n", stderr);
    if (ferror(stdout))
      fputs("error writing stdout\n", stderr);
    break;
  case Z_STREAM_ERROR:
    fputs("invalid compression level\n", stderr);
    break;
  case Z_DATA_ERROR:
    fputs("invalid or incomplete deflate data\n", stderr);
    break;
  case Z_MEM_ERROR:
    fputs("out of memory\n", stderr);
    break;
  case Z_VERSION_ERROR:
    fputs("zlib version mismatch!\n", stderr);
  }
#endif
}

bool SerialisableObject::SerialiseWrapper(BinaryStream &bs){
  //  if(!m_serialise) return false; //not sure i should ahve a serialise flag, causes major issues with empty mapps and vector elements!!!! so remove. People wouldnt be calling serialise method if they didnt want to serialse
  //std::cout<<"in serialise wrapper"<<std::endl;
  if(bs.m_write){
    if(!(bs << GetVersion())) return false;
  }
  else{
    std::string version="";
    if(!(bs >> version)) return false;
    if(version!=GetVersion()) return false;
  }
  return Serialise(bs);
}
