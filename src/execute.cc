#include <Gemmini/execute.h>

namespace ilang {

namespace Gemmini {

void DefineExecute(Ila& m, command_t& command, gemmini_memory_t memory){

    execute_statevars_t execute_statevars;
    
    DefineExecuteStatevars(m, execute_statevars);
    DefineConfigExecute(m, command, execute_statevars);
    DefineMatmulPreload(m, command, execute_statevars);
    DefineComputeMatmul(m, command, execute_statevars, memory);
}

void DefineExecuteStatevars(Ila& m, execute_statevars_t &execute_statevars){
    // Config execute statevars
    execute_statevars.dataflow      = m.NewBoolState("dataflow");
    execute_statevars.act           = m.NewBvState("act", 2);
    execute_statevars.sys_shift     = m.NewBvState("sys_shift", 32);
    execute_statevars.sys_acc_shift = m.NewBvState("sys_acc_shift", 32);
    execute_statevars.a_transpose   = m.NewBoolState("a_transpose");
    execute_statevars.b_transpose   = m.NewBoolState("b_transpose");
    execute_statevars.c_stride      = m.NewBvState("c_stride", 16);
    execute_statevars.a_stride      = m.NewBvState("a_stride", 16);

    // Preload statevars
    execute_statevars.preload_sp_addr = m.NewBvState("preload_sp_addr", 32);
    execute_statevars.output_sp_addr  = m.NewBvState("output_sp_addr", 32);
    execute_statevars.preload_cols    = m.NewBvState("preload_cols", 16);
    execute_statevars.preload_rows    = m.NewBvState("preload_rows", 16);
    execute_statevars.output_cols     = m.NewBvState("output_cols", 16);
    execute_statevars.output_rows     = m.NewBvState("output_rows", 16);

    // Compute statevars
    execute_statevars.systolic_array = m.NewMemState("execute_statevars.systolic_array", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH); 
    execute_statevars.systolic_array.SetEntryNum(ARRAY_DIM);
}


void DefineConfigExecute(Ila& m, command_t& command, execute_statevars_t &execute_statevars){
    
    // Declare instruction
    auto config_ex = m.NewInstr("config_ex");
    
    // Define decode
    config_ex.SetDecode(command.funct == BvConst(CONFIG_CMD, INSTR_FUNCT_WIDTH) & Extract(command.rs1, 1, 0) == BvConst(CONFIG_EX, 2));

    // Define update
    config_ex.SetUpdate(execute_statevars.dataflow, SelectBit(command.rs1, 2) == 1);
    config_ex.SetUpdate(execute_statevars.act, Extract(command.rs1,4,3));
    config_ex.SetUpdate(execute_statevars.sys_shift, Extract(command.rs2,31,0));
    config_ex.SetUpdate(execute_statevars.sys_acc_shift, Extract(command.rs1, 63, 32));
    config_ex.SetUpdate(execute_statevars.a_transpose, Extract(command.rs1,8, 8) == 1);
    config_ex.SetUpdate(execute_statevars.b_transpose, Extract(command.rs1, 9,9) == 1);
    config_ex.SetUpdate(execute_statevars.c_stride, Extract(command.rs2, 63, 48));
    config_ex.SetUpdate(execute_statevars.a_stride, Extract(command.rs1, 31, 16));
}

void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t &execute_statevars){
    // Declare instruction
    auto matmul_preload = m.NewInstr("matmul_preload");

    // Define decode
    matmul_preload.SetDecode(command.funct == BvConst(PRELOAD_CMD, INSTR_FUNCT_WIDTH));

    // Define update
    matmul_preload.SetUpdate(execute_statevars.preload_sp_addr, Extract(command.rs1, 31, 0));
    matmul_preload.SetUpdate(execute_statevars.output_sp_addr, Extract(command.rs2, 31, 0));
    matmul_preload.SetUpdate(execute_statevars.preload_cols, Extract(command.rs1, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH));
    matmul_preload.SetUpdate(execute_statevars.preload_rows, Extract(command.rs1, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16));
    matmul_preload.SetUpdate(execute_statevars.output_cols, Extract(command.rs2, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH));
    matmul_preload.SetUpdate(execute_statevars.output_rows, Extract(command.rs2, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16));

}

void DefineComputeMatmul(Ila& m, command_t& command, execute_statevars_t &execute_statevars, gemmini_memory_t memory){
    
    // Define compute matmul instruction for all possible input parameters
    for(int preload = 0; preload <= 1; preload++){
        for(int dataflow = 0; dataflow <= 1; dataflow++){
            for(int acc_address = 0; acc_address <= 1; acc_address++){
                if(acc_address == 1){
                    for(int accumulate_output = 0; accumulate_output <=1; accumulate_output++){
                        DefineComputeMatmulInstruction(m, command, execute_statevars, memory, preload, dataflow_t(dataflow), acc_address, accumulate_output);
                    }
                }else{
                    DefineComputeMatmulInstruction(m, command, execute_statevars, memory, preload, dataflow_t(dataflow), acc_address, false);
                }
            }
        }
    }
    

}

void DefineComputeMatmulInstruction(Ila& m, command_t& command, execute_statevars_t &execute_statevars, gemmini_memory_t memory, 
                                    bool preload, dataflow_t dataflow, bool acc_addr, bool accumulate_output){

     // Disable nonapplicable options if destination address is scratchpad
    if(!acc_addr){
        accumulate_output = false;
    }

    // Extract command arguments
    // TODO: Potentially organize into structs a_specs and b_specs
    auto a_addr  = Extract(command.rs1, 31, 0);
    auto bd_addr = Extract(command.rs2, 31, 0);
    auto a_cols  = Extract(command.rs1, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH);
    auto a_rows  = Extract(command.rs1, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16);
    auto bd_cols = Extract(command.rs2, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH);
    auto bd_rows = Extract(command.rs2, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16);

    // Declare instruction with m.NewInstr
    auto compute = m.NewInstr(_BuildInstrName(preload, dataflow, acc_addr, accumulate_output));

    // Define decode
    auto compute_preload = (BoolConst(preload) & command.funct == BvConst(COMPUTE_PRELOADED_CMD, INSTR_FUNCT_WIDTH));
    auto valid_compute_preload = compute_preload & (Extract(execute_statevars.preload_sp_addr, 31,30) == BvConst(0,2) | 
                                                    (~execute_statevars.preload_sp_addr == BvConst(0, 32)));
    auto compute_accumulated = BoolConst(!preload) & (command.funct == BvConst(COMPUTE_ACCUMULATE_CMD, INSTR_FUNCT_WIDTH));
    compute.SetDecode((valid_compute_preload | compute_accumulated) & 
                     (BoolConst(dataflow) == execute_statevars.dataflow) &
                     (BoolConst(acc_addr) == (Extract(execute_statevars.output_sp_addr, 31, 31) == 1)) &
                     (BoolConst(accumulate_output) ==  (Extract(execute_statevars.output_sp_addr, 30, 30) == 1)));

    if(preload)
        auto systolic_array_next = _PreloadArray(memory.spad, execute_statevars);

    auto ws_results = _InitializeWSResults(memory.spad, execute_statevars, bd_addr, bd_rows, bd_cols);
    

}

std::string _BuildInstrName(bool preload, dataflow_t dataflow, bool acc_addr,
                            bool accumulate_output){
    std::string to_return  = "compute_matmul";
         to_return        += dataflow == dataflow_t::OS ? "_os" : "_ws";
         to_return        += preload ? "_preload" : "_nopreload";
         to_return        += acc_addr ? (accumulate_output ? "_toAcc_accumulate" : "_toAcc_overwrite") : "_toSpad";
    ILA_INFO << to_return;
    
    return to_return;
}

ExprRef _PreloadArray(ExprRef &spad, execute_statevars_t &execute_statevars){

    auto systolic_array_next = execute_statevars.systolic_array;
    for (int i = 0; i < ARRAY_DIM; i++) {
      auto i_bv = BvConst(i, 32);

      for (int j = 0; j < ARRAY_DIM; j++) {
        auto j_bv = BvConst(j,32);

        auto preload_transpose = (execute_statevars.dataflow == BoolConst(dataflow_t::WS)) & execute_statevars.b_transpose;  

        auto r = Ite(preload_transpose, j_bv, i_bv);
        auto c = Ite(preload_transpose, i_bv, j_bv);

        // Get preload value to set
        auto load_cur_elmt = (i_bv < execute_statevars.preload_rows.SExt(32)) & (j_bv < execute_statevars.preload_cols.SExt(32));
        auto load_zero = execute_statevars.preload_sp_addr == 0;
        auto preload_value = Ite(!load_cur_elmt | load_zero, BvConst(0, INPUT_TYPE_WIDTH_BITS), 
                                GetMemElmt(spad, r + execute_statevars.preload_sp_addr, c, INPUT_TYPE_WIDTH_BITS));
                    
        
        // Store new systolic array row
        auto systolic_array_next = SetMemElmt(execute_statevars.systolic_array, r, c, preload_value.SExt(ACC_TYPE_WIDTH_BITS));
      }
    }
    return systolic_array_next;

}


ExprRef _InitializeWSResults(ExprRef &spad, execute_statevars_t &execute_statevars, ExprRef &bd_addr, ExprRef &bd_rows, ExprRef &bd_cols){
    
    auto results = MemConst(0, std::map<uint64_t, uint64_t>(), SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH);
    for (int i = 0; i < ARRAY_DIM; i++) {
      auto i_bv = BvConst(i, 32);

      for (int j = 0; j < ARRAY_DIM; j++) {
        auto j_bv = BvConst(j,32);

        auto store_zero = (~bd_addr) == BvConst(0, 32) | 
                        ((i_bv >= bd_rows.ZExt(i_bv.bit_width())) & (j_bv >= bd_cols.ZExt(i_bv.bit_width())));
        auto elmt   =  Ite(store_zero, BvConst(0, ACC_TYPE_WIDTH_BITS),
                                    GetBvElmt(spad.Load(i_bv), j_bv, INPUT_TYPE_WIDTH_BITS).SExt(ACC_TYPE_WIDTH_BITS));
        

      }

    }
    return results;
    

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