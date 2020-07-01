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
            evolution_test.cpp
            conf_test.cpp
            )
    target_link_libraries(species_test
            speciation Catch2::Catch2)

    IF(CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_definitions(species_test PRIVATE
                DEBUG=${CMAKE_BUILD_TYPE})
    ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

    add_test(species_test species_test)
    #add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
    add_custom_target(check COMMAND species_test -d yes --order lex
            DEPENDS species_test)

endif()
