#include "Tool.h"
#include <cxxabi.h>  // demangle

Tool::Tool(){
	m_tool_name = abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, nullptr);
}
