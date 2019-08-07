#include "MpiProfile.hpp"

#include <caliper/Caliper.h>
#include <caliper/SnapshotRecord.h>

using namespace cali;


MpiProfile::MpiProfile(const char* output)
    : ChannelController("mpifrac", 0, {
            { "CALI_CHANNEL_FLUSH_ON_EXIT",       "false" },
            { "CALI_CHANNEL_CONFIG_CHECK",        "false" },
            { "CALI_AGGREGATE_KEY",   "mpi.function" },
            { "CALI_EVENT_TRIGGER",   "mpi.function" },
            { "CALI_EVENT_ENABLE_SNAPSHOT_INFO",  "false" },
            { "CALI_SERVICES_ENABLE", "aggregate,event,mpi,mpireport,timestamp" },
            { "CALI_MPI_BLACKLIST",   "MPI_Wtime,MPI_Comm_rank,MPI_Comm_size"   },
            { "CALI_MPIREPORT_WRITE_ON_FINALIZE", "false" },
            { "CALI_TIMER_SNAPSHOT_DURATION",     "true"  },
            { "CALI_TIMER_INCLUSIVE_DURATION",    "false" },
            { "CALI_MPIREPORT_CONFIG",
                    "select mpi.function as Function"
                    "    ,min(sum#time.duration) as Min"
                    "    ,max(sum#time.duration) as Max"
                    "    ,avg(sum#time.duration) as Avg"
                    "    ,percent_total(sum#time.duration) as \"Total (%)\""
                    "  group by mpi.function"
                    "  format table" }
        })
{
    config()["CALI_MPIREPORT_FILENAME"] = output;
}

void
MpiProfile::flush()
{
    Caliper  c;
    Channel* chn = channel();

    c.flush_and_write(chn, nullptr);
    c.clear(chn);
}
