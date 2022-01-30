# Parallel Graphplan

Code for my Bachelor's Thesis (2018) about parallelizing the symbolic planning algorithm _Graphplan_ using incremental SAT (boolean satisfiability problem) solving.

## Setup

1. Clone the repository including git submodules in order to obtain ipasir
2. Build the project using CMake

## Usage

```bash
# Run the planner with 8 threads (sat solvers) on a problem of the childsnack domain
./parallel_graphplan -t=4 ../data/sas/childsnack10
```

The planner works on the 'sas' file format, which can be obtained from regular PDDL files using the translator of the [Fast Downward](https://github.com/aibasel/downward) planning system, found [here](https://github.com/aibasel/downward/blob/main/src/translate/translate.py). A few examples are given in the `data/sas` directory.


