# Build and Link Caliper

An easy way to install Caliper is via
[spack](https://github.com/spack/spack). For the data-centric analysis
in this tutorial, we need the Github master version and enable dyninst
support:

    spack install caliper@master +dyninst

Instructions for building Caliper manually with its CMake build system
can be found here: http://llnl.github.io/Caliper/build.html

## Linking an application with Caliper

To use Caliper, we need to add some
[source-code annotations](phase_annotation.md) 
and link the Caliper runtime library to the target application. In
general, this is accomplished by linking ``libcaliper.so``:

    $(LINK) -o myapp $(OBJECTS) -L/caliper/installation/dir/lib64 -lcaliper

Caliper provides a CMake configuration script, so that
CMake-based applications can find Caliper using
``find_package(caliper)``. 
For an example, see the LULESH example application in this repository:
https://github.com/LLNL/caliper-examples/blob/master/apps/LULESH/CMakeLists.txt

To build LULESH, add the location of the Caliper CMake script to
``CMAKE_PREFIX_PATH`` for the configuration step:

    $ cd apps/LULESH; mkdir build; cd build
    $ export CALI_DIR=$(spack location --install-dir caliper)
    $ cmake -DCMAKE_PREFIX_PATH=$CALI_DIR/share/cmake/caliper ..

[Next - Annotating Code Regions](https://github.com/LLNL/caliper-examples/blob/master/tutorial/phase_annotation.md)
