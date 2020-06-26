cmake_minimum_required(VERSION 3.1)
project(speciation)

set(CMAKE_CXX_STANDARD 17)

add_library(speciation INTERFACE)
set(speciation_include_dir ${PROJECT_SOURCE_DIR}/src)
target_include_directories(speciation INTERFACE ${speciation_include_dir})

target_sources(speciation INTERFACE
        ${speciation_include_dir}/speciation/speciation.h
        ${speciation_include_dir}/speciation/Species.h
        ${speciation_include_dir}/speciation/Age.h
        ${speciation_include_dir}/speciation/Individual.h
        ${speciation_include_dir}/speciation/Conf.h
        ${speciation_include_dir}/speciation/exceptions.h
        ${speciation_include_dir}/speciation/SpeciesCollection.h
        ${speciation_include_dir}/speciation/Genus.h
        ${speciation_include_dir}/speciation/PopulationManagement.h
        ${speciation_include_dir}/speciation/Selection.h
        ${speciation_include_dir}/speciation/Random.h)

add_subdirectory(tests)
