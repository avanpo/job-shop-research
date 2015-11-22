# Job Shop Research

A local search approach for solving a variant of the job shop problem, where the machines are replaced with a number of identical, parallel machines.

## Problem Definition

We are given:
* t machine types T\_1, ... , T\_t
  * machine type x has m\_x machines
* n jobs J\_1, ... , J\_n
  * job y has l operations O\_1, ... , O\_l
    * operation z has processing time p\_z, an idle time i\_z, a machine type it must be executed on, and the job that z belongs to

The order of operations in each job is given. After a given operation's processing time, the machine it was being executed on is immediately available. However, the next operation in the job can only be started after the idle time has passed.

Machines are continuously available from time 0, and can complete at most one operation at a time.

The goal is to minimize the completion time of the last job (makespan).

## Approach

This is an NP hard problem. Although exact methods exist, they quickly become infeasible for large instances. Previous research showed that exact methods for instances with as little as 100 operations can take approximately 2 hours to calculate.

This project looks at the efficacy of a local search approach.

## The Code

Note that this code is a first draft. It has not been optimized, nor has it been coded with extensibility and/or readability in mind.

There is a makefile provided. On a unix machine, simply type `make` to build.

## Performance

For some instances (those with sufficient machines relative to jobs), the lower bound given by the maximum job makespan is equal to the makespan of the optimal solution. This gives us certainty that we have found an optimal solution, whereas this is impossible to determine otherwise. Note that optimal solutions may be very abundant in this case, so the time required to find one may not be realistic for solutions without this property.

For instances with machine scarcity, the time shown is the time it took until there were 5 restarts that did not result in a better makespan. The assumption made is that the makespan is reasonably close to optimal at that point. The makespan does tend to converge quite fast, so more than 80% of the time is spent eeking out an extra 10% on the makespan.

total ops | jobs | types | total machines | time     | optimal
----------|------|-------|----------------|---------:|--------
      368 |   32 |     6 |             45 |     0:00 |     YES
      344 |   32 |     2 |              5 |     0:02 |
     1445 |   64 |    12 |            160 |     0:09 |     YES
     1555 |   64 |     4 |             16 |     1:45 |
     6258 |  128 |    24 |            436 |    13:23 |     YES

The times listed were for an Intel Core i3 M370 @ 2.40 Hz CPU.

For comparison, an instance with 100 total operations took ~2 hours to solve using exact methods.
