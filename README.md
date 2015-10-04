# Job Shop Research

A local search approach for solving a variant of the job shop problem, where the machines are replaced with a number of identical, parallel machines.

## Problem Definition

We are given:
* t machine types T_1, ... , T_t
  * machine type x has m_x machines
* n jobs J_1, ... , J_n
  * job y has l operations O_1, ... , O_l
    * operation z has processing time p_z, an idle time i_z, a machine type it must be executed on, and the job that z belongs to

The order of operations in each job is given. After a given operation's processing time, the machine it was being executed on is immediately available. However, the next operation in the job can only be started after the idle time has passed.

Machines are continuously available from time 0, and can complete at most one operation at a time.

The goal is to minimize the completion time of the last job (makespan).

## Approach

This is an NP hard problem. Although exact methods exist, they quickly become infeasible for large instances.

This project looks at the efficacy of a local search approach.
