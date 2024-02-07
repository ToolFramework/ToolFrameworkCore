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

#define red "\033[38;5;196m"
#define darkred "\033[38;5;88m"
#define green "\033[38;5;46m"
#define darkgreen "\033[38;5;22m"
#define blue "\033[38;5;21m"
#define darkblue "\033[38;5;18m"
#define yellow "\033[38;5;226m"
#define darkyellow "\033[38;5;142m"
#define orange "\033[38;5;208m"
#define darkorange "\033[38;5;130m"
#define pink "\033[38;5;201m"
#define darkpink "\033[38;5;129m"
#define purple "\033[38;5;57m"
#define darkpurple "\033[38;5;54m"
#define cyan  "\033[38;5;51m"
#define darkcyan  "\033[38;5;39m" 
#define white "\033[38;5;255m"
#define gray "\033[38;5;243m"
#define plain "\033[0m"

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
 

   if(messagelevel<=verbose){    
     int previous_messagelevel = buffer->m_messagelevel;
     int previous_verbosity = buffer->m_verbose;

     buffer->m_messagelevel=messagelevel;
     buffer->m_verbose=verbose;

     std::cout.rdbuf(buffer);
     std::cout<<message<<plain<<std::endl;

     buffer->m_messagelevel=previous_messagelevel;
     buffer->m_verbose=previous_verbosity;
   } 
 
 }
 
 
  /**
     Functionn to change the logs out file if set to a local path.
     
     @param localpath path to new log file.
     @return value is bool success of opening new logfile.
     
  */
 bool ChangeOutFile(std::string localpath){return buffer->ChangeOutFile(localpath);} 
 
 
 
 
 Logging& operator<<(MsgL a){

   buffer->m_messagelevel=a.messagelevel;
   buffer->m_verbose=a.verbose;
   
   return *this;
 }

 
 Logging& operator<<(std::ostream& (*foo)(std::ostream&)) { 
   
   std::cout.rdbuf(buffer);
   std::cout<<plain<<foo;//std::endl;
   
   return *this;
 }
 
 template<typename T>  Logging& operator<<(const T &a){
     
   std::cout.rdbuf(buffer);
   std::cout<<a;

   return *this;
   
 }

template<typename T>  Logging& operator<<(const T &a){
     
   std::cout.rdbuf(buffer);
   std::cout<<a;

   return *this;
   
 }

 
 protected:
 
 TFStreamBuf* buffer; ///< Stream buffer used to replace std::cout for redirection to coustom output.
 TFStreamBuf* errbuffer;

 
 };


#endif
