#ifndef TOOL_H
#define TOOL_H

#include <string>

#include "Store.h"
#include <string>
#include "Logging.h"
#include "DataModelBase.h"

class DataModel;

namespace ToolFramework{
   
  /**
   * \class Tool
   *
   * Abstract base class for Tools to inherit from. This allows a polymphic interface for the factor to use.
   *
   * $Author: B.Richards $
   * $Date: 2019/05/28 10:44:00 $
   */
  
  class Tool{
    
  public:
    
    Tool(){};
    virtual bool Initialise(std::string configfile,DataModel &data)=0; ///< virtual Initialise function that reads in the assigned config file and optain DataMoodel reference @param configfile Path and name of config file to read in. @param data Reference to DataModel. 
    virtual bool Execute()=0; ///< Virtual Execute function.
    virtual bool Finalise()=0; ///< Virtual Finalise function.
    virtual ~Tool(){}; ///< virtual destructor.
    std::string GetName() {return m_tool_name;};
    void SetName(std::string name) {m_tool_name=name;};
    
  protected:
    
    std::string m_tool_name;
    Store m_variables; ///< Store used to store configuration varaibles
    DataModel* m_data; ///< Pointer to transiant DataModel class
    Logging* m_log; ///< Pointer to logging class
    int m_verbose; ///< verbosity variable for direct logging level 
    MsgL ML(int messagelevel) {return MsgL(messagelevel,m_verbose);} ///< Function for setting logging level instream @param messagelevel the verboisty level at which to show the message. Checked against internal verbosity level.
    void MLC() {*(m_log)<<MsgL(0,m_verbose);}  ///< Function for clearing logging level
    template <typename T>  void Log(T message, int messagelevel, int verbosity){m_log->Log("-"+m_tool_name+"-: "+message,messagelevel,verbosity);}
    template <typename T>  void Log(T message, int messagelevel=0){m_log->Log("-"+m_tool_name+"-: "+message,messagelevel,m_verbose);}  ///< Logging fuction for printouts. @param message Templated message string. @param messagelevel The verbosity level at which to show the message. Checked against internal verbosity level
    void InitialiseTool(DataModel &data){m_data= &data; ///< Logging fuction to set the datamodel and and logging poitners @param messagelevel data DataModel reference;
      m_log=reinterpret_cast<DataModelBase*>(m_data)->Log;
    }
    bool InitialiseConfiguration(std::string configfile=""){ ///< Inisialisation of m_varaiables first from global vars and then local config file to overlaod. @param configfile configfile path.
      std::string vars_json="";
      if(reinterpret_cast<DataModelBase*>(m_data)->vars.Get(m_tool_name, vars_json)) m_variables.JsonParser(vars_json);
      if(configfile!="")  return m_variables.Initialise(configfile);
      return true;
    }
    void ExportConfiguration(){  ///< Export of local m_variables Store to matamodel vars for central exporting of configuration to data file.
      std::string vars_json="";
      m_variables>>vars_json;
      reinterpret_cast<DataModelBase*>(m_data)->vars.Set(m_tool_name,vars_json);
    }
    
  private:

  static const int v_error=0;
  static const int v_warning=1;
  static const int v_message=2;
  static const int v_debug=3;
    
    
    
    
  };
  
}

#endif
