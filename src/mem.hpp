#ifndef MEM_H
#define MEM_H

#include <cstdio>
#include <cstdlib>
#include <new>
#include <cstdio>
#include <map>

constexpr unsigned long catalogue_range = 60000;


struct MemoryUsage{
  
  ~MemoryUsage(){PrintUsage("Destructor");}
  uint32_t total_allocated = 0;
  uint32_t total_freed = 0;
  
  uint32_t CurrentUsage(){return total_allocated - total_freed;}
  
  void PrintUsage(std::string label=""){
    if(label!="") printf("(%s): ",label.c_str());
    printf("total_allocated = %u, total_freed = %u, current_usage = %u\n", total_allocated, total_freed, CurrentUsage());
  }
  
  void Clear(){
    total_allocated = 0;
    total_freed = 0;
  }
  void CatalogueAdd(void* ptr, size_t size){
    
    unsigned long i=0;
    for(; i<catalogue_range; i++){
      if(catalogue_ptr[i]==0){
	catalogue_ptr[i]=(unsigned long) ptr;
	catalogue_size[i]=size;
	break;
      }
    }
    if(i==catalogue_range) printf("INSUFFICENT CATALOGUE SIZE\n");
        
    
  }
  
  unsigned long CatalogueDel(void* ptr){
    unsigned long ret=0;
    unsigned long tmp_ptr=(unsigned long) ptr;
    unsigned long i=0;
    for(; i<catalogue_range; i++){
      if(catalogue_ptr[i]==tmp_ptr){
	ret = catalogue_size[i];
	catalogue_ptr[i]=0;
	catalogue_size[i]=0;
	break;
	
      }
    }
    if(i==catalogue_range) printf("NOT KNOWN IN CATALOGUE (probably trying to delete twice)\n");
      return ret;
    
  }
  
  unsigned long catalogue_ptr[catalogue_range];
  unsigned long catalogue_size[catalogue_range];
  
};

static MemoryUsage memory_usage;

/*  
    void* operator new(size_t size){
    printf("calling new\n");
    
    memory_usage.total_allocated += size;
    
    return malloc(size);
  }
  
  void operator delete (void* memory, size_t size){
    printf("calling delete\n");
    memory_usage.total_freed += size;
    
    free(memory);
  }

  void operator delete[] (void* memory, size_t size){
    printf("calling delete[]\n");
    memory_usage.total_freed += size;
    
    ::delete[](memory);
  }


void operator delete (void* memory){
  printf("calling delete no size\n");

    free(memory);
  }


void operator delete[] (void* memory){
  printf("calling delete[] no size\n");

  ::delete[] (memory);
  }
*/

 
// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz)
{
  memory_usage.total_allocated += sz;
  void* tmp= std::malloc(sz);
  //  std::printf("1) new(size_t), size = %zu p =%p\n", sz,tmp);
  memory_usage.CatalogueAdd(tmp,sz); 
  return tmp;
    /*
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
    */
}
 
// no inline, required by [replacement.functions]/3
void* operator new[](std::size_t sz)
{
  void* tmp = std::malloc(sz);
  memory_usage.CatalogueAdd(tmp,sz);
  //std::printf("2) new[](size_t), size = %zu, p = %p\n", sz, tmp);
  memory_usage.total_allocated += sz;
  return tmp;
  /*if (sz == 0)
    ++sz; // avoid std::malloc(0) which may return nullptr on success
    
    if (void *ptr = std::malloc(sz))
    return ptr;
    
    throw std::bad_alloc{}; // required by [new.delete.single]/3
  */
}
 
void operator delete(void* ptr) noexcept
{
  //  std::printf("3) delete(void*) p=%p\n",ptr);
  memory_usage.total_freed += memory_usage.CatalogueDel(ptr);
  
  
  std::free(ptr);
}
void operator delete(void* ptr, std::size_t size) noexcept;
void operator delete(void* ptr, std::size_t size) noexcept
{
  //std::printf("4) delete(void*, size_t), size = %zu\n", size);
    memory_usage.total_freed += size;
    memory_usage.CatalogueDel(ptr);   
    std::free(ptr);
}
 
void operator delete[](void* ptr) noexcept
{
  unsigned long size=memory_usage.CatalogueDel(ptr);
  //std::printf("5) delete[](void* ptr) p = %p, size = %u\n", ptr, size);
  memory_usage.total_freed += size;
  std::free(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept;
void operator delete[](void* ptr, std::size_t size) noexcept
{
  //std::printf("6) delete[](void*, size_t), size = %zu\n", size);
    memory_usage.total_freed += size;
    memory_usage.CatalogueDel(ptr);
    std::free(ptr);
}
 


#endif
