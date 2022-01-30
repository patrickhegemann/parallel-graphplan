cmake_minimum_required(VERSION 3.21)
project(parallel_graphplan)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

include_directories(include)

add_executable(parallel_graphplan
        include/Planners/LPEPEPlanner.h
        include/Planners/Planner.h
        include/Planners/PlannerWithSATExtraction.h
        include/Planners/SimpleParallelPlannerWithSAT.h
        include/common.h
        include/ipasir.h
        include/IPlanningProblem.h
        include/Logger.h
        include/ParallelGP.h
        include/ParameterProcessor.h
        include/Parser.h
        include/pgp_utility.h
        include/Plan.h
        include/PlanningProblem.h
        include/SATPriorityThreadPool.h
        include/SATSolverThreadPool.h
        include/Settings.h
        include/Threading.h
        include/ThreadPool.h
        ipasir/ipasir.h
        src/Planners/LPEPEPlanner.cpp
        src/Planners/Planner.cpp
        src/Planners/PlannerWithSATExtraction.cpp
        src/Planners/SimpleParallelPlannerWithSAT.cpp
        src/Logger.cpp
        src/ParallelGP.cpp
        src/Parser.cpp
        src/Plan.cpp
        src/PlanningProblem.cpp
        src/SATPriorityThreadPool.cpp
        src/SATSolverThreadPool.cpp
        src/ThreadPool.cpp
        )

find_library(IPASIR ipasirglucose4 ipasir/sat/glucose4/)
target_link_libraries(parallel_graphplan ${IPASIR})
target_link_libraries(parallel_graphplan pthread)
