#ifndef BUFFER_DISPATCHER_H
#define BUFFER_DISPATCHER_H

#include <atomic>
#include <vector>
#include <Utilities.h>
#include <JobQueue.h>
#include <Pool.h>
#include <Buffer.h>
#include <AlgorithmWrapper.h>
#include <memory>
#include <mutex>

namespace ToolFramework{

   template<class T> struct BufferDispatcher_args:Thread_args{
     BufferDispatcher_args(){;}
     ~BufferDispatcher_args(){;}
      
      Buffer<T>* buffer = 0;
      std::vector<T> local_buffer; 
      std::vector<AlgorithmWrapper<T> >* algorithms = 0;
      std::mutex* algorithms_mtx = 0;
      Job* job = 0;

      JobQueue* job_queue = 0;      
      Pool<Job>* job_pool = 0;
      
      std::atomic<uint64_t>* counter = 0;
      
    };  

    template<class T> class BufferDispatcher{
      
    public:
      
      BufferDispatcher(){;}
      ~BufferDispatcher(){Close();}
      bool Init(Buffer<T>* buffer, std::vector<AlgorithmWrapper<T> >* algorithms, JobQueue* job_queue, Pool<Job>* job_pool, std::mutex* algorithms_mtx = 0){

	args.buffer = buffer;
	args.algorithms = algorithms;
	args.algorithms_mtx = algorithms_mtx;
	args.job_queue = job_queue;
	args.job_pool = job_pool;
	counter = 0;
	args.counter = &counter;

	if(buffer == 0 || algorithms == 0 || job_queue == 0 || job_pool == 0) return false; 	
		
	m_util.CreateThread("BufferDispatcher", &Thread, &args);

	return true;
      }
      
      void Close(){
	if(args.buffer == 0 || args.algorithms == 0 || args.job_queue == 0 || args.job_pool == 0) return;
	m_util.KillThread(&args);
	
	args.buffer = 0;
	args.algorithms = 0;
	args.algorithms_mtx = 0;
	args.job_queue = 0;
	args.job_pool = 0;
	counter = 0;
	
      }

      void GetStats(Store* store, std::string prefix, std::mutex* mtx=0){

	uint64_t counter_value = counter;
	std::unique_ptr<std::lock_guard<std::mutex> >lock;
	if(mtx!=0)  lock.reset(new std::lock_guard<std::mutex>(*mtx));
	store->Set(prefix+"_dispatch_counter", counter_value); 

      }
      
      std::atomic<uint64_t> counter;
      
      
    private:
      
      static void Thread(Thread_args* arg){  
	BufferDispatcher_args<T>* args=reinterpret_cast<BufferDispatcher_args<T>*>(arg);

	std::unique_ptr<std::unique_lock<std::mutex> >lock;
	if(args->algorithms_mtx!=0)  lock.reset(new std::unique_lock<std::mutex>(*args->algorithms_mtx));
 
	//std::unique_lock<std::mutex> lock(*args->algorithms_mtx);
	if(args->algorithms->size()==0){
	  usleep(100);
	  if(lock) lock->unlock();
	  return;	
	}
	if(lock) lock->unlock();

	args->buffer->Swap(args->local_buffer);
	
	if(args->local_buffer.size() == 0){
	  usleep(100);
	  return;
	}

	if(lock) lock->lock();
	for(size_t i = 0; i < args->local_buffer.size(); i++){
	  
	  for(size_t j = 0; j < args->algorithms->size(); j++){
	    
	    args->job = args->job_pool->GetNew(args->algorithms->at(j).name);
	    args->job->m_id = args->algorithms->at(j).name;
	    args->job->func = args->algorithms->at(j).algo;
	    args->job->fail_func = args->algorithms->at(j).fail_func;
	    args->job->data = args->algorithms->at(j).setup_func(args->local_buffer.at(i));
	    args->job->out_pool = args->job_pool;
	    
	    args->job_queue->AddJob(args->job);
	    args->job = 0;
	    (*args->counter)++;
	  }
	  
	  args->local_buffer.at(i) = 0;
	  
	}
	if(lock) lock->unlock();
	
	args->local_buffer.clear();
	
	return;	
      }    
    
      BufferDispatcher_args<T> args;
      Utilities m_util;
    
    
    };
    
}

#endif
