#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <ctime>
#include <map>

#include <pthread.h>
#include <time.h>
#include <mutex>
#include <unistd.h>

namespace ToolFramework{

  #ifndef NO_COLOUR
  const char red[] = "\033[31m"; //"\033[38;5;88m"
  const char lightred[] = "\033[91m"; //"\033[38;5;196m"
  const char green[] = "\033[32m"; //"\033[38;5;22m"
  const char lightgreen[] = "\033[92m"; //"\033[38;5;46m"
  const char blue[] = "\033[34m"; //"\033[38;5;18m"
  const char lightblue[] = "\033[94m"; //"\033[38;5;21m"
  const char yellow[] = "\033[33m";  //"\033[38;5;142m"
  const char lightyellow[] = "\033[93m"; //"\033[38;5;226m"
  const char orange[] = "\033[38;5;130m";
  const char lightorange[] = "\033[38;5;208m";
  const char pink[] = "\033[35m";  //"\033[38;5;129m"
  const char lightpink[] = "\033[95m";  //"\033[38;5;201m"
  const char purple[] = "\033[38;5;54m";
  const char lightpurple[] = "\033[38;5;57m";
  const char cyan[] = "\033[36m";   //"\033[38;5;39m"
  const char lightcyan[] = "\033[96m";  //"\033[38;5;51m" 
  const char white[] = "\033[97m"; //"\033[38;5;255m"
  const char gray[] = "\033[37m"; //"\033[38;5;243m"
  const char plain[] ="\033[39m";  //"\033[0m"

  #else

  const char red[] = ""; 
  const char lightred[] = "";
  const char green[] = ""; 
  const char lightgreen[] = ""; 
  const char blue[] = ""; 
  const char lightblue[] = ""; 
  const char yellow[] = ""; 
  const char lightyellow[] = ""; 
  const char orange[] = ""; 
  const char lightorange[] = ""; 
  const char pink[] = ""; 
  const char lightpink[] = ""; 
  const char purple[] = ""; 
  const char lightpurple[] = ""; 
  const char cyan[] = ""; 
  const char lightcyan[] = "";  
  const char white[] = ""; 
  const char gray[] = ""; 
  const char plain[] = ""; 
  
#endif

  
  /**
   * \struct MsgL
   *
   *This struct variables for setting the message level of streamed messages to the logging class
   *
   *
   * $Author: B.Richards $
   * $Date: 2020/12/30 12:50:00 $
   */
  
  struct MsgL{
    
    MsgL(int in_messagelevel, int in_verbose){
      messagelevel=in_messagelevel;
      verbose=in_verbose;
    }
    MsgL ML(int in_messagelevel) {
      messagelevel=in_messagelevel;
      return *this;
    }
    
    int messagelevel; ///< Message level paramiter
    int verbose; ///< Verbosity level pramiter
    
  };
  
  
  /**
   * \class Logging
   *
   *This class handels the logging, which can be directed to screen or file or over the via the ToolChain Config file
   *
   *
   * $Author: B.Richards $
   * $Date: 2019/05/27 18:34:00 $
   */
  
  
  
  class Logging: virtual public std::ostream {
    
  public:
    
    class TFStreamBuf: virtual public std::stringbuf 
    {
      
    public:
      
      TFStreamBuf(){};
      TFStreamBuf(bool interactive, bool local=false,  std::string localpath="./log", bool error=false, std::ostream* filestream=0);
      
      virtual ~TFStreamBuf();
      
      virtual int sync ( );
      
      bool ChangeOutFile(std::string localpath);
      
      int m_messagelevel;
      int m_verbose;
      
      std::ostream*   output;
      std::ostream*   fileoutput;

      std::mutex lock1;
      std::mutex lock2;

    protected:
      
      std::ostream* tmp;
      
      bool m_local;
      bool m_interactive;
      bool m_error;
      
      std::ofstream file;
      std::streambuf *psbuf, *backup1, *backup2;
      
      bool no_delete;

    }; 
  
  
  /**
     Constructor for Logging class
     
     @param interactive if loging class prints log output to screen
     @param local if loggign class prints log output to file
     @param localpath Local path for log output file 
     @param split_output_files if log is directed to disk then whether or nto to use sperate files for standard output and standard error
  */
  
  // Logging(std::ostream& str, std::string mode, std::string localpath=""):std::ostream(&buffer), buffer(str, mode, localpath){};
  
  //Logging(std::ostream& str, std::string mode, std::string localpath=""):std::ostream(buffer), buffer(new MyStreamBuf(str, mode, localpath)), errbuffer(new MyStreamBuf(str, mode, localpath)){};
  
  Logging(bool interactive, bool local=false,  std::string localpath="./log", bool split_output_files=false);//:buffer(new MyStreamBuf(interactive, local, localpath, false)),errbuffer(new MyStreamBuf(interactive, local, localpath, true)){};
  
  //:std::ostream(buffer){};
  //, buffer(new MyStreamBuf(interactive, local, "", error)){};
  
  Logging(){};
  
  virtual ~Logging();
  
  /**
     Function to create a log messages. 
     
     @param message templated log message text.
     @param messagelevel message verbosity level of the message being sent (e.g. if 'messagelevel>= verbose' Then message is sent). 
     @param verbose verbosity level of the current Tool.    
     
  */
  
    template <typename T>  void Log(T message, int messagelevel=1, int verbose=1){
      //printf("logmessage\n");
      if(messagelevel<=verbose){
	
	buffer->lock1.lock();
	while(!buffer->lock2.try_lock()){
	    buffer->lock1.unlock();
	    usleep(100);
	    buffer->lock1.lock();
	  }	    
	//printf("locg message locked\n");
	int previous_messagelevel = buffer->m_messagelevel;
	int previous_verbosity = buffer->m_verbose;
	
	buffer->m_messagelevel=messagelevel;
	buffer->m_verbose=verbose;
	//printf("log about to sync\n");
	std::cout.rdbuf(buffer);
	std::cout<<message<<plain<<std::endl;
	//printf("log synced\n");
	buffer->m_messagelevel=previous_messagelevel;
	buffer->m_verbose=previous_verbosity;
	buffer->lock2.unlock();
	buffer->lock1.unlock();
      } 
      
    }
    
    
  /**
     Functionn to change the logs out file if set to a local path.
     
     @param localpath path to new log file.
     @return value is bool success of opening new logfile.
     
  */
  bool ChangeOutFile(std::string localpath){return buffer->ChangeOutFile(localpath);} 
  
  
  
  
  Logging& operator<<(MsgL a){

    buffer->lock1.lock();
    buffer->lock2.try_lock();
    
    buffer->m_messagelevel=a.messagelevel;
    buffer->m_verbose=a.verbose;
    
    buffer->lock2.unlock();
    buffer->lock1.unlock();
    
    return *this;
  }
  
  
  Logging& operator<<(std::ostream& (*foo)(std::ostream&)) {
    //printf("special stream tying to lock\n");
    buffer->lock1.lock();
    buffer->lock2.try_lock();
    
    //printf("special stream unlocking and about to sync\n");
    
    std::cout.rdbuf(buffer);
    std::cout<<plain<<foo;//std::endl;
    //printf("special stream synced\n");
    buffer->lock2.unlock();
    buffer->lock1.unlock();
    
    return *this;
  }
  
  template<typename T>  Logging& operator<<(T &a){
    //printf("stream locking\n");
    buffer->lock1.lock();
    buffer->lock2.try_lock();
    //printf("stream locked\n");
    
    std::cout.rdbuf(buffer);
    std::cout<<a;
    
    buffer->lock1.unlock();
    //printf("stream unlocked\n");
    return *this;
    
  }
  
  template<typename T>  Logging& operator<<(const T &a){
    //printf("stream locking\n");
    buffer->lock1.lock();
    buffer->lock2.try_lock();
    //printf("stream locked\n");
    
    std::cout.rdbuf(buffer);
    std::cout<<a;
    
    buffer->lock1.unlock();
    //printf("stream unlocked\n");
    
    return *this;
    
  }
  
  
  protected:
  
  TFStreamBuf* buffer; ///< Stream buffer used to replace std::cout for redirection to coustom output.
  TFStreamBuf* errbuffer;
  
  
  };
  
}

#endif
