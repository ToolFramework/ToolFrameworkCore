#include "ToolChain.h"

ToolChain::ToolChain(std::string configfile,  int argc, char* argv[]){
  
  if(!m_data.vars.Initialise(configfile)){
    std::cout<<"\033[38;5;196m ERROR!!!: No valid config file quitting \033[0m"<<std::endl;
    exit(1);
  }

  if(!m_data.vars.Get("verbose",m_verbose)) m_verbose=9;
  if(!m_data.vars.Get("error_level",m_errorlevel)) m_errorlevel=2;
  if(!m_data.vars.Get("attempt_recover",m_recover)) m_recover=false;
  if(!m_data.vars.Get("log_mode",m_log_mode)) m_log_mode="Interactive";
  if(!m_data.vars.Get("log_local_path",m_log_local_path)) m_log_local_path="./log";
 
  if(!m_data.vars.Get("Inline",m_inline)) m_inline=0;
  if(!m_data.vars.Get("Interactive",m_interactive)) m_interactive=false;  

  m_data.vars.Set("argc",argc);

  for(int i=0; i<argc ; i++){
    std::stringstream tmp;
    tmp<<"$"<<i;
    m_data.vars.Set(tmp.str(),argv[i]);
  }

  Init();

  std::string toolsfile="";
  m_data.vars.Get("Tools_File",toolsfile);

  if(!LoadTools(toolsfile)) exit(1);

  if(m_inline!=0) Inline();                                                                           
  else if(m_interactive) Interactive();  
 
  
}

ToolChain::ToolChain(int verbose, int errorlevel, std::string logmode, std::string log_local_path){
  
  m_verbose=verbose;
  m_errorlevel=errorlevel;
  m_log_mode = logmode;
  m_log_local_path=log_local_path;

  Init();
  
}

void ToolChain::Init(){

  if(m_log_mode!="Off"){
  bcout=std::cout.rdbuf();
  out=new  std::ostream(bcout);
  }
  
  m_data.Log= new Logging(*out,m_log_mode, m_log_local_path);
  m_log=m_data.Log;  

  if(m_log_mode!="Off") std::cout.rdbuf(&(m_data.Log->buffer));

  execounter=0;
  Initialised=false;
  Finalised=true;
  paused=false;
  
  *m_log<<MsgL(1,m_verbose)<<yellow<<"********************************************************\n"<<"**** Tool chain created ****\n"<<"********************************************************\n"<<std::endl;
  
}



void ToolChain::Add(std::string name,Tool *tool,std::string configfile){
  if(tool!=0){


    *m_log<<MsgL(1,m_verbose)<<cyan<<"Adding Tool='"<<name<<"' to ToolChain"<<std::endl;
    
    m_tools.push_back(tool);
    m_toolnames.push_back(name);
    m_configfiles.push_back(configfile);

    *m_log<<MsgL(1,m_verbose)<<green<<"Tool='"<<name<<"' added successfully\n"<<std::endl;
    
  }
  else *m_log<<MsgL(0,m_verbose)<<red<<"WARNING!!! Tool='"<<name<<"' Does Not Exist in factory!!! \n"<<std::endl;  
  
}



int ToolChain::Initialise(){

  bool result=0;

  if (Finalised){

    *m_log<<MsgL(1,m_verbose)<<yellow<<"********************************************************\n"<<"**** Initialising tools in toolchain ****\n"<<"********************************************************\n"<<std::endl;

  
    for(int i=0 ; i<m_tools.size();i++){  

      *m_log<<MsgL(2,m_verbose)<<cyan<<"Initialising "<<m_toolnames.at(i)<<std::endl;
      

#ifndef DEBUG      
      try{ 
#endif   

	if(m_tools.at(i)->Initialise(m_configfiles.at(i), m_data))  *m_log<<MsgL(2,m_verbose)<<green<<m_toolnames.at(i)<<" initialised successfully\n"<<std::endl;
	else{
	  *m_log<<MsgL(0,m_verbose)<<red<<"WARNING !!!!! "<<m_toolnames.at(i)<<" Failed to initialise (exit error code)\n"<<std::endl;
	  result=1;
	  if(m_errorlevel>1) exit(1);
	}
	
#ifndef DEBUG	
      }      
      catch(std::exception& e){
        *m_log<<MsgL(0,m_verbose)<<red<<e.what()<<"\n"<<std::endl;
        throw;
      }
      catch(...){
	*m_log<<MsgL(0,m_verbose)<<red<<"WARNING !!!!! "<<m_toolnames.at(i)<<" Failed to initialise (uncaught error)\n"<<std::endl;
	result=2;
	if(m_errorlevel>0) exit(1);
       }
#endif
     
    }

    *m_log<<MsgL(1,m_verbose)<<yellow<<"**** Tool chain initialised ****\n"<<"********************************************************\n"<<std::endl;

    execounter=0;
    Initialised=true;
    Finalised=false;
    }
  else {

    *m_log<<MsgL(1,m_verbose)<<purple<<"********************************************************\n"<<" ERROR: ToolChain Cannot Be Initialised as already running. Finalise old chain first \n"<<"********************************************************\n"<<std::endl;
    

    result=-1;
  }
  
  return result;
}



int ToolChain::Execute(int repeates){
 
  int result =0;
  
  if(Initialised){

    bool skip=false;
    m_data.vars.Set("Skip",skip);

    if(m_inline)  *m_log<<MsgL(2,m_verbose)<<yellow<<"********************************************************\n"<<"**** Executing toolchain "<<repeates<<" times ****\n"<<"********************************************************\n"<<std::endl;

    for(int i=0;i<repeates;i++){
     
      *m_log<<MsgL(3,m_verbose)<<yellow<<"********************************************************\n"<<"**** Executing tools in toolchain ****\n"<<"********************************************************\n"<<std::endl;
      
      for(int i=0 ; i<m_tools.size();i++){

	m_data.vars.Get("Skip",skip); 
	if(skip){
	  skip=false;
	  m_data.vars.Set("Skip",skip);
	  *m_log<<MsgL(4,m_verbose)<<cyan<<"Skipping Remaining Tools"<<std::endl;
	  break;
	}

	*m_log<<MsgL(4,m_verbose)<<cyan<<"Executing "<<m_toolnames.at(i)<<std::endl;
	
	
#ifndef DEBUG
	try{
#endif
	  
	  if(m_tools.at(i)->Execute()) *m_log<<MsgL(4,m_verbose)<<green<<m_toolnames.at(i)<<" executed successfully\n"<<std::endl;
	   
	  else{
	    *m_log<<MsgL(0,m_verbose)<<red<<"WARNING !!!!!! "<<m_toolnames.at(i)<<" Failed to execute (error code)\n"<<std::endl;	    
	    result=1;
	    if(m_errorlevel>1){
	      if(m_recover){
		m_errorlevel=0;
		Finalise();
	      }
	      exit(1);
	    }
	  }  
	  
#ifndef DEBUG
	}
	
	catch(std::exception& e){
	  *m_log<<MsgL(0,m_verbose)<<red<<e.what()<<"\n"<<std::endl;
	  throw;
	}
	catch(...){
	  *m_log<<MsgL(0,m_verbose)<<red<<"WARNING !!!!!! "<<m_toolnames.at(i)<<" Failed to execute (uncaught error)\n"<<std::endl;
	  
	  result=2;
	  if(m_errorlevel>0){
	    if(m_recover){
	      m_errorlevel=0;
	      Finalise();
	    }
	    exit(1);
	  }
	}
#endif	
	
      }

      *m_log<<MsgL(3,m_verbose)<<yellow<<"**** Tool chain executed ****\n"<<"********************************************************\n"<<std::endl;
    }
    
    execounter++;
    if(m_inline) *m_log<<MsgL(2,m_verbose)<<yellow<<"********************************************************\n"<<"**** Executed toolchain "<<repeates<<" times ****\n"<<"********************************************************\n"<<std::endl;
    
  }
  
  else {
    *m_log<<MsgL(0,m_verbose)<<purple<<"********************************************************\n"<<" ERROR: ToolChain Cannot Be Executed As Has Not Been Initialised yet.\n"<<"********************************************************\n"<<std::endl;
    result=-1;
  }

  return result;
}



int ToolChain::Finalise(){
 
  int result=0;

  if(Initialised){
    *m_log<<MsgL(1,m_verbose)<<yellow<<"********************************************************\n"<<"**** Finalising tools in toolchain ****\n"<<"********************************************************\n"<<std::endl;

    for(int i=0 ; i<m_tools.size();i++){

      *m_log<<MsgL(2,m_verbose)<<cyan<<"Finalising "<<m_toolnames.at(i)<<std::endl;

#ifndef DEBUG
      try{
#endif

	if(m_tools.at(i)->Finalise()) *m_log<<MsgL(2,m_verbose)<<green<<m_toolnames.at(i)<<" Finalised successfully\n"<<std::endl;
	
	else{
	  *m_log<<MsgL(0,m_verbose)<<red<<"WARNING !!!!!!! "<<m_toolnames.at(i)<<" Finalised successfully (error code)\n"<<std::endl;	  
	  result=1;
	  if(m_errorlevel>1)exit(1);
	}  

#ifndef DEBUG      
      }
      
      catch(std::exception& e){
	*m_log<<MsgL(0,m_verbose)<<red<<e.what()<<"\n"<<std::endl;
	throw;
      }
      catch(...){
	*m_log<<MsgL(0,m_verbose)<<red<<"WARNING !!!!!!! "<<m_toolnames.at(i)<<" Finalised successfully (uncaught error)\n"<<std::endl;
	
	result=2;
	if(m_errorlevel>0)exit(1);
      }
#endif

    }

    *m_log<<MsgL(1,m_verbose)<<yellow<<"**** Toolchain Finalised ****\n"<<"********************************************************\n"<<std::endl;
    
    execounter=0;
    Initialised=false;
    Finalised=true;
    paused=false;
  }
  
  else { 
    *m_log<<MsgL(0,m_verbose)<<purple<<"********************************************************\n"<<std::endl<<" ERROR: ToolChain Cannot Be Finalised As Has Not Been Initialised yet."<<std::endl<<"********************************************************"<<std::endl;
    
    result=-1;
  }



  return result;
}


bool ToolChain::LoadTools(std::string filename){

  if(filename!=""){
    std::ifstream file(filename.c_str());
    std::string line;
    if(file.is_open()){
      
      while (getline(file,line)){
        if (line.size()>0){
          if (line.at(0)=='#')continue;
	  std::string name;
	  std::string tool;
	  std::string conf;
	  std::stringstream stream(line);
	  
          if(stream>>name>>tool>>conf) Add(name,Factory(tool),conf);
	 
        }
      }
      file.close();        
      return true;
    }
    else{
      *m_log<<MsgL(0,m_verbose)<<red<<"********************************************************\n"<<"**** ERROR!! Tools file does not exist on disk. No Tools loaded ****\n"<<"********************************************************\n"<<std::endl;
      return false;
     
    }
  }
  else{
    *m_log<<MsgL(0,m_verbose)<<red<<"********************************************************"<<std::endl<<"**** Warning!!! Tools file path is blank. No Tools Loaded ****"<<std::endl<<"********************************************************\n"<<std::endl;
    
    return false;
  }

}

void ToolChain::Inline(){

  if(m_inline==-1){
    bool StopLoop=false;
    m_data.vars.Set("StopLoop",StopLoop);
    Initialise();
    while(!StopLoop){
      Execute();
      m_data.vars.Get("StopLoop",StopLoop);
    }
    Finalise();
    
  }
  
  else if(m_inline>0){
    Initialise();
    Execute(m_inline);
    Finalise();
  }


}




void ToolChain::Interactive(){
  exeloop=false;
   
  ToolChainargs* args=new ToolChainargs;

  args->msgflag=&msgflag;
  args->command="";

  pthread_create (&thread[0], NULL, ToolChain::InteractiveThread, args);

  bool running=true; 
  
  msgflag=false;
 
  while (running){
    
    if(msgflag){  

      printf("%s \n\n",(ExecuteCommand(args->command)).c_str());
      if(args->command=="Quit"){
	running=false;
      }
      
      args->command="";

      std::stringstream tmp;
      tmp<<"Please type command : "<<cyan<<"Start, Pause, Unpause, Stop, Status, Quit, ?, (Initialise, Execute, Finalise)"<<plain;
      printf("%s \n %s",tmp.str().c_str(),">");
      
      msgflag=false;
      
    }
    
    ExecuteCommand("");
  }
  
  pthread_join(thread[0], NULL);
  delete args;
  args=0;
  
}  



std::string ToolChain::ExecuteCommand(std::string command){
  std::stringstream returnmsg;
  
  if(command!=""){
    if(command=="Initialise"){
      int ret=Initialise();
      if (ret==0)returnmsg<<"Initialising ToolChain";
      else returnmsg<<red<<"Error Code "<<ret<<plain;
    }
    else if (command=="Execute"){
      int ret=Execute();
      if (ret==0)returnmsg<<"Executing ToolChain";
      else returnmsg<<red<<"Error Code "<<ret<<plain;
    }
    else if (command=="Finalise"){
      exeloop=false;
      int ret=Finalise();
      if (ret==0)returnmsg<<"Finalising  ToolChain";
      else returnmsg<<red<<"Error Code "<<ret<<plain;
    }
    else if (command=="Quit"){
      returnmsg<<"Quitting";
    }
    else if (command=="Start"){
      int ret=Initialise();
      exeloop=true;
      execounter=0;
      if (ret==0)returnmsg<<"Starting ToolChain";
      else returnmsg<<red<<"Error Code "<<ret<<plain;
    }
    else if(command=="Pause"){
      exeloop=false;
      paused=true;
      returnmsg<<"Pausing ToolChain";
    }
    else if(command=="Unpause"){
      if(Initialised){
	exeloop=true;
	paused=false;
	returnmsg<<"Unpausing ToolChain";
      }
      else returnmsg<<red<<"Error Code "<<Execute()<<plain;
    }
    else if(command=="Stop") {
      exeloop=false;
      int ret=Finalise();
      if (ret==0)returnmsg<<"Stopping ToolChain";
      else returnmsg<<red<<"Error Code "<<ret<<plain;
    }
    
    else if(command=="Restart") {
      int ret=Finalise()+Initialise();
      if (ret==0)returnmsg<<"Restarting ToolChain";
      else returnmsg<<red<<"Error Code "<<ret<<plain;
    }
    
    else if(command=="Status"){
      std::stringstream tmp;
      if(Finalised) tmp<<"Waiting to Initialise ToolChain";
      if(Initialised && execounter==0) tmp<<"Initialised waiting to Execute ToolChain";
      if(Initialised && execounter>0){
	if(paused)tmp<<"ToolChain execution pasued";
	else tmp<<"ToolChain running (loop counter="<<execounter<<")";
      }
      
      if(*(m_data.vars["Status"])!="") tmp<<" : "<<*(m_data.vars["Status"]);
      returnmsg<<tmp.str();
    }
    else if(command=="?")returnmsg<<" Available commands: Initialise, Execute, Finalise, Start, Stop, Restart, Pause, Unpause, Quit, Status, ?";
    else if(command!=""){
      returnmsg<<purple<<"command not recognised please try again"<<plain;
    }
  }
  if(Finalised || (!Finalised && !exeloop)) usleep(100);
  if(exeloop) Execute();
  return returnmsg.str();
}


void* ToolChain::InteractiveThread(void* arg){

  ToolChainargs* args=static_cast<ToolChainargs*>(arg);

  bool running=true;
  
  printf("%s %s %s %s\n %s %s %s","Please type command :",cyan," Start, Pause, Unpause, Stop, Restart, Status, Quit, ?, (Initialise, Execute, Finalise)",plain,green,">",plain);

  while (running){
    
    std::cin>>args->command;
    *(args->msgflag)=true;
    if (args->command=="Quit")running=false;
  }
  
  pthread_exit(NULL);
  
}


ToolChain::~ToolChain(){
  
  for (int i=0;i<m_tools.size();i++){
    delete m_tools.at(i);
    m_tools.at(i)=0;
  }
  
  m_tools.clear();
  

  delete m_data.Log;  
  m_data.Log=0;

  if(m_log_mode!="Off"){
    std::cout.rdbuf(bcout);
    delete out;
    out=0;
  }  
  
  
  
}

