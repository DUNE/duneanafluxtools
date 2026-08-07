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

option(duneanafluxtools_USE_DUNEANAOBJ "Whether to build caf tools" ON)
if(NOT DEFINED DUNE_ANAOBJ_BRANCH OR "${DUNE_ANAOBJ_BRANCH}x" STREQUAL "x")
  set(DUNE_ANAOBJ_BRANCH v04_00_00)
endif()

# -- dependencies start

find_package(ROOT 6 REQUIRED)

if(duneanafluxtools_USE_DUNEANAOBJ)
  find_package(duneanaobj QUIET)
  if(NOT TARGET duneanaobj::StandardRecord) # need to fetch it
    include(FetchContent)
    FetchContent_Declare(
        duneanaobj
        GIT_REPOSITORY https://github.com/DUNE/duneanaobj.git
        GIT_TAG        ${DUNE_ANAOBJ_BRANCH}
      )
    set(SKIP_CET ON)
    FetchContent_MakeAvailable(duneanaobj)
  endif()
endif()

# -- dependencies end

add_subdirectory(src/duneanafluxtools)

add_library(duneanafluxtools_all INTERFACE)

target_link_libraries(duneanafluxtools_all INTERFACE duneanafluxtools::fluxweighter)
target_include_directories(duneanafluxtools_all INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<INSTALL_INTERFACE:include>)

add_library(duneanafluxtools::all ALIAS duneanafluxtools_all)

if(duneanafluxtools_USE_DUNEANAOBJ)
  add_subdirectory(app)
  add_subdirectory(examples)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.duneanafluxtools.sh.in
  ${PROJECT_BINARY_DIR}/setup.duneanafluxtools.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.duneanafluxtools.sh DESTINATION bin)

add_subdirectory(inputs)

install(EXPORT duneanafluxtools-targets
  FILE duneanafluxtoolsTargets.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/duneanafluxtools
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/duneanafluxtoolsConfig.cmake.in
  ${CMAKE_BINARY_DIR}/duneanafluxtoolsConfig.cmake
  INSTALL_DESTINATION
    /this/is/ignored/for/some/reason/thanks/kitware
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
  ${CMAKE_BINARY_DIR}/duneanafluxtoolsConfigVersion.cmake
  VERSION ${duneanafluxtools_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES
    ${CMAKE_BINARY_DIR}/duneanafluxtoolsConfig.cmake
    ${CMAKE_BINARY_DIR}/duneanafluxtoolsConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/duneanafluxtools)
