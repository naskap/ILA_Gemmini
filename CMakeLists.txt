cmake_minimum_required(VERSION 3.9.6)

# ---------------------------------------------------------------------------- #
# PROJECT
# name version language
# ---------------------------------------------------------------------------- #
project(Gemmini LANGUAGES CXX)

# ---------------------------------------------------------------------------- #
# VARIABLE
# alias and configurations
# ---------------------------------------------------------------------------- #
set(MyTarget ${PROJECT_NAME})

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CXXFLAGS  " -g ")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")


# ---------------------------------------------------------------------------- #
# External dependencies
# ---------------------------------------------------------------------------- #
##
## ilang
##
find_package(ilang REQUIRED)

# ---------------------------------------------------------------------------- #
# TARGET
# executable
# ---------------------------------------------------------------------------- #
add_executable(${MyTarget} 
  app/main.cc
  src/gemmini-top.cc
  src/load.cc
  src/store.cc
  src/utils.cc
)

target_include_directories(${MyTarget} PRIVATE include)

target_link_libraries(${MyTarget} ilang::ilang)
