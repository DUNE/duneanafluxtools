# CMake version check
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(duneanafluxtools VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Changes default install path to be a subdirectory of the build dir.
# Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_INSTALL_PREFIX STREQUAL "" OR CMAKE_INSTALL_PREFIX STREQUAL
  "/usr/local")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
elseif(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
endif()

# Use the compilers found in the path
find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

find_package(ROOT 6 REQUIRED)

add_subdirectory(src/duneanafluxtools)

add_library(duneanafluxtools_all INTERFACE)

target_link_libraries(duneanafluxtools_all INTERFACE duneanafluxtools::fluxweighter)
target_include_directories(duneanafluxtools_all INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<INSTALL_INTERFACE:include>)

add_library(duneanafluxtools::all ALIAS duneanafluxtools_all)

add_subdirectory(app)
add_subdirectory(examples)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.duneanafluxtools.sh.in
  ${PROJECT_BINARY_DIR}/setup.duneanafluxtools.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.duneanafluxtools.sh DESTINATION bin)

add_subdirectory(inputs)
