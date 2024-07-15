#include <Gemmini.h>

#define FUNCT_CONFIG 0
#define CONFIG_TYPE_MVIN 0b01
#define ACC_MVIN_ACCTYPE 0
#define ACC_MVIN_INPUTTYPE 1
#define CONFIG_MVIN1 0
#define CONFIG_MVIN2 1
#define CONFIG_MVIN3 2
#define OPCODE_CUSTOM3 0b1111011
#define ARRAY_DIM 16


using namespace sc_core;
int sc_main(int argc, char* argv[]) {
  Gemmini g("g");
  
  unsigned int config_type    = CONFIG_TYPE_MVIN;
  unsigned int acc_mvin_type  = ACC_MVIN_ACCTYPE;
  unsigned int mvin_num       = CONFIG_MVIN1;
  unsigned int num_rows       = 12;
  unsigned int num_cols       = 32;
  unsigned int spad_stride    = num_rows;
  long     unsigned int scale = 1;
  unsigned int soc_mem_stride = num_cols * 2;
  
  sc_signal<sc_biguint<7>>  funct_write;
  sc_signal<sc_biguint<64>> rs1;
  sc_signal<sc_biguint<64>> rs2;
  sc_signal<sc_biguint<7>>  opcode;

  g.Gemmini_instr_funct_in(funct_write);
  g.Gemmini_instr_rs1_in(rs1);
  g.Gemmini_instr_rs2_in(rs2);
  g.Gemmini_instr_opcode_in(opcode);

  funct_write = 0;
  rs1         = (scale << 32)
                + (spad_stride << 16)
                + (mvin_num     << 3)
                + (acc_mvin_type << 2)
                + (config_type << 0);
  rs2    = soc_mem_stride;
  opcode = OPCODE_CUSTOM3;
  g.instr_log.open("./instr_log.txt",std::ofstream::out);
  sc_start();
  return 0; 
}
