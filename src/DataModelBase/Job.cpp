#include <Job.h>

using namespace ToolFramework;

Job::Job(std::string id){
  m_id = id;
  data = 0;
  m_complete = false;
  m_in_progress = false;
  m_failed = false;
  func = 0;
  fail_func = 0;
}
