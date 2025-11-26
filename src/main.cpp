#include <string>
//#include "mem.hpp"
#include "ToolChain.h"
#include "DataModel.h"
//#include "DummyTool.h"

using namespace ToolFramework;


int main(int argc, char* argv[]){

  //memory_usage.PrintUsage("init");
  std::string config_file;
  if (argc==1) config_file="configfiles/Dummy/ToolChainConfig";
  else config_file=argv[1];
  //memory_usage.PrintUsage("after config");

  
  DataModel* data_model = new DataModel();
  //memory_usage.PrintUsage("after DM");

  ToolChain tools(config_file, data_model, argc, argv);
  //memory_usage.PrintUsage("after tc");

  //DummyTool dummytool;    
  
  //tools.Add("DummyTool",&dummytool,"configfiles/DummyToolConfig");
  
  //int portnum=24000;
  //  tools.Remote(portnum);
  //tools.Interactive();
  
  //  tools.Initialise();
  // tools.Execute();
  //tools.Finalise();
  
  return 0;
  
}
