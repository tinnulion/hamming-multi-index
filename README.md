This is fast and memory-efficient spatial index for Hamming space vectors based on multi-indices. Detailed desctription of theoretical background could be found here: http://www.cs.toronto.edu/~norouzi/research/papers/multi_index_hashing.pdf

Index is able to use POPCNT insctuctions from AMD SSE4A or Intel SSE 4.2 sets.

The index was tested on Athlon II X2 220 (quite old and slow CPU) with 16 GB of RAM. For 256-bit strings main characteristics of the index are following:

1) Index consumes approximately 108 bytes per item, 11 GB of memory per 100M items.

2) Query time for range = 0.1 is less than 160 ms for 100M index (using one thread).

3) Linear scan (brute-force approach) takes 3500 ms in the same circumstances (more than x20 slower).

Index will be great to use with binary local feature descriptors like BRIEF, BRISK, FREAK et al.

The project is few header files, no dependencies except standard library, so it`s quite easy to integrate.

Update 17/04/2014 : C# wrapper was added to the project.
