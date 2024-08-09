// File: Gemmini.cc

#include <Gemmini/gemmini_base.h>


namespace ilang {

namespace Gemmini {

Ila GetGemminiIla(const std::string& model_name) {
  auto m = Ila(model_name);

  // Define accelerator command fields
  command_t command;
  command.funct  = m.NewBvInput("instr_funct", INSTR_FUNCT_WIDTH);
  command.rs2    = m.NewBvInput("instr_rs2", INSTR_RS2_WIDTH);
  command.rs1    = m.NewBvInput("instr_rs1", INSTR_RS1_WIDTH);
  command.opcode = m.NewBvInput("instr_opcode", INSTR_OPCODE_WIDTH);

  // Define memory
  gemmini_memory_t memory;
  memory.spad          = m.NewMemState("spad", SPAD_ADDRESS_WIDTH, SPAD_ROW_WIDTH);
  memory.spad.SetEntryNum(SPAD_ENTRIES);
  memory.accumulator   = m.NewMemState("accumulator", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH);
  memory.accumulator.SetEntryNum(ACC_ENTRIES);
  memory.soc_mem = 
      m.NewMemState("soc_mem", SOC_MEM_ADDRESS_WIDTH, SOC_MEM_ELEMENT_BITS);

  DefineLoad(m, command, memory);
  DefineStore(m, command, memory);

  return m;
}


}; // namespace Gemmini

}; // namespace ilang
