# Script to generate systemc testbenches from pre-written gemmini test cases
# Note: Not able to robustly handle variable-length-arrays which are defined differently between 
#       C and C++. If we want to make it more robust we should use actual c parser like pycparser
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

    # Replace variable length arrays
    if('conv' in test_fpath):
        top_of_file, main_body = _sub_varlenarrays(top_of_file, main_body)


    test_fname = test_fpath.split("/")[-1]
    outfname = os.path.join(output_dir, test_fname.replace(".c","_tb.cc"))
    
    
    to_write = f"""#include <gemmini_testbench.h>
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
    g.instr_update_log.open("./instr_update_log", std::ofstream::out);
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
  sc_start(10000000000.0,SC_SEC);
  return h.status.read(); 
}}

    """
    with open(outfname, "w") as outFile:
        outFile.write(to_write)


def _sub_varlenarrays(top_of_file : str, main_body : str) -> tuple[str]:
    top_of_file = re.sub(r"""void conv\(int batch_size, int in_channels,
        int in_row_dim, int in_col_dim,
        int out_channels, int kernel_dim,
        int out_row_dim, int out_col_dim,
        (.*)
        elem_t input\[batch_size\]\[in_row_dim\]\[in_col_dim\]\[in_channels\],
        elem_t weights\[out_channels\]\[kernel_dim\]\[kernel_dim\]\[in_channels\],
        acc_t bias\[out_channels\],
        elem_t output\[batch_size\]\[out_row_dim\]\[out_col_dim\]\[out_channels\]\)""",\
        r"""template <int batch_size, int in_channels,
        int in_row_dim, int in_col_dim,
        int out_channels, int kernel_dim,
        int out_row_dim, int out_col_dim>
void conv(\1
        elem_t input[batch_size][in_row_dim][in_col_dim][in_channels],
        elem_t weights[out_channels][kernel_dim][kernel_dim][in_channels],
        acc_t bias[out_channels],
        elem_t output[batch_size][out_row_dim][out_col_dim][out_channels])""", top_of_file)
    
    main_body = re.sub(r"""conv\(([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),[\s\n]*([a-zA-Z_]\w*),""", \
                       r"""conv<\1, \2,
            \3, \4,
            \5, \6,
            \7, \8>(
            \9, """, main_body)
    
    top_of_file = top_of_file.replace(r"""void flatten_weights(int out_channels, int kernel_dim, int in_channels,
        int patch_size,
        elem_t weights[out_channels][kernel_dim][kernel_dim][in_channels],
        elem_t weights_mat[patch_size][out_channels])""",
        r"""template <int out_channels, int kernel_dim, int in_channels, int patch_size>
void flatten_weights(
        elem_t weights[out_channels][kernel_dim][kernel_dim][in_channels],
        elem_t weights_mat[patch_size][out_channels])""")
        
    main_body = main_body.replace(r"""flatten_weights(OUT_CHANNELS, KERNEL_DIM, IN_CHANNELS,
            PATCH_SIZE,
            weights,
            weights_mat);""", r"""flatten_weights<OUT_CHANNELS, KERNEL_DIM, IN_CHANNELS, PATCH_SIZE>(weights,
            weights_mat);""")
    
    return top_of_file, main_body



if __name__ == "__main__":

    chipyard_root_default = '/home/nathan/sandbox/code/chipyard'
    chipyard_root = input(f"Enter the root directory of chipyard (default = {chipyard_root_default}): ") or chipyard_root_default
    if(not os.path.exists(chipyard_root)):
        raise Exception(f"Path {chipyard_root} doesn't exist")
    
    
    def _gen_testbenches_from_dir(tests_root):

        if(not os.path.exists(tests_root)):
           raise Exception(f"Path {tests_root} doesn't exist")
           

        for file in os.listdir(tests_root):
            if(file[-2:] != ".c" or ("alexnet" in file)):
                print(f"skipping {file}")
                continue
            gen_tb(os.path.join(tests_root,file))      

    baremetal_c_tests_root = os.path.join(chipyard_root, "generators/gemmini/software/gemmini-rocc-tests/bareMetalC")
    _gen_testbenches_from_dir(baremetal_c_tests_root)
    imagenet_c_tests_root = os.path.join(chipyard_root, "generators/gemmini/software/gemmini-rocc-tests/imagenet")
    _gen_testbenches_from_dir(imagenet_c_tests_root)  

