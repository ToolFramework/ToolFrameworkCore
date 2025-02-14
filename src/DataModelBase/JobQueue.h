#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <queue>
#include <mutex>
#include <Job.h>
#include <map>

namespace ToolFramework{

 /**
   * \struct QueueStats
   *
   * This is a struct to define stats measured for job types;
   *
   * $Author: B.Richards $
   * $Date: 2024/06/08 1:17:00 $
   */
  
  struct QueueStats{

    QueueStats();
    void Clear();
    unsigned long submitted;
    unsigned long queued;    

  };

  /**
   * \class JobQueue
   *
   * A class that is a queue of jobs for worker threads
   *
   * $Author: B.Richards $
   * $Date: 2024/06/08 1:17:00 $
   */

  
  class JobQueue{

    friend class WorkerPoolManager;
    
  public:
    
    JobQueue(); ///< simple constructor
    ~JobQueue(); ///< simple destructor
    
    bool AddJob(Job* job); ///< fucntion to adda  job to the queue @param job pointer to the job to add
    Job* GetJob(); ///< function to get job from the front of the queue, the function pops the job off the queue
    bool pop(); ///< function to pop a job off the front of the queue
    unsigned int size(); ///< function to return number of jobs in the queue
    void Print();
    void ClearStats();
    void Clear();
    
    
  private:
    
    std::queue<Job*> m_jobs;
    std::mutex m_lock;
    std::map<std::string, QueueStats> m_stats;
    
  };

}
  
#endif
