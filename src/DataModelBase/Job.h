#ifndef JOB_H
#define JOB_H

#include <string>
#include <Pool.h>

namespace ToolFramework{

  class JobDeque;
  
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
    bool (*func)(void*&); ///< function for worker thread to run
    void (*fail_func)(void*&); ///< function for worker thread to run
    void* data = 0; ///< data packet for thread to retreive data
    bool m_complete; ///< if the job is complete
    bool m_in_progress; ///< if the job is in progress 
    bool m_failed; ///< if the job has failed
    std::string m_id; ///< string to hold id
    JobDeque* out_deque = 0; ///< output deque to place finished job
    Pool<Job>* out_pool = 0; ///< output pool to place finished jobs


  private:

    
  };  

}

#endif
  
