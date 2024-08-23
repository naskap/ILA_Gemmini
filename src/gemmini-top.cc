// File: Gemmini.cc

#include <Gemmini/gemmini_base.h>


namespace ilang {

namespace Gemmini {

Ila GetGemminiIla(const std::string& model_name) {
  auto m = Ila(model_name);

  command_t command;
  DefineCommand(m, &command);

  gemmini_memory_t memory;
  DefineMemory(m, &memory);

  gemmini_statevars_t gemmini_statevars;
  DefineLoad(m, command, memory, gemmini_statevars.load);
  DefineStore(m, command, memory, gemmini_statevars.store);
  DefineExecute(m, command, memory, gemmini_statevars.exec);
  DefineLoopWS(m, command, memory, gemmini_statevars);
  DefineLoopConvWS(m, command, memory, gemmini_statevars);

  return m;
}

static void DefineCommand(Ila &m, command_t *c){
  c->funct  = m.NewBvInput("instr_funct", INSTR_FUNCT_WIDTH);
  c->rs2    = m.NewBvInput("instr_rs2", INSTR_RS2_WIDTH);
  c->rs1    = m.NewBvInput("instr_rs1", INSTR_RS1_WIDTH);
  c->opcode = m.NewBvInput("instr_opcode", INSTR_OPCODE_WIDTH);
}

static void DefineMemory(Ila &m, gemmini_memory_t *mem){
  mem->spad          = m.NewMemState("spad", SPAD_ADDRESS_WIDTH, SPAD_ROW_WIDTH);
  mem->spad.SetEntryNum(SPAD_ENTRIES);
  mem->accumulator   = m.NewMemState("accumulator", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH);
  mem->accumulator.SetEntryNum(ACC_ENTRIES);
  mem->soc_mem = m.NewMemState("soc_mem", SOC_MEM_ADDRESS_WIDTH, SOC_MEM_ELEMENT_BITS);
}



}; // namespace Gemmini

}; // namespace ilang
