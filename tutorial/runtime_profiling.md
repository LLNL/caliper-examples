# Basic Runtime Profiling

Caliper enables basic runtime profiling over annotated regions.
Here we show how to do so for the provided Caliper-annotated version of LULESH.

The simplest way to get started is with the predefined configuration profiles
by setting `CALI_CONFIG_PROFILE`.  
The following command will run LULESH while timing annotated functions and
print out a flat profile at the end of the execution:

```
CALI_CONFIG_PROFILE=flat-function-profile apps/LULESH/build/lulesh2.0
```

Output:

```
== CALIPER: (0): Flushing Caliper data
== CALIPER: (0): Trace: Flushed 436180 snapshots.
event.end#function              time.inclusive.duration
main                                           27883079
LagrangeLeapFrog                               27809407
LagrangeElements                               19117525
ApplyMaterialPropertiesForElems                16280433
EvalEOSForElems                                16161084
CalcEnergyForElems                             13360624
LagrangeNodal                                   7853608
CalcForceForNodes                               7054980
CalcVolumeForceForElems                         6995804
CalcPressureForElems                            6994081
CalcHourglassControlForElems                    5098672
CalcFBHourglassForceForElems                    1997461
CalcQForElems                                   1833767
IntegrateStressForElems                         1803895
CalcLagrangeElements                             955062
CalcTimeConstraintsForElems                      829378
CalcKinematicsForElems                           611723
CalcMonotonicQRegionForElems                     520508
CalcCourantConstraintForElems                    396165
CalcHydroConstraintForElems                      383921
CalcSoundSpeedForElems                           368622
CalcMonotonicQGradientsForElems                  350442
UpdateVolumesForElems                             36196
TimeIncrement                                      8730
```

A similar configuration can be explicitly defined with the following environment variables:

```
export CALI_SERVICES_ENABLE=event,report,timestamp,trace
export CALI_REPORT_CONFIG="SELECT event.end#function,sum(time.inclusive.duration) GROUP BY event.end#function FORMAT table ORDER BY time.inclusive.duration"
```

We can also generate a hierarchical profile of the nested annotations, while performing aggregation at runtime, with the following configuration:

```
export CALI_SERVICES_ENABLE=aggregate,event,report,timestamp
export CALI_AGGREGATE_KEY=function,loop
export CALI_TIMER_SNAPSHOT_DURATION=true
export CALI_REPORT_CONFIG=“select sum#time.duration format tree”
```

This will produce the following output from LULESH:

```
== CALIPER: (0): Flushing Caliper data
== CALIPER: (0): Aggregate: flushed 26 snapshots.
Path                                         sum#time.duration
main                                              26781.000000
  lulesh.cycle                                    10350.000000
    LagrangeLeapFrog                              10678.000000
      CalcTimeConstraintsForElems                 55943.000000
        CalcHydroConstraintForElems              361560.000000
        CalcCourantConstraintForElems            399618.000000
      LagrangeNodal                             1007553.000000
        CalcForceForNodes                         56070.000000
          CalcVolumeForceForElems                 95950.000000
            CalcHourglassControlForElems        3076597.000000
              CalcFBHourglassForceForElems      1959705.000000
            IntegrateStressForElems             1760854.000000
      LagrangeElements                            14211.000000
        UpdateVolumesForElems                     34233.000000
        CalcQForElems                            972758.000000
          CalcMonotonicQRegionForElems           454110.000000
          CalcMonotonicQGradientsForElems        338146.000000
        CalcLagrangeElements                     326398.000000
          CalcKinematicsForElems                 603887.000000
        ApplyMaterialPropertiesForElems          128076.000000
          EvalEOSForElems                       2045860.000000
            CalcSoundSpeedForElems               317678.000000
            CalcEnergyForElems                  5392462.000000
              CalcPressureForElems              5835381.000000
    TimeIncrement                                  8035.000000
```

In the next section, we will show how to annotate memory allocations for
data-centric analysis.

[Next - Annotating Memory Allocations](memory_annotations.md)
