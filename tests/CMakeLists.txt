option(ENABLE_TESTS "Enable tests" ON)
if (${ENABLE_TESTS})

    find_package(Catch2 REQUIRED)

    #TODO if Catch2 found
    enable_testing()

    add_executable(species_test
            test_individuals.h
            main_test.cpp
            species_test.cpp
            genus_test.cpp
            selection_test.cpp
            )
    target_link_libraries(species_test
            speciation Catch2::Catch2)
    add_test(species_test species_test)

endif()
