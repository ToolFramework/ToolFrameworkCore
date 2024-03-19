#include "Logging.h"

using namespace ToolFramework;

Logging::TFStreamBuf::~TFStreamBuf(){
  
  if(m_local && !no_delete) file.close();
 
  if(m_error){
    std::cerr.rdbuf(backup1);
    std::clog.rdbuf(backup2);
  }
  else std::cout.rdbuf(backup1);
  
  if(output){
    delete output;
    output=0;
  }
  if(fileoutput){
    if(!no_delete) delete fileoutput;
    fileoutput=0;
    
  }

}





Logging::TFStreamBuf::TFStreamBuf ( bool interactive, bool local, std::string localpath, bool error, std::ostream* filestream){

  output=0;
  fileoutput=0;

  m_local=local;
  m_interactive=interactive;
  m_error=error;


  if(m_error){
    m_messagelevel=0;
    m_verbose=0;
    backup1=std::cerr.rdbuf();
    backup2=std::clog.rdbuf();
  }
  else{
    m_messagelevel=1;
    m_verbose=1;
    backup1=std::cout.rdbuf();
    
  }
  
  if(m_local || m_interactive){
    
    if(m_local){
      if(!filestream){
	file.open(localpath.c_str());
	psbuf = file.rdbuf();
	fileoutput=new std::ostream(psbuf);
	no_delete=false;
      }
      else{
	fileoutput=filestream;
	no_delete=true;
      }
    }
    
    if(m_interactive) output=new std::ostream(backup1);
    
    if(m_error){
      std::cerr.rdbuf(this);
      std::clog.rdbuf(this);
    }
    else std::cout.rdbuf(this);
  }
  
}


int Logging::TFStreamBuf::sync ( )
{
  if( (( m_interactive || m_local) && (m_messagelevel <= m_verbose)) && str()!=""){
    
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
    std::string t(buffer);
    
    if(m_local){
      if(m_error)(*fileoutput)<<red;      
      (*fileoutput)<< "{"<<t<<"} [";
      if(m_error) (*fileoutput)<<"ERROR";
      else (*fileoutput)<<m_messagelevel;
      (*fileoutput)<<"]: " << str();
      if(m_error)(*fileoutput)<<plain;
      fileoutput->flush();
    }
    if(m_interactive){
      /*std::string code="";
	if(m_messagelevel ==0) code="231";
	else if (m_messagelevel ==1) code="174";
	else if (m_messagelevel ==2) code="129";
	else if (m_messagelevel ==3) code="154";
	else if (m_messagelevel ==4) code="159";
	else if (m_messagelevel ==5) code="125";
	else if (m_messagelevel ==6) code="21";
	else if (m_messagelevel ==7) code="46";
	else if (m_messagelevel ==8) code="51";
	else if (m_messagelevel ==9) code="93";
	else if (m_messagelevel ==10) code="208";
	else if (m_messagelevel ==10) code="226";
	else if (m_messagelevel >=12) code="201";
	
	
	output<<"\033[38;5;"<<code<<"m["<<m_messagelevel<<"]: " << str()<<"\033[0m";
      */ 
      if(m_error) (*output)<<red;
      (*output)<<"[";
      if(m_error) (*output)<<"ERROR";
      else (*output)<<m_messagelevel;
      (*output)<<"]: "<< str();
      if(m_error) (*output)<<plain;      
      output->flush(); 
    }
  }
  str("");

  
  
  return 0;
}

bool Logging::TFStreamBuf::ChangeOutFile(std::string localpath){
  
  if(m_local){
    file.close();
    file.open(localpath.c_str());
    psbuf = file.rdbuf();
    fileoutput->rdbuf(psbuf);
    return true;
  }
  
  return false;
  
}

 Logging::Logging(bool interactive, bool local,  std::string localpath, bool split_output_files){

   if(split_output_files){ 
     buffer=new TFStreamBuf(interactive, local, localpath+".o", false);
     errbuffer=new TFStreamBuf(interactive, local, localpath+".e", true);
   }
   else{
     buffer=new TFStreamBuf(interactive, local, localpath, false);
     errbuffer=new TFStreamBuf(interactive, local, localpath, true, buffer->fileoutput);
   }

}


Logging::~Logging(){
  
  delete buffer;
  buffer=0;

  delete errbuffer;
  errbuffer=0;  

}
