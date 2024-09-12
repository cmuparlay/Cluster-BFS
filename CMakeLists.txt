# -------------------------------------------------------------------
#                      Build system for ParlayLib
# -------------------------------------------------------------------
# Requirements:
#   - CMake version 3.14+
# -------------------------------------------------------------------

cmake_minimum_required(VERSION 3.14)
project(2HOPDISTANCE VERSION 1.0
        DESCRIPTION "A system supports fast s-t shortest distance queries in C++"
        LANGUAGES CXX)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

# Set a default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug Release RelWithDebInfo MinSizeRel)" FORCE)
  message(STATUS "No build type specified. Defaulted to DEBUG.")
  message(STATUS "To specify a build type, add -DCMAKE_BUILD_TYPE=<DEBUG/RELEASE/RELWITHDEBINFO/MINSIZEREL>")
endif(NOT CMAKE_BUILD_TYPE)

# Make sure -fno-omit-frame-pointer is set for profiling
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer")
else()
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oy-")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")

# message(STATUS "PARLAY VERSION ${PARLAY_VERSION}")
message(STATUS "---------------------------- General configuration -----------------------------")
message(STATUS "CMake Generator:                ${CMAKE_GENERATOR}")
message(STATUS "Compiler:                       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type:                     ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS:                ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG:          ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE:        ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS          ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_INSTALL_PREFIX:           ${CMAKE_INSTALL_PREFIX}" )

# Set module path
# list(APPEND CMAKE_MODULE_PATH "${2HOPDISTANCE_SOURCE_DIR}/cmake")

# -------------------------------------------------------------------
#                       Library definition

add_library(parlay INTERFACE)
# set(PARLAY_INCLUDE_DIR "${PARLAY_SOURCE_DIR}/include")
set(PARLAY_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
target_include_directories(parlay INTERFACE
  $<BUILD_INTERFACE:${PARLAY_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

option(PARLAY_USE_CXX_20 "Require Parlay to build with C++20")
if (PARLAY_USE_CXX_20)
  target_compile_features(parlay INTERFACE cxx_std_20)
else()
  target_compile_features(parlay INTERFACE cxx_std_17)
endif()

# Link against system threads
find_package(Threads REQUIRED)
target_link_libraries(parlay INTERFACE Threads::Threads)
# get_target_property(PARLAY_INCLUDES parlay INTERFACE_INCLUDE_DIRECTORIES)
# message(STATUS "Parlay includes: ${PARLAY_INCLUDES}")
# message(STATUS "Parlay includes dir: ${PARLAY_INCLUDE_DIR}")

message(STATUS "-------------------------------- Library options ------------------------------")

# -----------------------include Abseil----------------------------
# Add the Abseil directory
add_subdirectory(include/abseil-cpp)
# -------------------------------------------------------------------
#                Enable/disable elastic parallelism

option(PARLAY_ELASTIC_PARALLELISM "Enable elastic parallelism" On)

# -------------------------------------------------------------------
#              Support for alternative parallel runtimes

option(PARLAY_CILKPLUS "Enable integration with CilkPlus")
option(PARLAY_OPENCILK "Enable integration with OpenCilk")
option(PARLAY_OPENMP "Enable integration with OpenMP")
option(PARLAY_TBB "Enable integration with TBB")
option(PARLAY_SEQUENTIAL "Run Parlay with no parallelism")

if (PARLAY_CILKPLUS)
  message(STATUS "2HopDistance CilkPlus integration enabled")
  check_cxx_compiler_flag("-fcilkplus" CILKPLUS_SUPPORT)
  if(CILKPLUS_SUPPORT)
    target_compile_definitions(parlay INTERFACE PARLAY_CILKPLUS)
    target_compile_options(parlay INTERFACE "-fcilkplus")
    target_link_options(parlay INTERFACE "-fcilkplus")
  else()
    message(FATAL_ERROR "You are trying to enable CilkPlus integration, but your compiler does not support CilkPlus.
                         You'll need to switch to a compiler that does, or turn it off (-DPARLAY_CILKPLUS=Off)")
  endif()

elseif(PARLAY_OPENCILK)
  message(STATUS "2HopDistance OpenCilk integration enabled")
  check_cxx_compiler_flag("-fopencilk" OPENCILK_SUPPORT)
  if(OPENCILK_SUPPORT)
    target_compile_definitions(parlay INTERFACE PARLAY_OPENCILK)
    target_compile_options(parlay INTERFACE "-fopencilk")
    target_link_options(parlay INTERFACE "-fopencilk")
  else()
    message(FATAL_ERROR "You are trying to enable OpenCilk integration, but your compiler does not support OpenCilk.
                         You'll need to switch to a compiler that does, or turn it off (-PARLAY_OPENCILK=Off)")
  endif()
elseif(PARLAY_OPENMP)
  message(STATUS "2HopDistance OpenMP integration enabled")
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
    target_compile_definitions(parlay INTERFACE PARLAY_OPENMP)
    target_link_libraries(parlay INTERFACE OpenMP::OpenMP_CXX)
  else()
    message(FATAL_ERROR "You are trying to enable OpenMP integration, but your compiler does not support OpenMP, or
                         maybe you don't have OpenMP installed. Fix this, or turn it off (-PARLAY_OPENMP=Off)")
  endif()
elseif(PARLAY_TBB)
  message(STATUS "2HopDistance TBB integration enabled")
  find_package(TBB)
  if(TBB_FOUND)
    target_compile_definitions(parlay INTERFACE PARLAY_TBB)
    target_link_libraries(parlay INTERFACE TBB::tbb)
  else()
    message(FATAL_ERROR "You are trying to enable TBB integration, but the TBB library could not be found. Install it,
                         point CMake towards where you have it installed, or turn it off (-PARLAY_TBB=Off)")
  endif()
elseif(PARLAY_SEQUENTIAL)
  message(STATUS "Parlay sequential mode enabled (no parallelism!!)")
  target_compile_definitions(parlay INTERFACE PARLAY_SEQUENTIAL)
else()
  message(STATUS "Using Parlay scheduler. Switch with -DPARLAY_{CILKPLUS,OPENCILK,OPENMP,TBB}=On")
  if (PARLAY_ELASTIC_PARALLELISM)
    message(STATUS "Elastic parallelism enabled. Disable with -DPARLAY_ELASTIC_PARALLELISM=Off")
    target_compile_definitions(parlay INTERFACE PARLAY_ELASTIC_PARALLELISM=true)
    
    # On Windows we need to link against synchronization.lib for WaitOnAddress
    if (WIN32)
      target_link_libraries(parlay INTERFACE synchronization)
    endif()
  else()
    message(STATUS "Elastic parallelism disabled. Enable with -DPARLAY_ELASTIC_PARALLELISM=On")
    target_compile_definitions(parlay INTERFACE PARLAY_ELASTIC_PARALLELISM=false)
  endif()
endif()

# -------------------------------------------------------------------
#                          Benchmarks

message(STATUS "----------------------------------- Benchmarks -----------------------------------")

# User option to build examples

message(STATUS "benchmarks: Enabled")
include_directories(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(benchmark)


