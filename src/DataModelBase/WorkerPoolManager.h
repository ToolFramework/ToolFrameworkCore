#ifndef WORKER_POOL_MANAGER_H
#define WORKER_POOL_MANAGER_H

#include <string>
#include <iostream>

#include <JobQueue.h>
#include <JobDeque.h>
#include <Utilities.h>
#include <mutex>
#include <chrono>
#include <atomic>

namespace ToolFramework{

  /**
   * \struct PoolManagerStats
   *
   * This is a struct to define stats measured for job types;
   *
   * $Author: B.Richards $
   * $Date: 2024/06/08 1:17:00 $
   */

  struct PoolManagerStats{

    PoolManagerStats();
    unsigned long processing;
    unsigned long completed;
    unsigned long failed;

  };
  
  /**
   * \struct PoolWorker_args
   *
   * This is a struct to place data you want your thread to acess or exchange with it. The idea is the data inside is only used by the thread and so will be thread safe
   *
   * $Author: B.Richards $
   * $Date: 2024/06/08 1:17:00 $
   */
  
  
  struct PoolWorker_args : Thread_args{
    
    PoolWorker_args();
    ~PoolWorker_args();
    bool busy;
    unsigned int thread_sleep_us;
    bool self_serving;
    Job* job;
    JobQueue* job_queue;
    JobDeque* job_out_deque;
    std::map<std::string,PoolManagerStats>* stats;
    std::mutex* stats_mtx;
  };
  
  
  /**
   * \struct PoolManager_args
   *
   * This is a struct to place data you want your thread to acess or exchange with it. The idea is the datainside is only used by the thread and so will be thread safe
   *
   * $Author: B.Richards $
   * $Date: 2024/06/08 1:17:00 $
   */
  
  struct PoolManager_args:Thread_args{

    PoolManager_args();
    ~PoolManager_args();

    JobQueue* job_queue;
    unsigned int* thread_cap;  ///< Variable to cap the number of thread workers
    unsigned int* global_thread_cap;
    std::atomic<unsigned int>* global_thread_num;
    JobDeque* job_out_deque;
    bool self_serving;
    unsigned int thread_sleep_us;
    unsigned int thread_management_period_us;
    unsigned int job_assignment_period_us;
    Utilities* util;
    std::vector<PoolWorker_args*> args; ///< Vector of thread args (also holds pointers to the threads)
    unsigned long thread_num; ///< Counter for unique naming of threads
    bool manage;
    bool serve;
    bool sleep;
    unsigned int sleep_us;
    unsigned int free_threads; ///< Keeps track of free threads
    std::chrono::high_resolution_clock::time_point now;
    std::chrono::high_resolution_clock::time_point managing_timer;
    std::chrono::high_resolution_clock::time_point serving_timer;
    std::map<std::string,PoolManagerStats> stats;
    std::mutex stats_mtx;
    
  };
  /**
   * \class WorkerPoolManager
   *
   * This is a template for a Tool that dynamically more or less threads, such that there is always 1 available thread.This can therefore be used to scale to your worklaod, however be carefull when using more than one of these tools and to apply upperlimits if necessary both locally within this tool and globally so that more threads than is practical are created causing massive inefficency. Please fill out the descripton and author information.
   *
   * $Author: B.Richards $
   * $Date: 2024/06/08 1:17:00 $
   */

  class WorkerPoolManager{
    
    
  public:

    /**
       * Cconstructor for the Worker Pool Manager
       * @param job_queue is the input job queue
       * @param thread_cap is the maximum number of worker threads that will be spawned
       * @param global_thread_cap pointer to an external global thread cap
       * @param global_thread_num pointer to external global thread counter to incriment
       * @param job_out_deque optional completed job deque if you want an output list structure
       * @param self_serving whether threads help them selves to jobs or alternativly the manager will distribute jobs
       * @param threaded if the manager runs on a thread. if not it can be executed manually multiple times
       * @param thread_sleep_us how long threds will sleep for in us if no job is available
       * @param thrad_management_period_us how long between evaluating the number of worker threads to avoid rapid killing and recreating
       * @param job_assignment_period how long the managers sleeps between checking if there are free workers to assign them new jobs
       */
    WorkerPoolManager(JobQueue& job_queue,  unsigned int* thread_cap=0, unsigned int* global_thread_cap=0, std::atomic<unsigned int>* global_thread_num=0, JobDeque* job_out_deque=0, bool self_serving=false, bool threaded=true, unsigned int thread_sleep_us=100, unsigned int thread_management_period_us=10000, unsigned int job_assignment_period_us=1000);
    ~WorkerPoolManager(); ///< Simple Destructor
    
    void ManageWorkers(); ///< Function to manage workers and distribute jobs to be run when unthreaded if you choose to not have the managment run on a thread.
    unsigned int NumThreads(); ///< Function to return the number of current worker threads
    std::string GetStats(); ///< Function to get the current stats
    void GetStats(Store& output); ///< Function to get the current stats
    void PrintStats(); ///< Function to print the current stats to screen
    void ClearStats(); ///< Function to clear the current stats
    
  private:
    
    void CreateManagerThread(); ///< Function to Create Manager Thread
    static void CreateWorkerThread(std::vector<PoolWorker_args*> &in_args, bool &in_self_serving, unsigned int &in_thread_sleep_us, JobQueue* in_job_queue, JobDeque* in_job_out_deque,unsigned long &thread_num, Utilities* in_util, std::map<std::string,PoolManagerStats>* in_stats, std::mutex* in_stats_mtx, std::atomic<unsigned int>* global_thread_num=0 ); ///< Function to Create Worker Thread
    static void DeleteWorkerThread(unsigned int pos,  Utilities* in_util, std::vector<PoolWorker_args*> &in_args, std::atomic<unsigned int>* global_thread_num=0); ///< Function to delete thread @param pos is the position in the args vector below
    
    static void WorkerThread(Thread_args* arg); ///< Function to be run by the thread in a loop. Make sure not to block in it
    static void ManagerThread(Thread_args* arg); ///< Function to be run by the thread manager. Make sure not to block in it
    
    JobQueue* m_job_queue; ///< Job queue to hold jobs
    JobDeque* m_job_out_deque; ///< Job deque to hold completed jobs
    Utilities* m_util; ///< Pointer to utilities class to help with threading
    PoolManager_args m_manager_args; ///< Thread args for manager
    bool m_threaded;
    
  };
  
}

#endif
