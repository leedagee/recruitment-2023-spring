cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG -g -O3 -ffast-math")
project(SpMM LANGUAGES C CXX)

set(VTUNE_DIR "/opt/intel/oneapi/vtune/latest")

option(VTUNE "Insert vtune pause / resume.")
set(REPR 1 CACHE STRING "Repeat times")
set(NTHREADS 8 CACHE STRING "Num of threads")

if(${VTUNE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVTUNE_ENABLE")
    include_directories("${VTUNE_DIR}/sdk/include")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DREPR=${REPR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNTHREADS=${NTHREADS}")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUGCSR")

find_package(OpenMP)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("${PROJECT_SOURCE_DIR}/utils")

aux_source_directory (. SOURCES)
aux_source_directory (src SOURCES)
aux_source_directory (src/leedagee SOURCES)
aux_source_directory (include SOURCES)
aux_source_directory (utils SOURCES)
add_library(SpMM STATIC ${SOURCES})
set_property(TARGET SpMM PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_link_libraries(SpMM PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(SpMM PRIVATE "${VTUNE_DIR}/sdk/lib64/libittnotify.a")
target_link_libraries(SpMM PRIVATE dl)

# add_test()
enable_testing()
add_subdirectory(tests)

if(EXISTS /tmp/dataset)
    message("Found /tmp/dataset, using pre-generated data")
    include("etc/tests.cmake")
else()
    message("Not found /tmp/dataset, generating if needed")
    include("etc/tests-local.cmake")
endif()
