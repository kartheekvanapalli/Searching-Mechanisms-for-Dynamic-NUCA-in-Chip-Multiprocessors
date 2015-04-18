# Searching Mechanisms for Dynamic NUCA in Chip Multiprocessors - HKState-NUCA (Home Knows where to find data and its state within the NUCA cache)
#Abstract
Rapid growth in the cache sizes of Chip Multiprocessors (CMPs) will lead to increase in wire-
delays and unexpected access latencies. Non-uniform access latencies to on-chip caches led to
Non-Uniform Cache Architecture (NUCA) design, but to get data proximity features we need
to map data blocks dynamically to banks and migration of data blocks. Because of dynamic
mapping and migration schemes employed by D-NUCA we can’t keep track of data blocks.
In D-NUCA mapping and migration schemes, increasing data proximity which reduces access
latencies, but if we don’t have good searching mechanism for a data block in NUCA cache we
strain ourselves in searching whole cache in case of a miss. So in order to get maximum benefit
from D-NUCA we need an efficient searching mechanism.

In my paper i discussed existing searching mechanisms and highlighted overheads and
limitations to those algorithms. I proposed a novel searching algorithm for D-NUCA designs
in CMPs, which is called HKState-NUCA (Home Knows where to find data and its state
within the NUCA cache). This algorithm provides fast and power efficient access to data lines
than many of the existing searching mechanisms. We have shown that using HKState-NUCA
as data search mechanism in a D-NUCA design reduces search requests (hop count) about
50.89%, and achieves an average performance improvement of 7.04% compared to HK-NUCA
searching algorithm by introducing 0.8% space overhead.



# About the Simulator created for comparing my algorithm (HKState-NUCA) with other algorithms
I created a virtual standalone simulator called CmpSim in C, which is similar to some
prior simulators like SimpleSim , Multi2Sim , GEMS , CACTI etc. But CmpSim
doesn’t simulate data and instructions, it will take addresses specific to each core from different
benchmarks and produces hits, misses, replacements, hops count and block movements specific
to cache banks and cores.

CmpSim initially creates a cache grid of clusters, each cluster contains a cache bank of all the
banksets. In each cache bank contains its properties and sets, each set contains its ways, each
way contains a cache block and its properties. Here we have created Grid structures "images/cachestructure.png"
contains 2D Clusters, which contains pointers to Cluster structure. Cluster structure contains
2D Cache banks, which contains pointers to Cache bank structure.

Cache bank structure contains 1D Sets, which contains pointers to Set structure. Set struc-
ture contains 1D Ways, which contains pointers to Cache block structure. Here in every level
each structure has its own properties, thats the reason to create structures internally.In our
configuration we will compute bank set associativity based on the number of cores, so each
grid having n clusters, where n is double the number of cores (bankset associativity) and each
cluster having 8 cache banks.

In CmpSim we have created number of threads that are equal to number of cores by using
pthread library, here each thread will act as a core. We have generated the trace file for 6
Parsec benchmarks using GEMS simulator, the trace file contains core number and access
to an addresses. We have separated these addresses according to core number and we put them
in a file with core number, so that each core can access individually its own set of addresses
from its own file. In our experiment we have considered 16 cores, so we will run 16 threads
initially. Each thread will take an address from its own file and we will search for that address
in the cache. If the address is absent in the cache we will insert it into the cache, if it requires
any migrations or replacements we will do it. Here for every address at every thread (core) we
will run Linear Search, HK-NUCA search and HKState-NUCA for synchronous results. In all
these algorithms we are using different cache grids for each algorithm, So that we can get our
results for three algorithms in a single run.

Here we don’t have cache coherence protocols, so instead of that we are maintaining a hash
table and we will insert every address with respect to each core. If a core is accessing a address,
after taking the address from the core specific file we will insert it into hash table if it is already
existing, another core is accessing the same address so we will move to busy waiting state
(cpu polling). After this insertion no other core can access the address we are accessing, after
Implementing searching algorithms we will delete the address from the hash table and take
another address from to access, In this way we are dealing with coherence issue.

Because of threads’ parallelism we got some memory problems for shared variables (critical
section), we are using mutex locks to deal with critical section problems. Cache block is also a
critical section to deal with cache blocks we are using a busy state for every cache block, when
ever any core is accessing a cache block we will make the busy bit 1, so that no other core can
access that block for any other purpose like replacement, migration etc.
