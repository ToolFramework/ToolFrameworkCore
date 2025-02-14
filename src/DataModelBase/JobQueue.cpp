#include <JobQueue.h>

using namespace ToolFramework;


QueueStats::QueueStats(){

  Clear();
  
}

void QueueStats::Clear(){

  submitted = 0;
  queued = 0;
 
}



JobQueue::JobQueue(){}

JobQueue::~JobQueue(){

  m_lock.lock();
  while(m_jobs.size()){
    delete m_jobs.front();
    m_jobs.front()=0;
    m_jobs.pop();
  }
 m_lock.unlock();

}

bool JobQueue::AddJob(Job* job){

  if(job!=0 && job->func!=0){
    job->m_complete=false;
    job->m_in_progress=false;
    job->m_failed=false;
    m_lock.lock();
    m_jobs.push(job);
    m_stats[job->m_id].submitted++;
    m_stats[job->m_id].queued++;
    m_lock.unlock();
    
    return true;
  }
  return false;
  
}

Job* JobQueue::GetJob(){

  m_lock.lock();
  if(!m_jobs.size()){
    m_lock.unlock();
    return 0;
  } 
  Job* ret = m_jobs.front();
  m_jobs.front()=0;
  m_jobs.pop();
  m_stats[ret->m_id].queued--;
  m_lock.unlock();
  return ret;
}

bool JobQueue::pop(){

  m_lock.lock();
  if(m_jobs.size()){
    m_stats[m_jobs.front()->m_id].queued--;
     m_jobs.pop();
     m_lock.unlock();
    return true;
  }
  m_lock.unlock();
  return false;
}

unsigned int JobQueue::size(){

  m_lock.lock();
  unsigned int tmp= m_jobs.size();
  m_lock.unlock(); 
  return tmp; 

}

void JobQueue::Print(){

  m_lock.lock();
  printf("Total jobs queued = %u\n", size());
  for(std::map<std::string, QueueStats>::iterator it = m_stats.begin(); it!=m_stats.end(); it++){
    printf("  %s : submitted = %lu, queued = %lu \n", it->first.c_str(), it->second.submitted, it->second.queued);
  }
  m_lock.unlock();
  
}

void JobQueue::ClearStats(){

  m_lock.lock();
  m_stats.clear();
  m_lock.unlock();

}

void JobQueue::Clear(){

  m_lock.lock();
  while(m_jobs.size()){
    delete m_jobs.front();
    m_jobs.front()=0;
    m_jobs.pop();
  }
  m_stats.clear();
  m_lock.unlock();
  
}
