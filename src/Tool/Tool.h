#ifndef TOOL_H
#define TOOL_H

#include <string>

#include "Store.h"
#include <string>
#include "Logging.h"

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
    template <typename T>  void Log(T message, int messagelevel, int verbosity){m_log->Log("-"+GetName()+"-: "+message,messagelevel,verbosity);}
    template <typename T>  void Log(T message, int messagelevel=0){m_log->Log("-"+GetName()+"-: "+message,messagelevel,m_verbose);}  ///< Logging fuction for printouts. @param message Templated message string. @param messagelevel The verbosity level at which to show the message. Checked against internal verbosity level
    
    
  private:
    
    
    
    
    
  };
  
}

#endif
