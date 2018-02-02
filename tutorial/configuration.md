# Caliper Configuration

By default, Caliper does not collect anything and remains relatively
unintrusive to the application execution.
Users must provide a Caliper configuration to enable data collection.

The configuration defines:
- What data to collect (e.g. time, counters, samples)
- How to process it (tracing or aggregating)
- Output formats (e.g. Caliper raw data, JSON, or formatted human readable text)

Caliper may be configured via

- Environment variables
- Configuration files
- A runtime configuration API
- Pre-defined configuration profiles: Caliper comes with a few pre-defined
  configuration profiles for basic profiling use cases, e.g. "runtime-report" and
  "flat-function-profile".


## Services

Most of Caliper's runtime functionality is provided by individual
modules called "services". Services provide e.g. measurement data,
processing and data recording capabilities, or I/O functionality. They
can be flexibly combined to quickly assemble recording solutions for a
wide range of usage scenarios. Thus, selecting appropriate services is
the most important configuration decision. Typically, we will select at
least one service in each of the following groups to form a processing
pipeline:

- Data collection (e.g., timestamp, callpath, PAPI, MPI)
- Measurement trigger (e.g., event, sampler, libpfm sampling)
- Data buffering and processing (e.g. trace or aggregate)
- Output (e.g., report)

Caliper will warn if the service selection in the given configuration
does not cover all required groups, as no data can be recorded.

To enable services, we list them in the `CALI_SERVICES_ENABLE`
configuration variable, separated by comma.
This following minimal example uses the "event" service to record
begin and end events for each annotated region in the code, the
"trace" service to store the generated records in a trace buffer, and
the "recorder" service to write the trace out in Caliper's raw data
format:

    export CALI_SERVICES_ENABLE=event,recorder,trace

A complete list of services and their configuration is here:
http://llnl.github.io/Caliper/services.html


We will discuss a few simple examples in the next section.

[Next - Basic Runtime Profiling](https://github.com/LLNL/caliper-examples/blob/master/tutorial/runtime_profiling.md)

