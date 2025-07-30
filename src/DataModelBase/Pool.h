#ifndef POOL_H
#define POOL_H

#include <queue>
#include <mutex>
#include <stddef.h>
#include <cstdint>
#include <Utilities.h>
#include <chrono>

namespace ToolFramework{

  template<class T> struct Pool_args : Thread_args{
    
    Pool_args(){;}
    ~Pool_args(){;}
    size_t object_cap;
    uint16_t manage_period_ms;
    std::queue<T*>* objects;
    std::mutex* mtx;
    std::chrono::high_resolution_clock::time_point now;
    std::chrono::high_resolution_clock::time_point managing_timer;
    uint64_t count;
  };
  
  
  template<class T> class Pool{
    
  public:
    
    Pool(bool in_manage=false, uint16_t period_ms=1000, size_t in_object_cap=1){
      
      manage=in_manage;
      args.manage_period_ms=period_ms;
      args.object_cap=in_object_cap;
      
      if(manage) m_utils.CreateThread("pool_manager", &Thread, &args);
      
    }

    ~Pool(){
      if(manage) m_utils.KillThread(&args);
      Clear();
    }    
    
    template <typename... Args> T* GetNew(Args... in_args){
      mtx.lock();
      if(objects.size()>0){
	T* tmp=objects.front();
	objects.pop();
	mtx.unlock();
	return tmp;

      }
      mtx.unlock();
      return new T(in_args...);
    }
    
    void Add(T* object){
      mtx.lock();
      objects.push(object);
      mtx.unlock();
    }
    
    void Clear(){
      mtx.lock();
      while(!objects.empty()){
	delete objects.front();
	objects.pop();
      }
      mtx.unlock();
    }
    
    static void Thread(Thread_args* arg){
      
      Pool_args<T>* args = reinterpret_cast<Pool_args<T>*>(arg);
      args->count = std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - args->managing_timer).count();
      if(args->count < args->manage_period_ms){
	usleep((args->manage_period_ms - args->count)/1000);
	return;
      }
      args->mtx->lock();
      while(args->objects->size() > args->object_cap){
	delete args->objects->front();
	args->objects->pop();
      }
      args->mtx->unlock();
      args->managing_timer=std::chrono::high_resolution_clock::now();
      return;
    }    
    
  private:
    
    std::queue<T*> objects;
    std::mutex mtx;
    Utilities m_utils;
    Pool_args<T> args;
    bool manage;
    
  };
  
  
}

#endif
