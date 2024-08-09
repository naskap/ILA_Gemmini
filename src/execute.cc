#include <Gemmini/execute.h>

namespace ilang {

namespace Gemmini {

void DefineExecute(Ila& m, command_t& command, gemmini_memory_t memory){

    execute_statevars_t* execute_statevars = 
      (execute_statevars_t*)malloc(sizeof(execute_statevars_t));
    
    DefineExecuteStatevars(m, execute_statevars);
    DefineConfigExecute(m, command, execute_statevars);
    DefineMatmulPreload(m, command, execute_statevars);
}

void DefineExecuteStatevars(Ila& m, execute_statevars_t *execute_statevars){
    // Config execute statevars
    execute_statevars->dataflow = m.NewBoolState("dataflow");
    execute_statevars->act = m.NewBvState("act", 2);
    execute_statevars->sys_shift = m.NewBvState("sys_shift", 32);
    execute_statevars->sys_acc_shift = m.NewBvState("sys_acc_shift", 32);
    execute_statevars->a_transpose = m.NewBoolState("a_transpose");
    execute_statevars->b_transpose = m.NewBoolState("b_transpose");
    execute_statevars->c_stride = m.NewBvState("c_stride", 16);
    execute_statevars->a_stride = m.NewBvState("a_stride", 16);

    // Preload statevars
    execute_statevars->preload_sp_addr = m.NewBvState("preload_sp_addr", 32);
    execute_statevars->output_sp_addr = m.NewBvState("output_sp_addr", 32);
    execute_statevars->preload_cols = m.NewBvState("preload_cols", 16);
    execute_statevars->preload_rows = m.NewBvState("preload_rows", 16);
    execute_statevars->output_cols = m.NewBvState("output_cols", 16);
    execute_statevars->output_rows = m.NewBvState("output_rows", 16);
}


void DefineConfigExecute(Ila& m, command_t& command, execute_statevars_t *execute_statevars){
    
    // Declare instruction
    auto config_ex = m.NewInstr("config_ex");
    
    // Define decode
    config_ex.SetDecode(command.funct == BvConst(CONFIG_CMD, INSTR_FUNCT_WIDTH) & Extract(command.rs1, 1, 0) == BvConst(CONFIG_EX, 2));

    // Define update
    config_ex.SetUpdate(execute_statevars->dataflow, Extract(command.rs1, 2, 2) == 1);
    config_ex.SetUpdate(execute_statevars->act, Extract(command.rs1,4,3));
    config_ex.SetUpdate(execute_statevars->sys_shift, Extract(command.rs2,31,0));
    config_ex.SetUpdate(execute_statevars->sys_acc_shift, Extract(command.rs1, 63, 32));
    config_ex.SetUpdate(execute_statevars->a_transpose, Extract(command.rs1,8, 8) == 1);
    config_ex.SetUpdate(execute_statevars->b_transpose, Extract(command.rs1, 9,9) == 1);
    config_ex.SetUpdate(execute_statevars->c_stride, Extract(command.rs2, 63, 48));
    config_ex.SetUpdate(execute_statevars->a_stride, Extract(command.rs1, 31, 16));
}

void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t *execute_statevars){
    // Declare instruction
    auto matmul_preload = m.NewInstr("matmul_preload");

    // Define decode
    matmul_preload.SetDecode(command.funct == BvConst(PRELOAD_CMD, INSTR_FUNCT_WIDTH));

    // Define update
    matmul_preload.SetUpdate(execute_statevars->preload_sp_addr, Extract(command.rs1, 31, 0));
    matmul_preload.SetUpdate(execute_statevars->output_sp_addr, Extract(command.rs2, 31, 0));
    matmul_preload.SetUpdate(execute_statevars->preload_cols, Extract(command.rs1, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH));
    matmul_preload.SetUpdate(execute_statevars->preload_rows, Extract(command.rs1, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16));
    matmul_preload.SetUpdate(execute_statevars->output_cols, Extract(command.rs2, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH));
    matmul_preload.SetUpdate(execute_statevars->output_rows, Extract(command.rs2, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16));

}

// Q: What do we need OS and WS versions for? 
// DefineMatmulComputePreloaded
// DefineMatmulComputeAccumulated
// To paste into function:
    // Declare instruction with m.NewInstr
    // Define decode with SetDecode
    // Define update with instr.SetUpdate

} // namespace Gemmini

} // namespace ilang