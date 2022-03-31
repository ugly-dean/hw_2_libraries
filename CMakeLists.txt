cmake_minimum_required(VERSION 3.16)
project(Similarity)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Werror -Wextra -Wpedantic -Wall")
# set(CMAKE_C_FLAGS "-Wextra -Wpedantic -Wall")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fprofile-arcs -ftest-coverage -O0")

set(PROJECT_NAME similarity)

set(PROJECT_SOURCE_DIR .)

option(PARALLEL "Parallel implementation mode" OFF)
if (PARALLEL)
    message("Parallel mode is on")
    set(PROJECT_LIB_SOURCE_DIR ${PROJECT_SOURCE_DIR}/dynamic)
    set(PROJECT_LIB_INCLUDE_DIR ${PROJECT_LIB_SOURCE_DIR})
    set(PROJECT_CORE_LIB_NAME dynamic_sim)
    set(LIB_TYPE SHARED)
else()
    message("Static mode is on")
    set(PROJECT_LIB_SOURCE_DIR ${PROJECT_SOURCE_DIR}/static)
    set(PROJECT_LIB_INCLUDE_DIR ${PROJECT_LIB_SOURCE_DIR})
    set(PROJECT_CORE_LIB_NAME static_sim)
    set(LIB_TYPE STATIC)
endif()

add_library(${PROJECT_CORE_LIB_NAME} ${LIB_TYPE} ${PROJECT_LIB_SOURCE_DIR}/similarity.c)
target_include_directories(${PROJECT_CORE_LIB_NAME} PUBLIC ${PROJECT_LIB_INCLUDE_DIR})
message("Added ${LIB_TYPE} library with name ${PROJECT_CORE_LIB_NAME}")

# Enable generating coverage report in testing process
option(COVERAGE "Enable generating coverage report" OFF)
if (COVERAGE)
    message("Enabled code coverage report generation")
    target_compile_options(${PROJECT_CORE_LIB_NAME} PUBLIC "-coverage" "-lgcov" "-O0")
    target_link_libraries(${PROJECT_CORE_LIB_NAME} PUBLIC gcov)
endif ()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.c)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_LIB_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_CORE_LIB_NAME} m pthread)
message("Created app target with name ${PROJECT_NAME}")

# Testing with gtest Google C++ Testing Framework
option(TEST "Enable testing" OFF)
if (TEST)
    message("Enabled testing")
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    message("Google C++ Testing Framework loaded")

    file(GLOB TestsSRC "tests/*.cpp")
    set(testing_targets)
    foreach (file IN ITEMS ${TestsSRC})
        message("${file} TEST FILE FOUND")
        get_filename_component(testing_target ${file} NAME_WE)

        # Adding executable tests without sanitizers
        add_executable(${testing_target} ${file})
        target_link_libraries(${testing_target} gtest_main)
        target_link_libraries(${testing_target} ${PROJECT_CORE_LIB_NAME})

        # Adding executable tests with Address sanitizer
        add_executable(${testing_target}_Address ${file})
        target_link_libraries(${testing_target}_Address gtest_main)
        target_link_libraries(${testing_target}_Address ${PROJECT_CORE_LIB_NAME})
        target_compile_options(${testing_target}_Address PUBLIC -fsanitize=address)
        target_link_options(${testing_target}_Address PUBLIC -fsanitize=address)

        # Adding executable tests with Thread sanitizer
        add_executable(${testing_target}_Thread ${file})
        target_link_libraries(${testing_target}_Thread gtest_main)
        target_link_libraries(${testing_target}_Thread ${PROJECT_CORE_LIB_NAME})
        target_compile_options(${testing_target}_Thread PUBLIC -fsanitize=thread)
        target_link_options(${testing_target}_Thread PUBLIC -fsanitize=thread)

        # Adding executable tests with Undefined sanitizer
        add_executable(${testing_target}_Undefined ${file})
        target_link_libraries(${testing_target}_Undefined gtest_main)
        target_link_libraries(${testing_target}_Undefined ${PROJECT_CORE_LIB_NAME})
        target_compile_options(${testing_target}_Undefined PUBLIC -fsanitize=undefined)
        target_link_options(${testing_target}_Undefined PUBLIC -fsanitize=undefined)

        message("${testing_target} TARGET TESTS KIT CREATED")

        set(testing_targets
                ${testing_targets};
                ${testing_target};
                ${testing_target}_Address;
                ${testing_target}_Thread;
                ${testing_target}_Undefined
        )
    endforeach ()

    message("TESTING TARGETS TOTAL: ${testing_targets}")

    include(GoogleTest)
    foreach (target IN ITEMS ${testing_targets})
        gtest_discover_tests(${target})
    endforeach ()
endif ()

# This part of provides clang-tidy code analise
option(LINTERS-CHECK "Enable clang-tidy check" OFF)
if (LINTERS-CHECK)
    message("Enabled clang-tidy sources check")
    file(GLOB SOURCES *.c)

    add_custom_target(
            clang-tidy-check-code clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json  ${SOURCES}
            DEPENDS ${SOURCES}
    )
    add_custom_target(infer-check infer run --compilation-database compile_commands.json)

    add_custom_target(linter-check DEPENDS clang-tidy-check-code infer-check)
endif ()
