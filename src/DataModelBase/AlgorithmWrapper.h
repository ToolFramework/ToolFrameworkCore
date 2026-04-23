#ifndef ALGORITHM_WRAPPER_H
#define ALGORITHM_WRAPPER_H

#include <string>
#include <functional>

namespace ToolFramework{
  /**
   * \struct Trigger_struct
   *
   * This tricts acts as a generator for trigger jobs with fucntionpointers
   */
  
  template<class T> struct AlgorithmWrapper{
    
    AlgorithmWrapper(std::string in_name, bool (*in_algo)(void*&), std::function<void*(T)> in_setup_func,  void (*in_fail_func)(void*&)){
      name = in_name;
      algo = in_algo;
      setup_func = in_setup_func;
      fail_func = in_fail_func;
      
    }
    std::string name; ///< name of algorihtm
    bool (*algo)(void*&); ///< algorithm to run on data
    void (*fail_func)(void*&); ///< fail funciton if algroithm fails
    std::function<void*(T)> setup_func; ///< setup function to create arguments 
    
  };
}
#endif
