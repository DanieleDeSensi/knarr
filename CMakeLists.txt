cmake_minimum_required(VERSION 3.5.1)
project(riff)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

###########
# Options #
###########
option (ENABLE_TESTS "Enables testing" OFF)
option (ENABLE_CPPCHECK "Enables cppcheck checks" OFF)
option (ENABLE_CODECOV "Enables code coverage reports" OFF)
option (ENABLE_CLANGFORMAT "Enables clang-format formatting" OFF)

add_compile_options(-Wall -finline-functions -O3 -fPIC)

# This must be the first thing done, since COVERAGE_COMPILER_FLAGS must be used by all the targets
###########
# codecov #
###########
if (ENABLE_CODECOV)
    set (CMAKE_BUILD_TYPE Debug)
    if (NOT ENABLE_TESTS)
        message (FATAL_ERROR "You need to define -DENABLE_TESTS=ON when you use -DENABLE_CODECOV=ON")
    endif()
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
endif (ENABLE_CODECOV)

###########
# Library #
###########
add_subdirectory(src)
add_subdirectory(demo)

############
# cppcheck #
############
if (ENABLE_CPPCHECK)
    include(cmake/cppcheck.cmake)
endif (ENABLE_CPPCHECK)

###########
# Testing #
###########
if (ENABLE_TESTS)
	add_subdirectory(test)
    add_custom_target(test_riff
                      COMMAND ${PROJECT_SOURCE_DIR}/test/runtests.sh
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test
  	)
endif (ENABLE_TESTS)

###########
# codecov #
###########
if (ENABLE_CODECOV)
    set(COVERAGE_GCOVR_EXCLUDES 'src/external/*' 'test/*' 'demo/*')
    SETUP_TARGET_FOR_COVERAGE_GCOVR_XML(
        NAME coverage
        EXECUTABLE make && make test_riff
        DEPENDENCIES riff
    )
endif (ENABLE_CODECOV)

################
# clang-format #
################
if (ENABLE_CLANGFORMAT)
  include(cmake/clang_format.cmake)
endif (ENABLE_CLANGFORMAT)

#####################################
# compile without optimizations -O0 #
#####################################
# if (CMAKE_COMPILER_IS_GNUCC)
#   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#   set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
# endif (CMAKE_COMPILER_IS_GNUCC)
