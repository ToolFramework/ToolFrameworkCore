#ifndef JOB_H
#define JOB_H

#include <string>

namespace ToolFramework{

/**
   * \class Job
   *
   * A class to store jobs for worker threads
   *
   * $Author: B.Richards $
   * $Date: 2024/06/08 1:17:00 $
   */

  
  class Job{
    
  public:
    
    Job(std::string id); ///< constructor with string to pass identification information for retreival 
    bool (*func)(void*); ///< function for worker thread to run
    void (*fail_func)(void*); ///< function for worker thread to run
    void* data; ///< data packet for thread to retreive data
    bool m_complete; ///< if the job is complete
    bool m_in_progress; ///< if the job is in progress 
    bool m_failed; ///< if the job ahs failed
    std::string m_id; ///< string to hold id


  private:

    
  };  

}

#endif
  
