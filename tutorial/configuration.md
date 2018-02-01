# Caliper Configuration

By default, Caliper does not collect anything and remains relatively
unintrusive to the application execution.
Users must provide a Caliper configuration to enable data collection.

The configuration defines:
- what data to collect (time, counters, samples)
- how to process it (tracing, aggregating)
- output formats

Caliper may be configured in a number of ways:
- predefined configuration profiles
- environment variables
- configuration files
- through the runtime configuration API

Runtime functionality in Caliper is provided via ``services''.
Services provide measurement data or processing and data recording
capabilities. The flexible combination and configuration of these
services allows you to quickly assemble recording solutions for a wide
range of usage scenarios.  Detailed documentation is here:
http://llnl.github.io/Caliper/services.html

You can enable the services required for your measurement with the
`CALI_SERVICES_ENABLE` configuration variable, e.g.:

```
export CALI_SERVICES_ENABLE=event,recorder,trace
```

Simple examples are provided in the next section.

[Next - Configuring Runtime Profiling](runtime_profiling.md)
