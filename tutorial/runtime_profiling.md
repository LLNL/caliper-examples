# Basic Runtime Profiling

Caliper enables basic runtime profiling over annotated regions.
Here we show how to do so for the provided Caliper-annotated version of LULESH.

The simplest way to get started is with the predefined configuration profiles
by setting `CALI_CONFIG_PROFILE`.  
The following command will run LULESH while timing annotated functions and
print out a flat profile at the end of the execution:

```Text
CALI_CONFIG_PROFILE=flat-function-profile apps/LULESH/build/lulesh2.0
```

Output:

```Text
CalcFBHourglassForceForElems      12297947.000000 
CalcPressureForElems               9474974.000000 
CalcKinematicsForElems             8751233.000000 
IntegrateStressForElems            7267488.000000 
CalcMonotonicQGradientsForElems    5157426.000000 
CalcMonotonicQRegionForElems       3652755.000000 
LagrangeNodal                      2441737.000000 
CalcEnergyForElems                 2296179.000000 
CalcCourantConstraintForElems      1279482.000000 
CalcHydroConstraintForElems         870175.000000 
CalcSoundSpeedForElems              716388.000000 
CalcLagrangeElements                614706.000000 
CalcQForElems                       238462.000000 
UpdateVolumesForElems               189110.000000 
EvalEOSForElems                     117187.000000 
CalcHourglassControlForElems         22428.000000 
LagrangeElements                     12257.000000 
CalcVolumeForceForElems              11294.000000 
CalcTimeConstraintsForElems           7627.000000 
ApplyMaterialPropertiesForElems       7109.000000 
CalcForceForNodes                     6472.000000 
LagrangeLeapFrog                      5628.000000 
TimeIncrement                         5518.000000 
main                                    66.000000
```

A similar configuration can be explicitly defined with the following environment variables:

```
CALI_SERVICES_ENABLE=event,report,timestamp,trace
CALI_REPORT_CONFIG="SELECT event.end#function,sum(time.duration) GROUP BY event.end#function FORMAT table ORDER BY time.duration"
```

We can also generate a hierarchical profile of the nested annotations, while performing aggregation at runtime, with the following configuration:

```
CALI_SERVICES_ENABLE=aggregate,event,report,timestamp
CALI_AGGREGATE_KEY=function,loop
CALI_TIMER_SNAPSHOT_DURATION=true
CALI_REPORT_CONFIG="select sum#time.duration format tree"
```

This will produce the following output from LULESH:

```Text
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

[Next - Annotating Memory Allocations](https://github.com/LLNL/caliper-examples/blob/master/tutorial/memory_annotations.md)
