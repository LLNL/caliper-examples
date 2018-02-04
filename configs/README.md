# Caliper example configurations

This directory contains a variety of example configuration files for
Caliper. To use a configuration file, point Caliper to it via the
``CALI_CONFIG_FILE`` environment variable, e.g.

    $ CALI_CONFIG_FILE=configs/allocs.conf apps/LULESH/build/lulesh2.0

Some files (e.g. ``sampling.conf``) contain multiple configuration
profiles. You can select one with the ``CALI_CONFIG_PROFILE``
variable, e.g.

    $ CALI_CONFIG_FILE=configs/sampling.conf CALI_CONFIG_PROFILE=flat-sample-report \
        apps/LULESH/build/lulesh2.0

For more information about configuring Caliper, see the
[tutorial](https://github.com/LLNL/caliper-examples/blob/master/tutorial/configuration.md).
