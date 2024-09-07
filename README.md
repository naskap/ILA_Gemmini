# Gemmini ILA

This is the ILA model of the default configuration of the Gemmini accelerator (all rights reserved - University of California at Berkeley).

## Directory Structure

 - src: Source files to declare the ILA object
 - include/Gemmini: Header files to declare the ILA object
 - app: Main file to declare the ILA object, generate the systemc module, setup the simulation directory, and run some tests
 - script: Various helper scripts
 - sim_infra: Header files needed by the gemmini tests
    - Note: These files were all copied from gemmini repo, with some adjustments
 - sim_tests: Testbench drivers to be copied into generated systemc simulation folder
    - Note: These files were adapted from tests in the gemmini repo, using script/gen_testbenches.py. They also required manual adjustments to make them valid C++
 
 ## systemc file path needed for building systemc model

You need to specify the systemc path when building the generated systemc files in build/sim_model/

1. SystemC-2.3.1a or newer is needed
2. SystemC needs to be installed using CMake and specified using c++11
3. under flexnlp-ila/build/sim_model/build, cmake .. -DCMAKE_INSTALL_PREFIX=<systemc_intall_path> (ex. /home/(user_name)/local/systemc-2.3.3) to specify the systemc path.