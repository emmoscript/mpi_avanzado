cmake_minimum_required(VERSION 3.16)
project(MPI_Avanzado_Comunicaciones_Colectivas)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MPI
find_package(MPI REQUIRED)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Include directories
include_directories(${MPI_CXX_INCLUDE_PATH})

# Main executable
add_executable(mpi_promedio src/main.cpp)
target_link_libraries(mpi_promedio ${MPI_CXX_LIBRARIES})

# Benchmark executable
add_executable(mpi_benchmark src/benchmark.cpp)
target_link_libraries(mpi_benchmark ${MPI_CXX_LIBRARIES})

# Test executable
add_executable(mpi_test src/test.cpp)
target_link_libraries(mpi_test ${MPI_CXX_LIBRARIES})

# Installation
install(TARGETS mpi_promedio mpi_benchmark mpi_test
        RUNTIME DESTINATION bin)

# Copy documentation
install(FILES README.md INSTALL.md
        DESTINATION share/doc/mpi-avanzado) 