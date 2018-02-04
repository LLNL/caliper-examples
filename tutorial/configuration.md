# Caliper Configuration

By default, Caliper does not collect anything and remains relatively
unintrusive to the application execution.  Users must provide a
Caliper configuration to enable data collection.  The configuration
defines what data to collect (e.g. time, counters, samples), how to
process it (tracing or aggregating), and how to format the output
(e.g. Caliper raw data, JSON, or formatted human readable text).

Caliper may be configured via

- Environment variables
- Configuration files
- A runtime configuration API
- Pre-defined configuration profiles: Caliper comes with a few pre-defined
  configuration profiles for basic profiling use cases, e.g. "runtime-report" and
  "flat-function-profile".

## Services

As most of Caliper's runtime functionality is provided by individual
modules called "services", the most important runtime configuration
setting is to select the right services. Services provide
e.g. measurement data, processing and data recording capabilities, or
I/O functionality. They can be flexibly combined to quickly assemble
recording solutions for a wide range of usage scenarios.

Typically, we will select at least one service in each of the
following groups to form a processing pipeline:

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

To learn more about how to combine services to set up a Caliper
recording pipeline, see http://llnl.github.io/Caliper/workflow.html.
A complete list of services and their configuration is here:
http://llnl.github.io/Caliper/services.html

## Configuration profiles

Caliper is configured through configuration variables of the form
``CALIPER_<MODULE>_<KEY>=value``.

We can put a group of configuration variables in a configuration
file. The
[configs directory](https://github.com/LLNL/caliper-examples/blob/master/configs)
in this repository has a few examples. Caliper will automatically
read ``caliper.config`` in the current working directory if it
exists, or we can point Caliper to a configuration file with the
``CALI_CONFIG_FILE`` option (this has to be set as an environment
variable):

    CALI_CONFIG_FILE=../configs/topdown_ivybridge.conf ./myapp

It is possible to put multiple named configuration profiles into the
same file - see http://llnl.github.io/Caliper/configuration.html for
details. It is also possible to configure Caliper entirely through
environment variables (we don't recommend that approach).

Finally, Caliper comes with a few built-in configuration profiles. We
will discuss a few simple example configurations in the next section.

[Next - Basic Runtime Profiling](https://github.com/LLNL/caliper-examples/blob/master/tutorial/runtime_profiling.md)
