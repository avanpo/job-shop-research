# Flexible Job Shop Research

This project looks at the efficacy of the critical path neighborhood for the Flexible Job Shop and Flexible Blocking Job Shop problems. It should be noted that the definition used in this project is unfortunately less general than the formulation frequently used in literature. For a formal definition and analysis, see the included paper.

## Practical considerations

The project is implemented in C. This code is a first draft. It has not been optimized, nor has it been coded with extensibility and/or readability in mind.

There is a makefile provided. On a unix machine, simply type `make` in the command line to build. Running the executable without any arguments will display the manual, defining the expected and optional arguments.

The makefile also creates an executable to generate instance files, in the `instances` folder. A few of these generated instances are included. There are also two scripts to convert instances files from other sources (namely a previous experimentation project, and benchmark FJS instances from jobshop1.txt) to the format used by this code.
