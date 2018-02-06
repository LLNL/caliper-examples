# Tracking Memory Allocations

The Caliper services `alloc` and `sysalloc` enable collection and
analysis of memory allocation information, including the active amount
of allocated memory (for annotated allocations only or for all
allocations), the lifetimes of different allocations, and details of
individual allocations (size, shape).

The `alloc` can record all allocation annotations as they occur, the
active amount of allocated bytes during different annotation events
(such as entering/exiting functions/phases), and enable resolution of
data addresses to allocation containers (covered in the next section).
This will only work for memory allocations explicitly annotated using
the Caliper API---in order to collect all allocations, the user must
enable the `sysalloc` service, which hooks all system allocation
calls.

The following configuration will report the number of allocations
(system or Caliper-defined) that occurred in each annotated function:

```
CALI_REPORT_CONFIG="SELECT function,count() GROUP BY function WHERE mem.alloc FORMAT tree" \
CALI_SERVICES_ENABLE=alloc,sysalloc,trace,report \
./apps/LULESH/build/lulesh2.0
```

Output:

```
== CALIPER: (0): Flushing Caliper data
== CALIPER: (0): Trace: Flushed 1331076 snapshots.
Path                                        count
function
main                                            94
  TimeIncrement                                931
  LagrangeLeapFrog
    LagrangeNodal                             3728
      CalcForceForNodes                       1005
        CalcVolumeForceForElems               5592
          CalcHourglassControlForElems        6524
            CalcFBHourglassForceForElems      4660
          IntegrateStressForElems             4660
    CalcTimeConstraintsForElems
      CalcHydroConstraintForElems            10254
      CalcCourantConstraintForElems          10254
    LagrangeElements                           932
      CalcQForElems                           8394
        CalcMonotonicQRegionForElems         10252
        CalcMonotonicQGradientsForElems        932
      CalcLagrangeElements                    3731
        CalcKinematicsForElems                 932
      UpdateVolumesForElems                    932
      ApplyMaterialPropertiesForElems         3728
        EvalEOSForElems                     186400
          CalcSoundSpeedForElems             10252
          CalcEnergyForElems                195720
            CalcPressureForElems            195720
```

## Potential Arenas

We can use this information to determine potential candidates for
memory arena allocation.
Allocation labels that are frequently re-allocated at different time
steps may benefit from reusing a single allocation.

We can determine the number of times the same allocation label was
re-allocated using the following configuration:

```
CALI_REPORT_CONFIG="SELECT mem.alloc,count() GROUP BY mem.alloc WHERE mem.alloc FORMAT table" \
CALI_SERVICES_ENABLE=alloc,sysalloc,trace,report \
./apps/LULESH/build/lulesh2.0
```

Output:

```
== CALIPER: (0): Flushing Caliper data
== CALIPER: (0): Trace: Flushed 1331076 snapshots.
mem.alloc count
malloc    657134
dxx          932
dyy          932
dzz          932
delx_xi      932
delx_eta     932
delx_zeta    932
delv_xi      932
delv_eta     932
delv_zeta    932
nodelist       1
lxim           1
lxip           1
letam          1
letap          1
...
```

Typically, arena allocators allocate multiple blocks of the same size
for multiple data structures to reuse.
We can determine how many arena allocators would suffice for our
annotated allocations by grouping the results by size:

```
CALI_REPORT_CONFIG="SELECT mem.alloc,alloc.total_size,count() GROUP BY mem.alloc,alloc.total_size WHERE mem.alloc FORMAT table" \
CALI_SERVICES_ENABLE=alloc,sysalloc,trace,report \
./apps/LULESH/build/lulesh2.0
```

Output:

```
== CALIPER: (0): Flushing Caliper data
== CALIPER: (0): Trace: Flushed 1331076 snapshots.
mem.alloc alloc.total_size count
malloc             1728000  11184
malloc              864000      2
nodelist            864000      1
malloc              259200      3
delv_xi             259200    932
delv_eta            259200    932
delv_zeta           259200    932
fx                  238328      1
fy                  238328      1
fz                  238328      1
nodalMass           238328      1
malloc              238328     13
x                   238328      1
y                   238328      1
z                   238328      1
xd                  238328      1
yd                  238328      1
zd                  238328      1
xdd                 238328      1
ydd                 238328      1
zdd                 238328      1
malloc              216000   4678
dxx                 216000    932
dyy                 216000    932
dzz                 216000    932
delx_xi             216000    932
delx_eta            216000    932
delx_zeta           216000    932
...
```

We see that many of our allocations have the same sizes, including
labeled allocations that are made 932 times, once for each iteration
of the run.

## Python-based Analysis

Next, we refine our analysis further by outputting the raw data and
interactively analyzing it in a Jupyter notebook using python.

Caliper supports a variety of machine-readable output formats that we
can use to analyze raw data using external data analysis tools.
Caliper's query language, CalQL, supports outputting to JSON, which
can be readily parsed from different Python libraries.  Here, we show
how to parse and analyze LULESH memory allocation data in Jupyter,
using Python, Pandas, and Plotly:

[LULESH Memory Allocation Analysis](https://nbviewer.jupyter.org/github/llnl/caliper-examples/blob/master/jupyter_notebooks/LULESH%20Memory%20Allocation%20Analysis.ipynb)
