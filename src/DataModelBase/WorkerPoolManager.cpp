#include "WorkerPoolManager.h"

using namespace ToolFramework;

PoolWorker_args::PoolWorker_args() : Thread_args() {}

PoolWorker_args::~PoolWorker_args() {}

PoolManager_args::PoolManager_args() : Thread_args() {}

PoolManager_args::~PoolManager_args() {}

WorkerPoolManager::WorkerPoolManager(JobQueue& job_queue, unsigned int* thread_cap, unsigned int* global_thread_cap, unsigned int* global_thread_num, JobDeque* job_out_deque, bool self_serving, bool threaded, unsigned int thread_sleep_us, unsigned int thread_management_period_us, unsigned int job_assignment_period_us){

  m_util = new Utilities();
  
  m_job_queue = &job_queue;
  m_manager_args.job_queue=m_job_queue;
  m_threaded = threaded;
  m_manager_args.thread_cap = thread_cap;
  m_manager_args.global_thread_cap = global_thread_cap;
  m_manager_args.global_thread_num = global_thread_num;
  m_manager_args.job_out_deque = job_out_deque;
  m_manager_args.self_serving= self_serving;
  m_manager_args.thread_sleep_us = thread_sleep_us;
  m_manager_args.thread_management_period_us = thread_management_period_us;
  m_manager_args.job_assignment_period_us = job_assignment_period_us;
  m_manager_args.util = m_util;
  m_manager_args.thread_num = 0;
  m_manager_args.manage = false;
  m_manager_args.serve = false;
  m_manager_args.sleep = false;
  m_manager_args.sleep_us = ( m_manager_args.thread_management_period_us < m_manager_args.job_assignment_period_us ? m_manager_args.thread_management_period_us : m_manager_args.job_assignment_period_us );
  
  CreateWorkerThread(m_manager_args.args, m_manager_args.self_serving, m_manager_args.thread_sleep_us, m_manager_args.job_queue, m_manager_args.job_out_deque, m_manager_args.thread_num, m_util, global_thread_num);

  m_manager_args.free_threads = 1;
  if (m_threaded) CreateManagerThread();
  
}

WorkerPoolManager::~WorkerPoolManager() {

  m_util->KillThread(&m_manager_args);
  
  for (unsigned int i = 0; i < m_manager_args.args.size(); i++)
    m_util->KillThread(m_manager_args.args.at(i));

  m_manager_args.args.clear();

  delete m_util;
  m_util = 0;

}

void WorkerPoolManager::CreateManagerThread() {

  std::string tmp="TManager";
  m_util->CreateThread(tmp, &ManagerThread, &m_manager_args);

}


void WorkerPoolManager::CreateWorkerThread(std::vector<PoolWorker_args*>& in_args, bool &in_self_serving, unsigned int &in_thread_sleep_us, JobQueue* in_job_queue, JobDeque* in_job_out_deque,unsigned long &thread_num, Utilities* in_util, unsigned int* global_thread_num) {
  PoolWorker_args* tmparg = new PoolWorker_args();
  tmparg->busy = false;
  tmparg->thread_sleep_us = in_thread_sleep_us;
  tmparg->job = 0;
  tmparg->job_queue = 0;
  tmparg->job_out_deque = in_job_out_deque;
  if(in_self_serving) tmparg->job_queue=in_job_queue;
  tmparg->self_serving = in_self_serving;
  in_args.push_back(tmparg);
  std::stringstream tmp;
  tmp << "T" << thread_num;
  in_util->CreateThread(tmp.str(), &WorkerThread, in_args.at(in_args.size() - 1));
  thread_num++;
  if(global_thread_num) global_thread_num++;
}

void WorkerPoolManager::DeleteWorkerThread(unsigned int pos,  Utilities* in_util, std::vector<PoolWorker_args*> &in_args, unsigned int* global_thread_num) {
  in_util->KillThread(in_args.at(pos));
  delete in_args.at(pos);
  in_args.at(pos) = 0;
  in_args.erase(in_args.begin() + pos );
  if(global_thread_num) global_thread_num--;
}

void WorkerPoolManager::WorkerThread(Thread_args* arg) {
  PoolWorker_args* args = reinterpret_cast<PoolWorker_args*>(arg);

  if ((!args->busy && !args->self_serving) || (args->self_serving && !args->job_queue->size())) usleep(args->thread_sleep_us);
  else {
    if(args->self_serving){
      args->job=args->job_queue->GetJob();
      if(!args->job) return;
      args->job->m_in_progress=true;
      args->busy = true;
    }

    try{
      if(args->job->func(args->job->data)) args->job->m_complete=true;
      else args->job->m_failed=true;
    }
    catch(...){
      args->job->m_failed=true;
    }
    if (args->job_out_deque) {
      args->job_out_deque->push_back(args->job);
      args->job->m_in_progress=false;
    } else {
      delete args->job;
      args->job=0;
    }
    args->busy = false;   
  }
  
}

void WorkerPoolManager::ManagerThread(Thread_args* arg) {

  PoolManager_args* args = reinterpret_cast<PoolManager_args*>(arg);

  args->now = std::chrono::high_resolution_clock::now();
  args->manage = std::chrono::duration<double, std::micro>(args->now - args->managing_timer).count() > args->thread_management_period_us;
  args->sleep = !args->manage;

  if(!args->self_serving){
    args->serve = std::chrono::duration<double, std::micro>(args->now - args->serving_timer).count() > args->job_assignment_period_us;
    args->sleep = !args->serve && !args->manage;  
  }
  
  if (args->sleep){
    usleep(args->sleep_us);
    return;
  }
  
  if(args->serve){
   
    if (args->job_queue->size() > 0) {
      for (unsigned int i = 0; i < args->args.size(); i++) {
	if (!args->args.at(i)->busy && args->job_queue->size() > 0) {
	  args->args.at(i)->job = args->job_queue->GetJob();
	  args->args.at(i)->job->m_in_progress=true;
	  args->args.at(i)->busy = true;
        }
      }
    }
    args->serving_timer = std::chrono::high_resolution_clock::now();
  }
  
  if(args->manage){

    args->free_threads = 0;
    unsigned int last_free = 0;
    for (unsigned int i = 0; i < args->args.size(); i++) {
      if (!args->args.at(i)->busy) {
	args->free_threads++;
	last_free = i;
      }
    }
    
    if (args->free_threads < 1 && args->args.size()<(*(args->thread_cap)) && ( !args->global_thread_cap || (*(args->global_thread_num))<(*(args->global_thread_cap))  ) ) CreateWorkerThread(args->args, args->self_serving, args->thread_sleep_us, args->job_queue, args->job_out_deque, args->thread_num, args->util, args->global_thread_num);
    
    if (args->free_threads > 1) DeleteWorkerThread(last_free, args->util, args->args, args->global_thread_num);
    
    args->managing_timer = std::chrono::high_resolution_clock::now();
  }
  
}

void WorkerPoolManager::ManageWorkers() {

  if(m_threaded) return;

  ManagerThread(&m_manager_args);

}

unsigned int WorkerPoolManager::NumThreads() {

  return m_manager_args.args.size();

}
