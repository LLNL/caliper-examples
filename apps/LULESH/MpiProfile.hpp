/// \file MpiProfile.hpp
/// \brief MpiProfile class

#pragma once

#include <caliper/ChannelController.h>

class MpiProfile : public cali::ChannelController
{
public:
    
    explicit MpiProfile(const char* output = "stderr");

    void flush();

    ~MpiProfile()
    { }
};    
