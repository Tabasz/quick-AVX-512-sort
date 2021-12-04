Maciej Hołubowicz @znirzej
Jan Tabaszewski   @Tabasz

## AVX-512 sorting in C++

This repository contains the code of our master thesis at the University of Warsaw: "Optimization of the fastest available primitive type sorting algorithm using AVX-512."

We have adapted the code from ''Fast and Robust Vectorized In-Place Sorting of Primi-tive Type'', Blacher, Giesen, Kühne, SEA 2021 (https://drops.dagstuhl.de/opus/volltexte/2021/13775/pdf/LIPIcs-SEA-2021-3.pdf) to AVX512 and further optimized it.
We believe that at the time of the publication of the code, it is the fastest open source implementation of 32-bit integers for Intel CPUs

Together with the code we provide tools used for automatic generation of it and tools for testing and comparing it's performance with other open source high performance sorting implementations.

## Requirements

You only need g++ to compile the code (we used version 9.4.0). We use the '-march=native' flag to compile on Intel(R) Xeon(R) Gold 6226R supporting AVX-512. To compile not natively, the minimum subset of flags required is '-mavx512f -mavx512vl'

## Running and testing

There are multiple testing scenarios defined in Makefile, most important one is
```make test1_512 && ./bin/test1_512```

## Automatic code generation
Sorting network code has been generated automatically. The files responsible are:
utils/irregular_networks.py - generates column sorting by irregular networks
utils/network_generator.cpp - generates column sorting by the Odd-even networks as well as column merging by a modified Odd-even network.