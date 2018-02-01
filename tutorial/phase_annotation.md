# Annotating Code Regions

Caliper provides a library to annotate different code regions.
Different APIs exist to create annotations, the simplest of which uses
macros.  There are macros for common cases, including annotating
functions, loops, iterations, and user-defined regions.

In the following simple example we annotate functions `main`, `foo`,
and `bar`, as well as a loop and its associated iteration counter.

```c
#include <caliper/cali.h>

void foo() {
  CALI_CXX_MARK_FUNCTION;
  // ...
}

void bar() {
  CALI_CXX_MARK_FUNCTION;
  // ...
}

int main()
{
  CALI_CXX_MARK_FUNCTION;

  CALI_CXX_MARK_LOOP_BEGIN(myloop, ”loop”);
  for (int i = 0; i < 4; ++i) {
    CALI_CXX_MARK_LOOP_ITERATION(myloop, i);
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
