# Annotating Memory Allocations 

We can perform data-centric analysis by annotating memory allocations,
which Caliper may then use to attribute collected performance data to
them.

Memory allocation annotations are similar to code region annotations,
we can define labels for allocations using macros.
We can either label 1-dimensional ranges of bytes using
`CALI_DATATRACKER_TRACK` or multi-dimensional ranges of specified
element sizes using `CALI_DATATRACKER_TRACK_DIMENSIONAL`.
The following example shows both:

```c
void do_work(size_t M, size_t W, size_t N)
{
    double *arrayA = (double*)malloc(N);
    CALI_DATATRACKER_TRACK(arrayA, N);

    double *matA =
	 (double*)malloc(sizeof(double)*M*W);

    size_t num_dimensions = 2;
    size_t A_dims[] = {M,W};
    CALI_DATATRACKER_TRACK_DIMENSIONAL(
		matA,
		sizeof(double),
		A_dims,
		num_dimensions);
	...

    CALI_DATATRACKER_FREE(arrayA);
    CALI_DATATRACKER_FREE(matA);
}
```

More advanced memory allocation annotations may be created by invoking
the low-level API, documented here:
http://llnl.github.io/Caliper/AnnotationAPI.html

[Next - Tracking Memory Allocations](https://github.com/LLNL/caliper-examples/blob/master/tutorial/memory_allocations.md)
