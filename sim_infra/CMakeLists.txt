# CMakeLists.txt for Gemmini
cmake_minimum_required(VERSION 3.14.0)
project(Gemmini LANGUAGES CXX)

option(ILATOR_VERBOSE "Enable instruction sequence logging" ON)
add_compile_definitions(ILATOR_VERBOSE=1)

option(JSON_SUPPORT "Build JSON parser support" OFF)

find_package(SystemCLanguage CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD})

aux_source_directory(extern extern_src)

add_library(Gemmini_lib
${extern_src}
${CMAKE_CURRENT_SOURCE_DIR}/src/all_valid_funcs_in_hier.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/compute.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/constant_memory_def.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_CONFIG_LOAD0.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_CONFIG_LOAD1.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_CONFIG_LOAD2.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_CONFIG_STORE.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load0.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load0_acc_acctype_accumulate.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load0_acc_acctype_overwrite.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load0_acc_inputtype_accumulate.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load0_acc_inputtype_overwrite.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load0_spad.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load1.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load1_acc_acctype_accumulate.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load1_acc_acctype_overwrite.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load1_acc_inputtype_accumulate.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load1_acc_inputtype_overwrite.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load1_spad.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load2.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load2_acc_acctype_accumulate.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load2_acc_acctype_overwrite.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load2_acc_inputtype_accumulate.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load2_acc_inputtype_overwrite.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_load2_spad.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_store.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_store_row_fromacc_acctype.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_store_row_fromacc_acctype_maxpool.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_store_row_fromacc_inputtype.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_store_row_fromacc_inputtype_maxpool.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_store_row_fromspad_inputtype.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/idu_store_row_fromspad_inputtype_maxpool.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/memory_update_functions_0.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/memory_update_functions_1.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/setup_initial_condition.cc
../../uninterpreted_func/uninterpreted_func.cc
)

target_include_directories(Gemmini_lib PRIVATE include)
target_include_directories(Gemmini_lib PRIVATE /home/nathan/sandbox/code/usr/ac_types/include)
target_link_libraries(Gemmini_lib SystemC::systemc)


file(GLOB TESTBENCHES "app/*.cc")

foreach(TESTBENCH ${TESTBENCHES})


get_filename_component(FILENAME ${TESTBENCH} NAME)
string(REPLACE "_tb.cc" "" TEST_NAME ${FILENAME})



add_executable(Gemmini_test_${TEST_NAME}
  ${TESTBENCH}
)

target_compile_definitions(Gemmini_test_${TEST_NAME} PRIVATE CHECK_RESULTS=1)
target_include_directories(Gemmini_test_${TEST_NAME} PRIVATE ../../sim_infra/)
target_include_directories(Gemmini_test_${TEST_NAME} PRIVATE include)
target_link_libraries(Gemmini_test_${TEST_NAME} PRIVATE Gemmini_lib )



endforeach()



if(${ILATOR_VERBOSE})
  target_compile_definitions(Gemmini_test_aligned PRIVATE ILATOR_VERBOSE)
endif()
if(${JSON_SUPPORT})
  include(FetchContent)
  FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.8.0
  )
  FetchContent_MakeAvailable(json)
  target_link_libraries(Gemmini nlohmann_json::nlohmann_json)
endif()
