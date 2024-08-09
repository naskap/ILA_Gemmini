import os

def gen_cmakelists(sim_folder : str):
    assert os.path.exists(sim_folder)

    src_folder = os.path.join(sim_folder, "src")
    assert os.path.exists(src_folder)

    src_list = [os.path.join("${CMAKE_CURRENT_SOURCE_DIR}/src", fname) for fname in os.listdir(src_folder)]
    srcs = "\n".join(src_list)

    return f"""# CMakeLists.txt for Gemmini
cmake_minimum_required(VERSION 3.14.0)
project(Gemmini LANGUAGES CXX)

add_compile_definitions(ILATOR_VERBOSE=1)

option(JSON_SUPPORT "Build JSON parser support" OFF)

find_package(SystemCLanguage CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD ${{SystemC_CXX_STANDARD}})

aux_source_directory(extern extern_src)

add_library(Gemmini_lib
${{extern_src}}
{srcs}
../../uninterpreted_func/uninterpreted_func.cc
)

target_include_directories(Gemmini_lib PRIVATE include)
target_include_directories(Gemmini_lib PRIVATE /home/nathan/sandbox/code/usr/ac_types/include)
target_link_libraries(Gemmini_lib SystemC::systemc)


file(GLOB TESTBENCHES "app/*.cc")

foreach(TESTBENCH ${{TESTBENCHES}})


get_filename_component(FILENAME ${{TESTBENCH}} NAME)
string(REPLACE "_tb.cc" "" TEST_NAME ${{FILENAME}})



add_executable(Gemmini_test_${{TEST_NAME}}
  ${{TESTBENCH}}
)

target_compile_definitions(Gemmini_test_${{TEST_NAME}} PRIVATE CHECK_RESULTS=1)
target_include_directories(Gemmini_test_${{TEST_NAME}} PRIVATE ../../sim_infra/)
target_include_directories(Gemmini_test_${{TEST_NAME}} PRIVATE include)
target_link_libraries(Gemmini_test_${{TEST_NAME}} PRIVATE Gemmini_lib )



endforeach()



if(${{ILATOR_VERBOSE}})
  target_compile_definitions(Gemmini_test_aligned PRIVATE ILATOR_VERBOSE)
endif()
if(${{JSON_SUPPORT}})
  include(FetchContent)
  FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.8.0
  )
  FetchContent_MakeAvailable(json)
  target_link_libraries(Gemmini nlohmann_json::nlohmann_json)
endif()
"""

if __name__ == "__main__":
    print(gen_cmakelists("../build/sim_model11"))