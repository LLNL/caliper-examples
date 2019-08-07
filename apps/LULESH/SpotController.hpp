#pragma once

#include <caliper/ChannelController.h>

class SpotController : public cali::ChannelController
{
    bool m_use_mpi;
    
public:

    explicit SpotController(bool use_mpi = false);

    ~SpotController();

    void flush();
};
