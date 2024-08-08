# Script to generate systemc testbenches from pre-written gemmini test cases
import os
import re

def gen_tb(test_fpath : str, output_dir : str = None):
    
    # Set output dir to be ../sim_tests
    if(output_dir is None):
        output_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)),"sim_tests")
    
    with open(test_fpath,"r") as inFile:
        file_contents = inFile.read()
    
    # Extract top of file and main function contents
    main_loc = re.search(r"int main.*",file_contents)
    top_of_file = file_contents[:main_loc.span()[0]]
    main_body = file_contents[main_loc.span()[1]:]
    inFile.close()
    
    # Replace return/exit values with setting status signals
    main_body = re.sub(r'return\s+(\d+)', r'status = \1; return', main_body)
    main_body = re.sub(r'exit\(\s*(\d+)\s*\)', r'status = \1; return', main_body)

    #
    test_fname = test_fpath.split("/")[-1]
    outfname = os.path.join(output_dir, test_fname.replace(".c","_tb.cc"))
    outFile = open(outfname, "w")
    
    outFile.write(f"""#include <gemmini_testbench.h>
{top_of_file}
SC_MODULE(Testbench){{
  sc_clock clk;
  sc_signal<sc_uint<8>> status;
  SC_CTOR(Testbench) : clk("clk",1,SC_SEC){{
    SC_THREAD(tb_thread)
    g.Gemmini_instr_funct_in(funct_write);
    g.Gemmini_instr_rs1_in(rs1);
    g.Gemmini_instr_rs2_in(rs2);
    g.Gemmini_instr_opcode_in(opcode);
    g.instr_log.open("./instr_log.txt",std::ofstream::out);
    status = test_status::UNFINISHED;
  }}
  void tb_thread(){{
  
  int argc = 1;
  char *argv[] = {{const_cast<char*>("./Gemmini_test_{test_fname[:-2]}")}};

  {main_body}
  
}};

int sc_main(int argc, char* argv[]) {{
  assert(__BYTE_ORDER == __LITTLE_ENDIAN);
  Testbench h("h");
  sc_start(10000,SC_SEC);
  return h.status.read(); 
}}

    """)
    outFile.close()




if __name__ == "__main__":

    chipyard_root = input("Enter the root directory of chipyard: ")
    if(not os.path.exists(chipyard_root)):
        raise Exception(f"Path {chipyard_root} doesn't exist")
    
    tests_root = os.path.join(chipyard_root, "generators/gemmini/software/gemmini-rocc-tests/bareMetalC")
    if(not os.path.exists(tests_root)):
        raise Exception(f"Path {tests_root} doesn't exist. Make sure you setup gemmini and its associated test infrastructure")
    
    for file in os.listdir(tests_root):
        if(file[-2:] != ".c"):
            print(f"skipping {file}")
            continue
        gen_tb(os.path.join(tests_root,file))        

