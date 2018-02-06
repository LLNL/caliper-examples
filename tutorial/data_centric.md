# Data-Centric Sample Profiling

Typical profiling tools profile the time spent in different instructions.  More
recent research has shown the benefits of analyzing performance metrics with
respect to application data, termed ``data-centric'' profiling 
[[Liu et al.](http://ieeexplore.ieee.org/document/6877461/), 
[Gimenez et al.](http://ieeexplore.ieee.org/document/7013001/)].

Caliper supports data-centric profiling using memory allocation annotations.
Any data collection services enabled in Caliper that produce data addresses 
can be automatically resolved to determine their containing allocation.
As a result, we can aggregate different metrics with respect to annotated 
allocations (and system allocations).

Mechanisms to sample accessed memory addresses vary by architecture. 
Because the memory address producing services are decoupled from resolving
their allocation containers, Caliper's data-centric profiling capabilities are
agnostic to the underlying hardware.

## Configuring Memory Access Sampling

The following example shows how to enable Intel-specific PEBS memory address
load sampling and collect the information necessary to generate data-centric profiles:

```
CALI_LIBPFM_EVENTS=MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD
CALI_LIBPFM_SAMPLE_PERIOD=10000
CALI_LIBPFM_PRECISE_IP=2
CALI_LIBPFM_CONFIG1=20
CALI_LIBPFM_SAMPLE_ATTRIBUTES=time,tid,cpu,addr,weight,data_src

CALI_ALLOC_RESOLVE_ADDRESSES=true

CALI_RECORDER_FILENAME=memory_access_samples.cali

CALI_SERVICES_ENABLE=libpfm,pthread,recorder,trace,symbollookup,callpath,alloc,instlookup,report
```

Here we take a sample every 10000 loads, sampling only accesses that take
longer than 20 cycles, using the required precise IP flag of 2 (from the Intel
PMU manual).
Each sample contains a timestamp, thread ID, CPU ID, data address, load latency
(weight), and data source (details about the memory hardware it was found in).
We also use the `recorder` service, which writes all the Caliper-collected data in a format
that can be queried directly post-mortem, using the `cali-query` tool.

## Generating Profiles

We generate a data-centric profile using hte following query:

```
cali-query -q "SELECT alloc.label#libpfm.addr,statistics(libpfm.weight),count() GROUP BY alloc.label#libpfm.addr FORMAT table ORDER BY libpfm.weight DESC" memory_access_samples.cali
```

Output:

```
alloc.label#libpfm.addr libpfm.weight min#libpfm.weight max#libpfm.weight avg#libpfm.weight count
                              2119070                21             11302         80.231334 43221
nodelist                         6481                21               241         51.436508   126
y                                3866                22               321         71.592593    54
xd                               3700                21               367         86.046512    43
x                                3606                21               336         66.777778    54
yd                               3129                21               298        104.300000    30
delv_zeta                        2761                21               324        145.315789    19
z                                2530                21               312         58.837209    43
zd                               2423                21               305         67.305556    36
delv_eta                         2357                37               332        147.312500    16
ql                               1885                21               471        209.444444     9
delx_xi                          1727                60               310        215.875000     8
arealg                           1286               112               419        257.200000     5
q                                1223                22               455        174.714286     7
delx_eta                          992                70               337        141.714286     7
v                                 877                22               204         54.812500    16
delx_zeta                         808                22               309        134.666667     6
ss                                807                24               327        161.400000     5
```

Unresolved accesses account for the majority of the samples.
Second, the accesses to the variable `nodelist`, used for indirection in
LULESH, accounted for more total load latency cycles than any other, on average
was relatively quick.
`yd` and `delv_zeta` had both high total latency and high average latency,
making them potential candidates for optimization.

We can construct additional queries to generate profiles over code, instruction types, and memory hardware resources.
Some more examples:

```
cali-query -q "SELECT *,statistics(libpfm.weight) GROUP BY libpfm.memory_level FORMAT table ORDER BY libpfm.memory_level" memory_access_samples.cali
libpfm.memory_level avg#libpfm.weight sum#libpfm.weight min#libpfm.weight max#libpfm.weight count#libpfm.weight
L1                          53.674037           1298697                21             11044               24196
L2                          50.887324             21678                21               586                 426
L3                         291.143594            338600                23             11302                1163
LFB                        358.657868            353278                21              4445                 985
Local RAM                  347.500000               695               341               354                   2
Remote Cache 1 Hops        658.070796            148724               215              7513                 226
Remote RAM 1 Hop           811.833333              4871               334              1956                   6
Uncached Memory             33.000000                33                33                33                   1
```

Here we can see a breakdown of memory access latencies in terms of the cache hierarchy.
Notably, accesses to the line-fill buffer (LFB) are expensive, indicating lots of time spent waiting for data to reach L1.

If we group by both memory level and allocation label, we can determine which data structures resided in which caches:

```
cali-query -q "SELECT *,count() GROUP BY libpfm.memory_level,alloc.label#libpfm.addr FORMAT table ORDER BY count,libpfm.memory_level" memory_access_samples.cali
count libpfm.memory_level alloc.label#libpfm.addr
...
   12 L3                  delv_eta
   12 L3                  elemMass
   13 L3                  delv_zeta
   16 L3                  yd
   23 L3                  zd
   25 L3                  nodelist
   29 L3                  xd
   30 L3                  z
   35 L3                  y
   44 L3                  x
  840 L3
    1 LFB                 vdov
    1 LFB                 delv
    1 LFB                 ql
    1 LFB                 x
    1 LFB                 arealg
    1 LFB                 fz
    1 LFB                 ss
    1 LFB                 xd
    1 LFB                 qq
    2 LFB                 yd
    3 LFB                 y
    3 LFB                 zd
    4 LFB                 v
   90 LFB                 nodelist
  874 LFB
...
```

We see that `nodelist` was typically found in the line-fill buffer, and
variables `x`, `y`, and `z` were periodically found in L3.

We can also generate JSON data using `cali-query` to perform more in-depth analysis
and create plots using Python:

```
cali-query -q "FORMAT json(quote-all)" memory_access_samples.cali > memory_access_samples.json
```

The following section shows example plots and analysis of this data:

[LULESH Memory Access Sampling Analysis](https://nbviewer.jupyter.org/github/llnl/caliper-examples/blob/master/jupyter_notebooks/LULESH%20Memory%20Load%20Sampling%20Analysis.ipynb)
