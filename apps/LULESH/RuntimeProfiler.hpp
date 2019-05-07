/// \file RuntimeProfiler.hpp
/// \brief RuntimeProfiler class

#pragma once

#include <caliper/ChannelController.h>

class RuntimeProfiler : public cali::ChannelController
{
public:
    
    explicit RuntimeProfiler(bool use_mpi = false, const char* output = "stderr");

    ~RuntimeProfiler()
    { }
};    
