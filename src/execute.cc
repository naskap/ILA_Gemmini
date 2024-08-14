#include <Gemmini/execute.h>

namespace ilang {

namespace Gemmini {

void DefineExecute(Ila& m, command_t& command, gemmini_memory_t memory){

    execute_statevars_t svs;
    
    DefineExecuteStatevars(m, svs);
    DefineConfigExecute(m, command, svs);
    DefineMatmulPreload(m, command, svs);
    
    DefineComputeMatmul(m, command, svs, memory);
    // DefineStoreOutputChild(m, command, svs, memory);
}

void DefineExecuteStatevars(Ila& m, execute_statevars_t &svs){
    // Config execute statevars
    svs.dataflow      = m.NewBoolState("dataflow");
    svs.act           = m.NewBvState("act", 2);
    svs.sys_shift     = m.NewBvState("sys_shift", 32);
    svs.sys_acc_shift = m.NewBvState("sys_acc_shift", 32);
    svs.a_transpose   = m.NewBoolState("a_transpose");
    svs.b_transpose   = m.NewBoolState("b_transpose");
    svs.c_stride      = m.NewBvState("c_stride", 16);
    svs.a_stride      = m.NewBvState("a_stride", 16);

    // Preload statevars
    svs.preload_sp_addr = m.NewBvState("preload_sp_addr", 32);
    svs.output_sp_addr  = m.NewBvState("output_sp_addr", 32);
    svs.preload_cols    = m.NewBvState("preload_cols", 16);
    svs.preload_rows    = m.NewBvState("preload_rows", 16);
    svs.output_cols     = m.NewBvState("output_cols", 16);
    svs.output_rows     = m.NewBvState("output_rows", 16);

    // Compute parent statevars
    svs.systolic_array = m.NewMemState("systolic_array", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH); 
    svs.systolic_array.SetEntryNum(ARRAY_DIM);
    svs.ws_results = m.NewMemState("ws_results", SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH); 
    svs.ws_results.SetEntryNum(ARRAY_DIM);
    svs.child_state = m.NewBvState("child_state", 8);

    // Compute child statevars
    svs.i = m.NewBvState("i", 32);
    svs.j = m.NewBvState("j", 32);
    svs.k = m.NewBvState("k", 32);
}


void DefineConfigExecute(Ila& m, command_t& command, execute_statevars_t &svs){
    
    // Declare instruction
    auto config_ex = m.NewInstr("config_ex");
    
    // Define decode
    config_ex.SetDecode(command.funct == BvConst(CONFIG_CMD, INSTR_FUNCT_WIDTH) & Extract(command.rs1, 1, 0) == BvConst(CONFIG_EX, 2));

    // Define update
    config_ex.SetUpdate(svs.dataflow, SelectBit(command.rs1, 2) == 1);
    config_ex.SetUpdate(svs.act, Extract(command.rs1,4,3));
    config_ex.SetUpdate(svs.sys_shift, Extract(command.rs2,31,0));
    config_ex.SetUpdate(svs.sys_acc_shift, Extract(command.rs1, 63, 32));
    config_ex.SetUpdate(svs.a_transpose, Extract(command.rs1,8, 8) == 1);
    config_ex.SetUpdate(svs.b_transpose, Extract(command.rs1, 9,9) == 1);
    config_ex.SetUpdate(svs.c_stride, Extract(command.rs2, 63, 48));
    config_ex.SetUpdate(svs.a_stride, Extract(command.rs1, 31, 16));
}

void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t &svs){
    // Declare instruction
    auto matmul_preload = m.NewInstr("matmul_preload");

    // Define decode
    matmul_preload.SetDecode(command.funct == BvConst(PRELOAD_CMD, INSTR_FUNCT_WIDTH));

    // Define update
    matmul_preload.SetUpdate(svs.preload_sp_addr, Extract(command.rs1, 31, 0));
    matmul_preload.SetUpdate(svs.output_sp_addr, Extract(command.rs2, 31, 0));
    matmul_preload.SetUpdate(svs.preload_cols, Extract(command.rs1, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH));
    matmul_preload.SetUpdate(svs.preload_rows, Extract(command.rs1, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16));
    matmul_preload.SetUpdate(svs.output_cols, Extract(command.rs2, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH));
    matmul_preload.SetUpdate(svs.output_rows, Extract(command.rs2, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16));

}

void DefineComputeMatmul(Ila& m, command_t& command, execute_statevars_t &svs, gemmini_memory_t memory){
    
    // Declare instruction
    auto compute_matmul = m.NewInstr("compute_matmul");

    // Set decode
    auto compute_preload = (command.funct == BvConst(COMPUTE_PRELOADED_CMD, INSTR_FUNCT_WIDTH));
    auto valid_compute_preload = compute_preload & (Extract(svs.preload_sp_addr, 31,30) == BvConst(0,2) | 
                                                    (~svs.preload_sp_addr == BvConst(0, 32)));
    auto compute_accumulated = (command.funct == BvConst(COMPUTE_ACCUMULATE_CMD, INSTR_FUNCT_WIDTH));
    compute_matmul.SetDecode(valid_compute_preload | compute_accumulated);

    // Trigger child ILA
    auto child_state_next = Ite(compute_preload, BvConst(compute_child_states::PRELOAD, 8), 
                            Ite(svs.dataflow == BoolConst(dataflow_t::OS),  BvConst(compute_child_states::OS_COMPUTE,8),
                                                                                        BvConst(compute_child_states::INITIALIZE_WS_RESULTS,8)));
    compute_matmul.SetUpdate(svs.child_state, child_state_next);
    compute_matmul.SetUpdate(svs.i, BvConst(0, 32));
    compute_matmul.SetUpdate(svs.j, BvConst(0, 32));
    compute_matmul.SetUpdate(svs.k, BvConst(0, 32));
    // compute_matmul.SetUpdate(svs.ws_results, MemConst(0, std::map<uint64_t, uint64_t>(), SPAD_ADDRESS_WIDTH, ACC_ROW_WIDTH));

    // Declare child
    auto child = m.NewChild("compute_child");
    child.SetValid(svs.child_state != BvConst(compute_child_states::INACTIVE, 8));

    // Extract command arguments
    compute_args_t compute_args;
    compute_args.a.addr  = Extract(command.rs1, 31, 0);
    compute_args.bd.addr = Extract(command.rs2, 31, 0);
    compute_args.a.cols  = Extract(command.rs1, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH);
    compute_args.a.rows  = Extract(command.rs1, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16);
    compute_args.bd.cols = Extract(command.rs2, SPAD_ADDRESS_WIDTH + 15, SPAD_ADDRESS_WIDTH);
    compute_args.bd.rows = Extract(command.rs2, SPAD_ADDRESS_WIDTH + 31, SPAD_ADDRESS_WIDTH + 16);

    // Define child instructions
    DefinePreload(child, memory.spad, svs);
    DefineInitializeWSResults(child, memory.spad, svs, compute_args.bd);
    DefineMatmulWS(child, memory.spad, svs, compute_args);
    DefineMatmulOS(child, memory.spad, svs, compute_args);
    for(int dataflow = 0; dataflow <= 1; dataflow++){
        for(int acc_address = 0; acc_address <= 1; acc_address++){
            if(acc_address == 1){
                for(int accumulate_output = 0; accumulate_output <=1; accumulate_output++){
                    DefineStoreOutputInstruction(child, command, svs, memory, dataflow_t(dataflow), acc_address, accumulate_output);
                }
            }else{
                DefineStoreOutputInstruction(child, command, svs, memory, dataflow_t(dataflow), acc_address, false);
            }
        }
    }
    
}


std::string _BuildComputeMatmulInstrName(bool preload, dataflow_t dataflow){
    std::string to_return  = "compute_matmul";
         to_return        += dataflow == dataflow_t::OS ? "_os" : "_ws";
         to_return        += preload ? "_preload" : "_nopreload";
    ILA_INFO << to_return;
    
    return to_return;
}

void DefinePreload(Ila &child, ExprRef &spad, execute_statevars_t &svs){

    auto preload = child.NewInstr("preload_systolic_array");
    preload.SetDecode(svs.child_state == BvConst(compute_child_states::PRELOAD,8));

    // Get preload value to set
    auto preload_transpose = (svs.dataflow == BoolConst(dataflow_t::WS)) & svs.b_transpose;  
    auto r = Ite(preload_transpose, svs.j, svs.i);
    auto c = Ite(preload_transpose, svs.i, svs.j);
    auto load_cur_elmt = (svs.i < svs.preload_rows.SExt(32)) & 
                         (svs.j < svs.preload_cols.SExt(32));
    auto load_zero = ~svs.preload_sp_addr == 0;
    auto preload_value = Ite(!load_cur_elmt | load_zero, BvConst(0, INPUT_TYPE_WIDTH_BITS), 
                            GetMemElmt(spad, r + svs.preload_sp_addr, c, INPUT_TYPE_WIDTH_BITS));
                
    
    // Store new systolic array row
    auto systolic_array_next = SetMemElmt(svs.systolic_array, svs.i, svs.j, preload_value.SExt(ACC_TYPE_WIDTH_BITS));
    preload.SetUpdate(svs.systolic_array, systolic_array_next);


    // Update iteration variables
    std::vector<ExprRef> iteration_vars = {svs.i, 
                                            svs.j};
    std::vector<ExprRef> iteration_maxs = {BvConst(ARRAY_DIM, 32), 
                                            BvConst(ARRAY_DIM, 32)};
    
    auto last_pixel = IterateLoopVars(preload, iteration_vars, iteration_maxs);

    // Set next state
    auto child_state_next = Ite(last_pixel, 
                                Ite(svs.dataflow == dataflow_t::OS, 
                                    BvConst(compute_child_states::OS_COMPUTE,8), 
                                    BvConst(compute_child_states::INITIALIZE_WS_RESULTS,8)
                                    ), 
                                BvConst(compute_child_states::PRELOAD, 8)
                                );
    preload.SetUpdate(svs.child_state, child_state_next);

}


void DefineInitializeWSResults(Ila &child, ExprRef &spad, execute_statevars_t &svs, tile_compute_args_t &bd_args){
    
    // Declare instruction
    auto init_ws_results = child.NewInstr("init_ws_results");
    init_ws_results.SetDecode(svs.child_state == BvConst(compute_child_states::INITIALIZE_WS_RESULTS,8));

    // Get seed element from spad
    auto bw = svs.i.bit_width();
    auto store_zero = (~bd_args.addr) == BvConst(0, 32) | 
                    ((svs.i >= bd_args.rows.ZExt(bw)) | (svs.j >= bd_args.cols.ZExt(bw)));
    auto elmt   =  Ite(store_zero, BvConst(0, ACC_TYPE_WIDTH_BITS), 
                GetMemElmt(spad, bd_args.addr + svs.i, svs.j, INPUT_TYPE_WIDTH_BITS).SExt(ACC_TYPE_WIDTH_BITS));
    
    // Store elemt
    auto ws_results_next = SetMemElmt(svs.ws_results, svs.i, svs.j, elmt);
    init_ws_results.SetUpdate(svs.ws_results, ws_results_next);


    // Update iteration variables
    std::vector<ExprRef> iteration_vars = {svs.i, 
                                            svs.j};
    std::vector<ExprRef> iteration_maxs = {BvConst(ARRAY_DIM, 32), 
                                            BvConst(ARRAY_DIM, 32)};
    
    auto last_pixel = IterateLoopVars(init_ws_results, iteration_vars, iteration_maxs);

    // Set next state
    auto child_state_next = Ite(last_pixel,  BvConst(compute_child_states::WS_COMPUTE, 8),
                                BvConst(compute_child_states::INITIALIZE_WS_RESULTS, 8));
    init_ws_results.SetUpdate(svs.child_state, child_state_next);
 
}

void DefineMatmulWS(Ila &child, ExprRef &spad, execute_statevars_t &svs, compute_args_t &compute_args){
    
    auto matmul_ws = child.NewInstr("matmul_ws");
    matmul_ws.SetDecode(svs.child_state == BvConst(compute_child_states::WS_COMPUTE,8));
    
    // Retrieve elements
    auto a =_GetTileAElmt(spad, svs, compute_args.a, svs.i, svs.k);
    auto sys_array_elmt = GetMemElmt(svs.systolic_array, svs.k, svs.j, ACC_TYPE_WIDTH_BITS);
    auto cur_elmt = GetMemElmt(svs.ws_results, svs.i, svs.j, ACC_TYPE_WIDTH_BITS);

    // Compute and store ws_results update
    auto ws_results_next = SetMemElmt(svs.ws_results, svs.i, svs.j, cur_elmt + a.SExt(ACC_TYPE_WIDTH_BITS) * sys_array_elmt);
    matmul_ws.SetUpdate(svs.ws_results, ws_results_next);

    // Update iteration variables
    std::vector<ExprRef> iteration_vars = {svs.i, svs.j, svs.k};
    std::vector<ExprRef> iteration_maxs = {BvConst(ARRAY_DIM, 32), BvConst(ARRAY_DIM, 32), BvConst(ARRAY_DIM, 32)};
    
    auto last_pixel = IterateLoopVars(matmul_ws, iteration_vars, iteration_maxs);

    // Set next state
    auto child_state_next = Ite(last_pixel,  BvConst(compute_child_states::OUTPUT_RESULTS, 8),
                                BvConst(compute_child_states::WS_COMPUTE, 8));
    matmul_ws.SetUpdate(svs.child_state, child_state_next);

}

void DefineMatmulOS(Ila &child, ExprRef &spad, execute_statevars_t &svs, compute_args_t &compute_args){

    auto matmul_os = child.NewInstr("matmul_os");
    matmul_os.SetDecode(svs.child_state == BvConst(compute_child_states::OS_COMPUTE,8));

    auto a = _GetTileAElmt(spad, svs, compute_args.a, svs.i, svs.k);

    // Get b tile element
    auto r = Ite(svs.b_transpose, svs.j, svs.k);
    auto c = Ite(svs.b_transpose, svs.k, svs.j);
    auto store_zero = (~compute_args.bd.addr == 0) | ((svs.k >= compute_args.bd.rows.ZExt(svs.k.bit_width())) | (svs.j >= compute_args.bd.cols.ZExt(svs.j.bit_width())));
    auto b = Ite(store_zero, BvConst(0, INPUT_TYPE_WIDTH_BITS),
                GetMemElmt(spad, compute_args.bd.addr + r.ZExt(spad.addr_width()), c, INPUT_TYPE_WIDTH_BITS));

    // Accumulate into systolic array
    auto cur_elmt = GetMemElmt(svs.systolic_array, svs.i, svs.j, ACC_TYPE_WIDTH_BITS);
    auto to_store = cur_elmt + a.SExt(ACC_TYPE_WIDTH_BITS) * b.SExt(ACC_TYPE_WIDTH_BITS);
    auto systolic_array_next = SetMemElmt(svs.systolic_array, svs.i, svs.j, to_store);
    matmul_os.SetUpdate(svs.systolic_array, systolic_array_next);
           
    // Update iteration variables
    std::vector<ExprRef> iteration_vars = {svs.i, svs.j, svs.k};
    std::vector<ExprRef> iteration_maxs = {BvConst(ARRAY_DIM, 32), BvConst(ARRAY_DIM, 32), BvConst(ARRAY_DIM, 32)};
    auto last_pixel = IterateLoopVars(matmul_os, iteration_vars, iteration_maxs);
    
    // Update next state
    auto child_state_next = Ite(last_pixel,  BvConst(compute_child_states::OUTPUT_RESULTS, 8),
                                BvConst(compute_child_states::OS_COMPUTE, 8));
    matmul_os.SetUpdate(svs.child_state, child_state_next);

}

ExprRef _GetTileAElmt(ExprRef &spad, execute_statevars_t &svs, tile_compute_args_t &a_args, ExprRef &i_bv, ExprRef &k_bv){
    auto r = svs.a_stride.ZExt(i_bv.bit_width()) * (Ite(svs.a_transpose, k_bv, i_bv));
    auto c = Ite(svs.a_transpose, i_bv, k_bv);
    
    auto rand_num = BvConst(rand() % ((1 << INPUT_TYPE_WIDTH_BITS) - 1), INPUT_TYPE_WIDTH_BITS);
    auto a = Ite(~a_args.addr == 0, rand_num,
                Ite((i_bv >= a_args.rows.ZExt(i_bv.bit_width())) | (k_bv >= a_args.cols.ZExt(k_bv.bit_width())), BvConst(0, INPUT_TYPE_WIDTH_BITS),
                                                    GetMemElmt(spad, a_args.addr + r.ZExt(spad.addr_width()), c, INPUT_TYPE_WIDTH_BITS)));
    return a;
}



void DefineStoreOutputInstruction(Ila &store_child, command_t &command, execute_statevars_t &svs, 
                        gemmini_memory_t &memory, dataflow_t const &dataflow, bool acc_address, bool accumulate_output){

    // Disable invalid options
    if(!acc_address){
        accumulate_output = false;
    }
    
    // Declare instruction
    auto store_output = store_child.NewInstr(_BuildStoreOutputInstrName(dataflow, acc_address, accumulate_output));
    
    // Set decode
    store_output.SetDecode((BvConst(compute_child_states::OUTPUT_RESULTS, 8) == svs.child_state) &
                     (BoolConst(dataflow) == svs.dataflow) &
                     (BoolConst(acc_address) == (SelectBit(svs.output_sp_addr, 31) == 1)) &
                     (BoolConst(accumulate_output) == (SelectBit(svs.output_sp_addr, 30) == 1)));


    auto value = dataflow == dataflow_t::OS ? GetMemElmt(svs.systolic_array, svs.i, svs.j, ACC_TYPE_WIDTH_BITS) :
                                              GetMemElmt(svs.ws_results, svs.i, svs.j, ACC_TYPE_WIDTH_BITS);

    auto base_sp_address = Extract(svs.output_sp_addr, 28,0).ZExt(32);
    auto row_address = base_sp_address + svs.c_stride.ZExt(32) * svs.i;

    auto shifted = dataflow == dataflow_t::OS ? _RoundingRightShift(value, svs.sys_shift) : 
                                                    _RoundingRightShift(value, BvConst(0, value.bit_width()));

    if(acc_address){
        
        shifted = CastBv(shifted, OUTPUT_TYPE_WIDTH_BITS);
    
        auto acc_next = memory.accumulator;
        if(accumulate_output){
            auto cur_elmt = GetMemElmt(memory.accumulator, row_address, svs.j, ACC_TYPE_WIDTH_BITS);
            acc_next = SetMemElmt(memory.accumulator, row_address, svs.j, cur_elmt + value);
        } else{
            acc_next = SetMemElmt(memory.accumulator, row_address, svs.j, value);
        }
        store_output.SetUpdate(memory.accumulator, acc_next);

    } else{
        shifted = CastBv(shifted, INPUT_TYPE_WIDTH_BITS);

        auto activated = ApplyActivation(shifted, svs.act, INPUT_TYPE_WIDTH_BITS);

        auto spad_next = SetMemElmt(memory.spad, row_address, svs.j, activated);
        store_output.SetUpdate(memory.spad, spad_next);
    }

    // Update iteration variables
    std::vector<ExprRef> iteration_vars = {svs.i, 
                                            svs.j};
    std::vector<ExprRef> iteration_maxs = {svs.output_rows, 
                                            svs.output_cols};
    
    auto last_pixel = IterateLoopVars(store_output, iteration_vars, iteration_maxs);
    
    // Update next state
    auto child_state_next = Ite(last_pixel,  BvConst(compute_child_states::INACTIVE, 8),
                                BvConst(compute_child_states::OUTPUT_RESULTS, 8));
    store_output.SetUpdate(svs.child_state, child_state_next);

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
    auto other_ones_shifted_out = Ite(((x) & ((BvConst(1, bw) << ((shift)-1)) - 1)) != 0, BvConst(1,bw), BvConst(0,bw)) ;
    auto new_lsb_is_one = (simple_shift & BvConst(1, bw));
    auto round_factor = Ite(shift <= 1, BvConst(0, x.bit_width()), 
                                msb_shifted_out & (other_ones_shifted_out | new_lsb_is_one));
    return simple_shift + round_factor;
}
    // // Disable nonapplicable options if destination address is scratchpad
    // 

// }

// ExprRef _GetAMatrixVal(ExprRef &spad, execute_statevars_t &svs, ExprRef &a_addr, ExprRef &a_rows, ExprRef &a_cols){

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