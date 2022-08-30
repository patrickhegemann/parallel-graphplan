# Parallel Graphplan

Symbolic planning in parallel using _Graphplan_ and SAT solving.

<!-- toc -->
- [About Parallel Graphplan](#about-parallel-graphplan)
    - [Built With](#built-with)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [References](#references)
<!-- tocstop -->

## About Parallel Graphplan

Code for my Bachelor's Thesis (2018) about parallelizing the symbolic planning algorithm _Graphplan_ [1] using incremental SAT (boolean satisfiability problem) solving [2].

Similar to Graphplan, the presented algorithm constructs and expands a planning graph, which a plan can then be extracted from.
The extraction step is formalized as a boolean satisfiability problem and passed to a SAT solver.
If the formula is satisfiable, a valid plan has been found and can be retrieved from the solution.
On the other hand, if a formula is unsatisfiable, i.e., a plan could not be extracted, incremental SAT solving is used to extend the formula to a longer horizon.
This already provides a speedup since the formula does not have to be created from scratch after each failed extraction step, and the progress of the SAT solver, e.g., in the form of learned clauses, is not lost by restarting the solver.
Furthermore, the algorithm is parallelized by extracting plans from different horizon lengths at the same time.
In some cases, this can result in a superlinear speedup since the extraction of a valid plan becomes easier with growing horizon length.


<!-- todo: long description of the project and what it can do and how (~motivation) -->



### Built With
* [ipasir](https://github.com/biotomas/ipasir) - The Standard Interface for Incremental Satisfiability Solving
* [lingeling](https://github.com/arminbiere/lingeling) - SAT Solver

## Getting Started

1. Clone the repository including git submodules (in order to also obtain ipasir):

    ```bash
    git clone --recurse-submodules https://github.com/patrickhegemann/parallel-graphplan.git
    ```
2. Build the project using CMake:

    ```bash
    cd parallel-graphplan/build
    cmake ..
    make
    ```

## Usage

Parallel Graphplan works directly on a file format called `sas` (State-Action-State).
Such files can be obtained from standard PDDL files using [this translator script](https://github.com/aibasel/downward/blob/main/src/translate/translate.py) of the Fast Downward planning system.
A few example files are given in the `data/sas/` directory.

You can then run the planner like this:

```bash
# Run the planner with 4 threads on a problem of the barman domain
./parallel_graphplan -t=4 ../data/sas/barman-pfile01-001
```

<details>

<summary><b>Output: Plan (click to expand)</b></summary>

```
LAYER   STEP    ACTION
1       1       grasp right shot4
2       2       fill-shot shot4 ingredient3 right left dispenser3
3       3       pour-shot-to-clean-shaker shot4 ingredient3 shaker1 right l0 l1
4       4       clean-shot shot4 ingredient3 right left
5       5       fill-shot shot4 ingredient1 right left dispenser1
6       6       pour-shot-to-used-shaker shot4 ingredient1 shaker1 right l1 l2
7       7       refill-shot shot4 ingredient1 right left dispenser1
8       8       grasp left shot3
8       9       leave right shot4
9       10      fill-shot shot3 ingredient2 left right dispenser2
10      11      grasp right shaker1
10      12      leave left shot3
11      13      shake cocktail1 ingredient3 ingredient1 shaker1 right left
13      14      grasp left shot4
13      15      pour-shaker-to-shot cocktail1 shot2 right shaker1 l2 l1
14      16      leave left shot4
14      17      empty-shaker right shaker1 cocktail1 l1 l0
15      18      clean-shaker right left shaker1
16      19      grasp left shot4
16      20      leave right shaker1
17      21      grasp right shot3
17      22      pour-shot-to-clean-shaker shot4 ingredient1 shaker1 left l0 l1
18      23      leave left shot4
18      24      pour-shot-to-used-shaker shot3 ingredient2 shaker1 right l1 l2
19      25      grasp left shaker1
19      26      leave right shot3
20      27      shake cocktail3 ingredient1 ingredient2 shaker1 left right
22      28      pour-shaker-to-shot cocktail3 shot1 left shaker1 l2 l1
23      29      grasp right shot3
23      30      leave left shaker1
24      31      refill-shot shot3 ingredient2 right left dispenser2
25      32      grasp left shaker1
26      33      leave right shot3
26      34      empty-shaker left shaker1 cocktail3 l1 l0
27      35      clean-shaker left right shaker1
28      36      grasp right shot3
28      37      leave left shaker1
29      38      pour-shot-to-clean-shaker shot3 ingredient2 shaker1 right l0 l1
30      39      clean-shot shot3 ingredient2 right left
31      40      fill-shot shot3 ingredient3 right left dispenser3
35      41      pour-shot-to-used-shaker shot3 ingredient3 shaker1 right l1 l2
36      42      clean-shot shot3 ingredient3 right left
37      43      grasp left shaker1
37      44      leave right shot3
38      45      shake cocktail2 ingredient2 ingredient3 shaker1 left right
39      46      pour-shaker-to-shot cocktail2 shot3 left shaker1 l2 l1
```
</details>


## References

1. Blum, A. L., & Furst, M. L. (1997). Fast planning through planning graph analysis. Artificial intelligence, 90(1-2), 281-300.
2. Balyo, T., Biere, A., Iser, M., & Sinz, C. (2016). SAT race 2015. Artificial Intelligence, 241, 45-65.
