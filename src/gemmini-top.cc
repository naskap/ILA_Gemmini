// File: Gemmini.cc

#include <Gemmini/Gemmini.h>


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
  memory.accumulator   = m.NewMemState("accumulator", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH);
  memory.soc_mem = 
      m.NewMemState("soc_mem", SOC_MEM_ADDRESS_WIDTH, SOC_MEM_ELEMENT_BITS);



  DefineLoad(m, command, memory);
  DefineStore(m, command, memory);

  return m;
}


void DefineStore(Ila& m, command_t& command, ExprRef& spad, ExprRef& soc_mem) {

  // Define Config store
  ExprRef* store_stride = (ExprRef*)malloc(sizeof(ExprRef) * 3);

  maxpool_params_t* store_maxpool_params = 
      (maxpool_params_t*)malloc(sizeof(maxpool_params_t) * 3);

  InstrRef* config_store = (InstrRef*)malloc(sizeof(InstrRef) * 3);
  for (int i = 0; i < 3; i++) {

    // Config store architectural states
    store_stride[i] = m.NewBvState("store" + std::to_string(i) + "_stride",
                                   64); // Main memory stride in bytes
    store_maxpool_params[i].enable_and_stride = m.NewBvState(
        "store" + std::to_string(i) + "_maxpool_enable_and_stride", 2);
    store_maxpool_params[i].window_size = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_window_size", 2);
    store_maxpool_params[i].upper_pad = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_upper_pad", 2);
    store_maxpool_params[i].left_pad = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_left_pad", 2);
    store_maxpool_params[i].out_dim = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_out_dim", 8);
    store_maxpool_params[i].out_rows = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_out_rows", 8);
    store_maxpool_params[i].out_cols = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_out_cols", 8);
    store_maxpool_params[i].in_rows = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_in_rows", 8);
    store_maxpool_params[i].in_cols = 
        m.NewBvState("store" + std::to_string(i) + "_maxpool_in_cols", 8);

    // Config store Instruction
    config_store[i] = m.NewInstr("CONFIG_store" + std::to_string(i));
    config_store[i].SetDecode((command.funct == 0) &
                              (ilang::Extract(command.rs1, 1, 0) == 1) &
                              (ilang::Extract(command.rs1, 4, 3) == i));

        // Config store update
        config_store[i]
            .SetUpdate(store_stride[i], command.rs2);
    config_store[i].SetUpdate(store_maxpool_params[i].enable_and_stride, ilang::Extract(command.rs1, 5, 4));
    config_store[i].SetUpdate(store_maxpool_params[i].window_size,
                              ilang::Extract(command.rs1, 7, 6));
    config_store[i].SetUpdate(store_maxpool_params[i].upper_pad,
                              ilang::Extract(command.rs1, 9, 8));
    config_store[i].SetUpdate(store_maxpool_params[i].left_pad,
                              ilang::Extract(command.rs1, 11, 10));
    config_store[i].SetUpdate(store_maxpool_params[i].out_dim,
                              ilang::Extract(command.rs1, 31, 24));
    config_store[i].SetUpdate(store_maxpool_params[i].out_rows,
                              ilang::Extract(command.rs1, 39, 32));
    config_store[i].SetUpdate(store_maxpool_params[i].out_cols,
                              ilang::Extract(command.rs1, 47, 40));
    config_store[i].SetUpdate(store_maxpool_params[i].in_rows,
                              ilang::Extract(command.rs1, 55, 48));
    config_store[i].SetUpdate(store_maxpool_params[i].in_cols,
                              ilang::Extract(command.rs1, 63, 56));
  }
}

}; // namespace Gemmini

}; // namespace ilang
