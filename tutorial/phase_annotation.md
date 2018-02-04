# Annotating Code Regions

Caliper provides a library to annotate different code regions.
Different APIs exist to create annotations, the simplest of which uses
macros.  There are macros for common cases, including annotating
functions, loops, iterations, and user-defined regions.

In the following simple example we annotate functions `main`, `foo`,
and `bar`, as well as a loop and its associated iteration counter.


```c
#include <string>
#include <caliper/cali.h>

void foo() {
  CALI_CXX_MARK_FUNCTION; // Mark the function. Exports "function"="foo"
  // ...
}

void bar() {
  CALI_CXX_MARK_FUNCTION;
  // ...
}

int main(int argc, const char* argv[])
{
  CALI_CXX_MARK_FUNCTION;

  CALI_MARK_BEGIN("init"); // Mark a user-defined code region. Exports "annotation"="init"
  int count = std::stoi(argv[1]);
  CALI_MARK_END("init");

  CALI_CXX_MARK_LOOP_BEGIN(myloop, ”mainloop”); // Mark a loop. Exports "loop"="mainloop"
  for (int i = 0; i < count; ++i) {
    CALI_CXX_MARK_LOOP_ITERATION(myloop, i); // Mark iteration. Exports "iteration#mainloop"=<i>
    foo();
    bar();
  }
  CALI_CXX_MARK_LOOP_END(myloop);
}
```

More advanced annotations may be created by invoking the low-level
API, documented here:
http://llnl.github.io/Caliper/AnnotationAPI.html

[Next - Configuring Caliper](https://github.com/LLNL/caliper-examples/blob/master/tutorial/configuration.md)
