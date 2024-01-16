#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <pthread.h>
#include <time.h> 
#include <unistd.h>

#include "Tool.h"
#include "DataModelBase.h"
#include "Logging.h"
#include "Factory.h"
#include "Store.h"

class DataModel;

namespace ToolFramework{  
  
  /**
   * \struct ToolChainargs
   *
   * Simple struct to pass thread initalisation variables to Interactive ToolChain thread
   *
   * $Author: B.Richards $
   * $Date: 2019/05/28 10:44:00 $
   */
  
  struct ToolChainargs{
    
    ToolChainargs(){};
    
    bool *msgflag; ///< Message flag used to indiacte if a new interactive command has been submitted to the interactive thread and needs execution on the main thread. 
    std::string command;
    
  };
  
  /**
   * \class ToolChain
   *
   * This class holds a dynamic list of Tools which can be Initialised, Executed and Finalised to perform program operation. Large number of options in terms of run modes and setting can be assigned.
   *
   * $Author: B.Richards $
   * $Date: 2019/05/28 10:44:00 $
   */
  
  class ToolChain{
    
  public:
    
    ToolChain(){};
    ToolChain(std::string configfile, DataModel* data_model, int argc=0, char* argv[]=0); ///< Constructor that obtains all of the configuration varaibles from an input file. @param configfile The path and name of the config file to read configuration values from.
    
    /**
       Constructor with explicit configuration variables passed as arguments.
       @param verbose The verbosity level of the ToolChain. The higher the number the more explicit the print outs. 0 = silent apart from errors.
       @param errorlevel The behavior that occurs when the ToolChain encounters an error. 0 = do not exit for both handeled and unhandeled errors, 1 = exit on unhandeled errors only, 2 = exit on handeled and unhandeled errors.
       @param log_interactive sets the logging class to use standard output and error to screen
       @param log_local sets the logging class to redirect standard output and error to disk
       logmode Where log printouts should be forwarded too. "Interactive" = cout, "Remote" = send to a remote logging system, "local" = ouput logs to a file. 
       @param log_local_path The file path and name of where to store logs if in Local logging mode.
       @param log_split_files when loggign to local file whether to split standard output and standard error into differnt files
       @param in_data_model option to specify an external data modle for use in ToolChain.
    */
    ToolChain(int verbose, int errorlevel=0, bool log_interactive=true, bool log_local=false, std::string log_local_path="./log", bool log_split_files=false, DataModel* in_data_model=0); 
    //verbosity: true= print out status messages , false= print only error messages;
    //errorlevels: 0= do not exit; error 1= exit if unhandeled error ; exit 2= exit on handeled and unhandeled errors; 
    virtual ~ToolChain(); 
    bool Add(std::string name, Tool *tool,std::string configfile=""); ///< Add a Tool to the ToolChain. @param name The name used in logs when reffering to the Tool. @param tool A pointer to the tool to be added to the ToolChain. @param configfile The configuration file path and name to be passed to the Tool. 
    int Initialise(); ///< Initialise all Tools in the ToolChain sequentially.
    int Execute(int repeates=1); ///< Execute all Tools in the ToolChain sequentially. @param repeates How many times to run sequential Execute loop.
    int Finalise(); ///< Finalise all Tools in the ToolCahin sequentially.
    void Interactive(); ///< Start interactive thread to accept commands and run ToolChain in interactive mode.
    bool LoadTools(std::string filename);
    DataModelBase* m_data; ///< Direct access to transient data model class of the Tools in the ToolChain. This allows direct initialisation and copying of variables.
    
  protected:
    
    virtual void Init();
    void Inline();
    
    static  void *InteractiveThread(void* arg);
    std::string ExecuteCommand(std::string connand);
    
    //Tools configs and data
    std::vector<Tool*> m_tools;
    std::vector<std::string> m_toolnames;
    std::vector<std::string> m_configfiles;
    
    //conf variables
    int m_verbose;
    int m_errorlevel;
    bool m_log_interactive;
    bool m_log_local;
    bool m_log_split_files;
    std::string m_log_local_path;
    bool m_interactive;
    int m_inline;
    bool m_recover;
    
    //status variables
    bool exeloop;
    unsigned long execounter;
    bool Initialised;
    bool Finalised;
    bool paused;
    Logging* m_log;
    
    //socket coms and threading variables
    pthread_t thread[1];
    bool msgflag;
    
  };


}

#endif
