cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Ofast")
project(SpMM LANGUAGES C CXX)

set(VTUNE_DIR "/opt/intel/oneapi/vtune/2023.1.0")
set(VTUNE_REPR 1)

if(${VTUNE_ENABLE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVTUNE_ENABLE")
    set(VTUNE_REPR 256)
    include_directories("${VTUNE_DIR}/sdk/include")
endif()

if(${STAT_FLADDOPS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTAT_FLADDOPS")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVTUNE_REPR=${VTUNE_REPR}")

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

# add_test()
enable_testing()
add_subdirectory(tests)

include("etc/tests-local.cmake")
