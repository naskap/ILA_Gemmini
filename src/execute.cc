#include <Gemmini/execute.h>

namespace ilang {

namespace Gemmini {

void DefineExecute(Ila& m, command_t& command, gemmini_memory_t memory){

    execute_statevars_t execute_statevars;
    
    DefineExecuteStatevars(m, execute_statevars);
    DefineConfigExecute(m, command, execute_statevars);
    DefineMatmulPreload(m, command, execute_statevars);
    
    // DefineComputeMatmul(m, command, execute_statevars, memory);
    DefineStoreOutputChild(m, command, execute_statevars, memory);
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

    // Compute parent statevars
    execute_statevars.systolic_array = m.NewMemState("systolic_array", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH); 
    execute_statevars.systolic_array.SetEntryNum(ARRAY_DIM);
    execute_statevars.ws_results = m.NewMemState("ws_results", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH); 
    execute_statevars.ws_results.SetEntryNum(ARRAY_DIM);
    execute_statevars.child_valid= m.NewBoolState("child_valid");

    // Compute child statevars
    execute_statevars.cur_row = m.NewBvState("cur_row", 32);
    execute_statevars.cur_col = m.NewBvState("cur_col", 32);
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

// void DefineComputeMatmul(Ila& m, command_t& command, execute_statevars_t &execute_statevars, gemmini_memory_t memory){
    
//     // Define compute matmul instruction for all possible input parameters
//     for(int preload = 0; preload <= 1; preload++){
//         for(int dataflow = 0; dataflow <= 1; dataflow++){
//             DefineComputeMatmulInstruction(m, command, execute_statevars, memory, preload, dataflow_t(dataflow));
//         }
//     }
    

// }

void DefineComputeMatmulInstruction(Ila& m, command_t& command, execute_statevars_t &execute_statevars, gemmini_memory_t memory, 
                                    bool preload, dataflow_t dataflow){
    
    // Extract command arguments
    compute_args_t compute_args;
    compute_args.a.addr  = Extract(command.rs1, 31, 0);
    compute_args.bd.addr = Extract(command.rs2, 31, 0);
    compute_args.a.cols  = Extract(command.rs1, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH);
    compute_args.a.rows  = Extract(command.rs1, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16);
    compute_args.bd.cols = Extract(command.rs2, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH);
    compute_args.bd.rows = Extract(command.rs2, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16);

    // Declare instruction with m.NewInstr
    auto compute = m.NewInstr(_BuildComputeMatmulInstrName(preload, dataflow));

    // Define decode
    auto compute_preload = (BoolConst(preload) & command.funct == BvConst(COMPUTE_PRELOADED_CMD, INSTR_FUNCT_WIDTH));
    auto valid_compute_preload = compute_preload & (Extract(execute_statevars.preload_sp_addr, 31,30) == BvConst(0,2) | 
                                                    (~execute_statevars.preload_sp_addr == BvConst(0, 32)));
    auto compute_accumulated = BoolConst(!preload) & (command.funct == BvConst(COMPUTE_ACCUMULATE_CMD, INSTR_FUNCT_WIDTH));
    compute.SetDecode((valid_compute_preload | compute_accumulated) & 
                     (BoolConst(dataflow) == execute_statevars.dataflow));

    
    auto systolic_array_next = preload ? _PreloadArray(memory.spad, execute_statevars) : execute_statevars.systolic_array;
    if(dataflow == dataflow_t::OS){
        systolic_array_next = _ComputeMatmulOS(memory.spad, execute_statevars, compute_args, systolic_array_next);
        compute.SetUpdate(execute_statevars.systolic_array, systolic_array_next);
    } else{
        auto ws_results = _InitializeWSResults(memory.spad, execute_statevars, compute_args.bd);
        ws_results = _ComputeMatmulWS(memory.spad, execute_statevars, compute_args, ws_results);
        compute.SetUpdate(execute_statevars.ws_results, ws_results);
    }
    
}

std::string _BuildComputeMatmulInstrName(bool preload, dataflow_t dataflow){
    std::string to_return  = "compute_matmul";
         to_return        += dataflow == dataflow_t::OS ? "_os" : "_ws";
         to_return        += preload ? "_preload" : "_nopreload";
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
        systolic_array_next = SetMemElmt(systolic_array_next, r, c, preload_value.SExt(ACC_TYPE_WIDTH_BITS));
      }
    }
    return systolic_array_next;

}


ExprRef _InitializeWSResults(ExprRef &spad, execute_statevars_t &execute_statevars, tile_compute_args_t &bd_args){
    
    auto results = MemConst(0, std::map<uint64_t, uint64_t>(), SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH);
    for (int i = 0; i < ARRAY_DIM; i++) {
      auto i_bv = BvConst(i, 32);

      for (int j = 0; j < ARRAY_DIM; j++) {
        auto j_bv = BvConst(j,32);

        auto store_zero = (~bd_args.addr) == BvConst(0, 32) | 
                        ((i_bv >= bd_args.rows.ZExt(i_bv.bit_width())) & (j_bv >= bd_args.cols.ZExt(i_bv.bit_width())));
        auto elmt   =  Ite(store_zero, BvConst(0, ACC_TYPE_WIDTH_BITS),
                                    GetBvElmt(spad.Load(i_bv), j_bv, INPUT_TYPE_WIDTH_BITS).SExt(ACC_TYPE_WIDTH_BITS));
        
      }

    }
    return results;
}

ExprRef _ComputeMatmulWS(ExprRef &spad, execute_statevars_t &execute_statevars, compute_args_t &compute_args, ExprRef &ws_results){
    for(int i = 0; i < ARRAY_DIM; i++){
        auto i_bv = BvConst(i,32);
        for(int j = 0; j < ARRAY_DIM; j++){
            auto j_bv = BvConst(j,32);
            for(int k=0; k < ARRAY_DIM; k++){
                auto k_bv = BvConst(k,32);
                auto a =_GetTileAElmt(spad, execute_statevars, compute_args.a, i_bv, k_bv);
                ws_results = SetMemElmt(ws_results, i_bv, j_bv, a.SExt(ACC_TYPE_WIDTH_BITS) * GetMemElmt(execute_statevars.systolic_array, k_bv, j_bv, ACC_TYPE_WIDTH_BITS));
            }
        }
    }
    return ws_results;

}

ExprRef _ComputeMatmulOS(ExprRef &spad, execute_statevars_t &execute_statevars, compute_args_t &compute_args, ExprRef &systolic_array){
    for(int i = 0; i < ARRAY_DIM; i++){
        auto i_bv = BvConst(i,32);
        for(int j = 0; j < ARRAY_DIM; j++){
            auto j_bv = BvConst(j,32);
            for(int k=0; k < ARRAY_DIM; k++){
                auto k_bv = BvConst(k,32);
                
                auto a = _GetTileAElmt(spad, execute_statevars, compute_args.a, i_bv, k_bv);

                // Get b tile element
                auto r = Ite(execute_statevars.b_transpose, j_bv, k_bv);
                auto c = Ite(execute_statevars.a_transpose, k_bv, j_bv);
                auto store_zero = (~compute_args.bd.addr == 0) | ((k_bv >= compute_args.bd.rows.ZExt(k_bv.bit_width())) | (j_bv >= compute_args.bd.cols.ZExt(j_bv.bit_width())));
                auto b = Ite(store_zero, BvConst(0, INPUT_TYPE_WIDTH_BITS),
                            GetMemElmt(spad, compute_args.bd.addr + r.ZExt(spad.addr_width()), c, INPUT_TYPE_WIDTH_BITS));

                // Accumulate into systolic array
                auto cur_elmt = GetMemElmt(systolic_array, i_bv, j_bv, ACC_TYPE_WIDTH_BITS);
                auto to_store = cur_elmt + a.SExt(ACC_TYPE_WIDTH_BITS) * b.SExt(ACC_TYPE_WIDTH_BITS);
                systolic_array = SetMemElmt(systolic_array, i_bv, j_bv, to_store);
           
            }
        }
    }
    return systolic_array;

}

ExprRef _GetTileAElmt(ExprRef &spad, execute_statevars_t &execute_statevars, tile_compute_args_t &a_args, ExprRef &i_bv, ExprRef &k_bv){
    auto r = execute_statevars.a_stride.ZExt(i_bv.bit_width()) * (Ite(execute_statevars.a_transpose, k_bv, i_bv));
    auto c = Ite(execute_statevars.a_transpose, i_bv, k_bv);
    
    auto rand_num = BvConst(rand() % ((1 << INPUT_TYPE_WIDTH_BITS) - 1), INPUT_TYPE_WIDTH_BITS);
    auto a = Ite(~a_args.addr == 0, rand_num,
                Ite((i_bv >= a_args.rows.ZExt(i_bv.bit_width())) | (k_bv >= a_args.cols.ZExt(k_bv.bit_width())), BvConst(0, INPUT_TYPE_WIDTH_BITS),
                                                    GetMemElmt(spad, a_args.addr + r.ZExt(spad.addr_width()), c, INPUT_TYPE_WIDTH_BITS)));
    return a;
}

void DefineStoreOutputChild(Ila &m, command_t &command, execute_statevars_t &execute_statevars, gemmini_memory_t &memory){
    auto store_child = m.NewChild("store_child");
    store_child.SetValid(execute_statevars.child_valid == 1);

    for(int dataflow = 0; dataflow <= 1; dataflow++){
        for(int acc_address = 0; acc_address <= 1; acc_address++){
            if(acc_address == 1){
                for(int accumulate_output = 0; accumulate_output <=1; accumulate_output++){
                    DefineStoreOutputInstruction(store_child, command, execute_statevars, memory, dataflow_t(dataflow), acc_address, accumulate_output);
                }
            }else{
                DefineStoreOutputInstruction(store_child, command, execute_statevars, memory, dataflow_t(dataflow), acc_address, false);
            }
        }
    }

}

void DefineStoreOutputInstruction(Ila &store_child, command_t &command, execute_statevars_t &execute_statevars, 
                        gemmini_memory_t &memory, dataflow_t const &dataflow, bool acc_address, bool accumulate_output){

    // Disable invalid options
    if(!acc_address){
        accumulate_output = false;
    }
    
    auto store_output = store_child.NewInstr(_BuildStoreOutputInstrName(dataflow, acc_address, accumulate_output));

    store_output.SetDecode((BoolConst(dataflow) == execute_statevars.dataflow) &
                     (BoolConst(acc_address) == (SelectBit(execute_statevars.output_sp_addr, 31) == 1)) &
                     (BoolConst(accumulate_output) == (SelectBit(execute_statevars.output_sp_addr, 30) == 1)));

    auto value = dataflow == dataflow_t::OS ? GetMemElmt(execute_statevars.systolic_array, execute_statevars.cur_row, execute_statevars.cur_col, ACC_TYPE_WIDTH_BITS) :
                                              GetMemElmt(execute_statevars.ws_results, execute_statevars.cur_row, execute_statevars.cur_col, ACC_TYPE_WIDTH_BITS);

    auto base_sp_address = Extract(execute_statevars.output_sp_addr, 28,0).ZExt(32);
    auto row_address = base_sp_address + execute_statevars.c_stride.ZExt(32) * execute_statevars.cur_row;

    auto shifted = dataflow == dataflow_t::OS ? _RoundingRightShift(value, execute_statevars.sys_shift) : 
                                                    _RoundingRightShift(value, BvConst(0, value.bit_width()));

    if(acc_address){
        
        shifted = CastBv(shifted, OUTPUT_TYPE_WIDTH_BITS);
    
        auto acc_next = memory.accumulator;
        if(accumulate_output){
            auto cur_elmt = GetMemElmt(memory.accumulator, row_address, execute_statevars.cur_col, ACC_TYPE_WIDTH_BITS);
            acc_next = SetMemElmt(memory.accumulator, row_address, execute_statevars.cur_col, cur_elmt + value);
        } else{
            acc_next = SetMemElmt(memory.accumulator, row_address, execute_statevars.cur_col, value);
        }
        store_output.SetUpdate(memory.accumulator, acc_next);

    } else{
        shifted = CastBv(shifted, INPUT_TYPE_WIDTH_BITS);

        auto activated = ApplyActivation(shifted, execute_statevars.act, INPUT_TYPE_WIDTH_BITS);

        auto spad_next = SetMemElmt(memory.spad, row_address, execute_statevars.cur_col, activated);
        store_output.SetUpdate(memory.spad, spad_next);
    }

    // Update state variables
    std::vector<ExprRef> iteration_vars = {execute_statevars.cur_row, 
                                            execute_statevars.cur_col};
    std::vector<ExprRef> iteration_maxs = {execute_statevars.output_rows, 
                                            execute_statevars.output_cols};
    
    auto last_pixel = IterateLoopVars(store_output, iteration_vars, iteration_maxs);
    store_output.SetUpdate(execute_statevars.child_valid, !(last_pixel));

}

std::string _BuildStoreOutputInstrName(dataflow_t const &dataflow, bool acc_address, bool accumulate_output){
    std::string to_return  = "store_matmul_output";
         to_return        += dataflow == dataflow_t::OS ? "_os" : "_ws";
         to_return        += acc_address ? (accumulate_output ? "_toAcc_accumulate" : "_toAcc_overwrite") : "_toSpad";

    ILA_INFO << to_return;
    
    return to_return;
}


ExprRef _RoundingRightShift(ExprRef const &x, ExprRef const &shift){
    ILA_ASSERT(x.bit_width() == shift.bit_width()); // Can cast later if this becomes a nuisance
    unsigned int bw = x.bit_width();
    auto simple_shift = x >> shift;
    auto msb_shifted_out = (x >> (shift - 1)) & BvConst(1, bw);
    auto other_ones_shifted_out = ((x) & ((BvConst(1, bw) << ((shift)-1)) - 1)) & BvConst(1, bw);
    auto new_lsb_is_one = (simple_shift & BvConst(1, bw)) & BvConst(1, bw);
    auto round_factor = Ite(shift <= 1, BvConst(0, x.bit_width()), 
                                msb_shifted_out & (other_ones_shifted_out | new_lsb_is_one));
    return simple_shift + round_factor;
}
    // // Disable nonapplicable options if destination address is scratchpad
    // 

// }

// ExprRef _GetAMatrixVal(ExprRef &spad, execute_statevars_t &execute_statevars, ExprRef &a_addr, ExprRef &a_rows, ExprRef &a_cols){

// }

// Q: What do we need OS and WS versions for? 
// DefineMatmulComputePreloaded
// DefineMatmulComputeAccumulated
// To paste into function:
    // Declare instruction with m.NewInstr
    // Define decode with SetDecode
    // Define update with instr.SetUpdate

} // namespace Gemmini

} // namespace ilang