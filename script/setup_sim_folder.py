import argparse
import os
import shutil
from gen_cmakelists import gen_cmakelists

if __name__ == "__main__":

    ILA_Gemmini_root = os.path.dirname(os.path.abspath(os.curdir))

    # Take name argument
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", action = 'store', type=str, help = "Name of the generated simulation folder to setup")
    args = parser.parse_args()
    
    sim_folder = args.name
    assert os.path.exists(sim_folder), f"Simulation folder {sim_folder} doesn't exist!"
    
    # Copy tests to sim_folder
    sim_tests = os.path.join(ILA_Gemmini_root,"sim_tests")
    shutil.copytree(src=sim_tests, \
                    dst=os.path.join(sim_folder, "app"), 
                    dirs_exist_ok=True)

    # Copy a test runner utility script
    shutil.copy(src=os.path.join(ILA_Gemmini_root,"sim_infra/test_runner.sh"),
                dst=sim_folder)
    
    # Overwrite default CMakeLists.txt
    cmakelists_fpath = os.path.join(sim_folder, "CMakeLists.txt")
    new_cmakelists_text = gen_cmakelists(sim_folder)
    with open(cmakelists_fpath, "w") as file:
        file.write(new_cmakelists_text)

    # Create build folder and run cmake
    sim_build_folder = os.path.join(sim_folder, "build")
    os.makedirs(sim_build_folder, exist_ok=True)
    os.chdir(sim_build_folder)

    # Run cmake
    process = os.popen("cmake ..")
    print(process.read())
    process.close()

    # Run select tests
    os.chdir("..")
    with os.popen("./test_runner.sh --quiet select") as process:
        for line in process:
            print(line, end='')  # Print line (may not be immediate)
