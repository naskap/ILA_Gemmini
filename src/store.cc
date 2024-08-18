
#include <Gemmini/store.h>

namespace ilang {

namespace Gemmini {


void DefineStore(Ila& m, command_t& command, gemmini_memory_t memory, store_statevars_t &store_statevars) {

  DefineStoreStateVars(m, store_statevars);
  DefineConfigStoreInstruction(m, command, store_statevars);
  DefineStoreInstruction(m, command, memory, store_statevars);
}

void DefineStoreStateVars(Ila& m, store_statevars_t& store_statevars) {

    // Store configs
    store_statevars.stride                           = m.NewBvState("store_stride", 32);
    store_statevars.accScale                         = m.NewBvState("store_accScale", 32);
    store_statevars.activation                       = m.NewBvState("store_activation", 2);
    store_statevars.maxpool_params.enable_and_stride = m.NewBvState("store_maxpool_enable_and_stride", 2);
    store_statevars.maxpool_params.window_size       = m.NewBvState("store_maxpool_window_size", 2);
    store_statevars.maxpool_params.upper_pad         = m.NewBvState("store_maxpool_upper_pad", 2);
    store_statevars.maxpool_params.left_pad          = m.NewBvState("store_maxpool_left_pad", 2);
    store_statevars.maxpool_params.out_dim           = m.NewBvState("store_maxpool_out_dim", 8);
    store_statevars.maxpool_params.in_rows           = m.NewBvState("store_maxpool_in_rows", 8);
    store_statevars.maxpool_params.in_cols           = m.NewBvState("store_maxpool_in_cols", 8);
    store_statevars.maxpool_params.out_rows          = m.NewBvState("store_maxpool_out_rows", 8);
    store_statevars.maxpool_params.out_cols          = m.NewBvState("store_maxpool_out_cols", 8);

    // Store child helper variables
    store_statevars.child_valid = m.NewBoolState("store_child_valid");
    store_statevars.cur_row     = m.NewBvState("store_cur_row", 16);
    store_statevars.cur_col     = m.NewBvState("store_cur_col", 16);
    store_statevars.cur_ch      = m.NewBvState("store_cur_ch", 16);
    store_statevars.cur_wrow    = m.NewBvState("store_cur_wrow", 16);
    store_statevars.cur_wcol    = m.NewBvState("store_cur_wcol", 16);
    store_statevars.cur_max     = m.NewBvState("store_cur_max", ACC_TYPE_WIDTH_BITS);

}

void DefineConfigStoreInstruction(Ila& m, command_t command,
                                  store_statevars_t& store_statevars) {

    // Declare instruction
    auto config_store = m.NewInstr("CONFIG_STORE");

    // Define decode
    config_store.SetDecode((command.funct == 0) &
                           (Extract(command.rs1, 1, 0) == 2));

    // Define update functions
    config_store.SetUpdate(store_statevars.accScale, Extract(command.rs2,63,32));
    config_store.SetUpdate(store_statevars.stride, Extract(command.rs2,31,0));
    config_store.SetUpdate(store_statevars.activation, Extract(command.rs1,3,2));
    config_store.SetUpdate(store_statevars.maxpool_params.enable_and_stride, Extract(command.rs1, 5,4));
    config_store.SetUpdate(store_statevars.maxpool_params.window_size, Extract(command.rs1, 7,6));
    config_store.SetUpdate(store_statevars.maxpool_params.upper_pad, Extract(command.rs1, 9,8));
    config_store.SetUpdate(store_statevars.maxpool_params.left_pad, Extract(command.rs1, 11,10));
    config_store.SetUpdate(store_statevars.maxpool_params.out_dim, Extract(command.rs1, 31,24));
    config_store.SetUpdate(store_statevars.maxpool_params.in_rows, Extract(command.rs1, 39,32));
    config_store.SetUpdate(store_statevars.maxpool_params.in_cols, Extract(command.rs1, 47,40));
    config_store.SetUpdate(store_statevars.maxpool_params.out_rows, Extract(command.rs1, 55,48));
    config_store.SetUpdate(store_statevars.maxpool_params.out_cols, Extract(command.rs1, 63,56));
    
}

void DefineStoreInstruction(Ila& m, command_t command, gemmini_memory_t memory,
                            store_statevars_t& store_statevars) {

    // Parent store instruction defn, decode, and update
    auto store = m.NewInstr("store");
    store.SetDecode(command.funct == STORE_CMD);
    store.SetUpdate(store_statevars.child_valid, BoolConst(true));
    store.SetUpdate(store_statevars.cur_row, BvConst(0, 16));
    store.SetUpdate(store_statevars.cur_col, BvConst(0, 16));
    store.SetUpdate(store_statevars.cur_ch, BvConst(0, 16));
    store.SetUpdate(store_statevars.cur_wrow, BvConst(0, 16));
    store.SetUpdate(store_statevars.cur_wcol, BvConst(0, 16));
    store.SetUpdate(store_statevars.cur_max, BvConst( 1UL << (ACC_TYPE_WIDTH_BITS - 1), ACC_TYPE_WIDTH_BITS));
    

    // Define child ILA
    auto child = m.NewChild("store");
    child.SetValid(store_statevars.child_valid == 1);

    // Declare child ILA instruction
    for(int maxpool=0; maxpool <= 1; maxpool++){
        DefineStoreChildInstruction(child, command, memory, store_statevars, true, true, (bool) maxpool);
        DefineStoreChildInstruction(child, command, memory, store_statevars, true, false, (bool) maxpool);
        DefineStoreChildInstruction(child, command, memory, store_statevars, false, false, (bool) maxpool);
    }

}

void DefineStoreChildInstruction(Ila& child, 
                                command_t command, 
                                gemmini_memory_t memory, 
                                store_statevars_t store_statevars, 
                                bool from_accumulator, 
                                bool acctype,
                                bool maxpool){
    
    // Decode command fields
    auto soc_mem_base_address = command.rs1;
    auto is_acc_addr          = Extract(command.rs2, 31,31) == 1;
    auto read_acctype         = Extract(command.rs2, 29,29) == 1;
    auto src_base_address     = Extract(command.rs2, 28,0);
    auto num_cols             = Extract(command.rs2, 47, 32);
    auto num_rows             = Extract(command.rs2, 63, 48);
    
    

    // Declare instruction
    std::string instr_name = "store_row";
    if(from_accumulator){
        instr_name = instr_name + "_fromacc";

    } else{
        instr_name = instr_name + "_fromspad";
        acctype    = false;
    }
    if(acctype){
        instr_name = instr_name + "_acctype";
    } else{
        instr_name = instr_name + "_inputtype";
    }
    if(maxpool){
        instr_name = instr_name + "_maxpool";
    }
    ILA_INFO << instr_name;
    auto store_elem = child.NewInstr(instr_name);
    
    // Decode
    store_elem.SetDecode((is_acc_addr == BoolConst(from_accumulator)) 
                        & (BoolConst(acctype) == read_acctype) 
                        & ((store_statevars.maxpool_params.enable_and_stride != 0) == BoolConst(maxpool)));


    if(maxpool){
        maxpool_params_t maxp_params = store_statevars.maxpool_params;
        
        // Compute cur elmt indices
        auto channels = num_cols;
        auto orow     = store_statevars.cur_row.ZExt(32) * maxp_params.enable_and_stride.ZExt(32) + store_statevars.cur_wrow.ZExt(32) - maxp_params.upper_pad.ZExt(32);
        auto ocol     = store_statevars.cur_col.ZExt(32) * maxp_params.enable_and_stride.ZExt(32) + store_statevars.cur_wcol.ZExt(32) - maxp_params.left_pad.ZExt(32);
        auto row_addr = src_base_address.ZExt(32) + orow* maxp_params.out_cols.ZExt(32) + ocol;

        // Compute whether cur element is in bounds
        auto cond_out_of_bounds = orow < BvConst(0,orow.bit_width()) | ocol < BvConst(0,ocol.bit_width()) | orow >= maxp_params.out_rows.ZExt(32) | ocol >= maxp_params.out_cols.ZExt(32);

        // Compute cur element
        int  elem_size = from_accumulator ? ACC_TYPE_WIDTH_BITS : INPUT_TYPE_WIDTH_BITS;
        auto elem           = BvConst(0, elem_size);
        if(from_accumulator){
            auto acc_value = GetMemElmt(memory.accumulator, row_addr, store_statevars.cur_ch, elem_size);
            auto acc_value_scaled = ScaleAccType(acc_value, store_statevars.accScale);
                 elem             = Ite((!cond_out_of_bounds), acc_value_scaled, elem);
        }else{
            auto spad_value = GetMemElmt(memory.spad, row_addr, store_statevars.cur_ch, elem_size);
                 elem       = Ite((!cond_out_of_bounds), spad_value, elem);
        }

        //  Update max
        auto elem_extended = elem.SExt(ACC_TYPE_WIDTH_BITS);
        auto updated_max   = Ite(elem_extended > store_statevars.cur_max, elem_extended, store_statevars.cur_max);
        store_elem.SetUpdate(store_statevars.cur_max, updated_max);

        // Update state variables
        std::vector<ExprRef> iteration_vars = {store_statevars.cur_row, 
                                                store_statevars.cur_col, 
                                                store_statevars.cur_ch,
                                                store_statevars.cur_wrow,
                                                store_statevars.cur_wcol};
        std::vector<ExprRef> iteration_maxs = {maxp_params.in_rows, 
                                               maxp_params.in_cols, 
                                               channels,
                                               maxp_params.window_size,
                                               maxp_params.window_size};
        
        auto last_pixel = IterateLoopVars(store_elem, iteration_vars, iteration_maxs);
        store_elem.SetUpdate(store_statevars.child_valid, !(last_pixel));

    }else{

        // Compute soc_mem_address (aka destination address)
        auto store_elmt_size_bytes = Ite(read_acctype, BvConst(ACC_TYPE_WIDTH_BYTES,64), BvConst(INPUT_TYPE_WIDTH_BYTES, 64));
        auto soc_mem_offset        = (ZExt(store_statevars.cur_row, 64)*store_statevars.stride.ZExt(64))
                                + (store_elmt_size_bytes * ZExt(store_statevars.cur_col, 64));
        auto soc_mem_address = soc_mem_base_address + soc_mem_offset;


        // Compute spad address
        int  load_elmt_size    = from_accumulator ? ACC_TYPE_WIDTH_BITS : INPUT_TYPE_WIDTH_BITS;
        auto load_elmt_size_bv = BvConst(load_elmt_size, 16);
        auto array_dim_bv      = BvConst(ARRAY_DIM, 16);
        auto block             = store_statevars.cur_col / array_dim_bv;
        auto spad_col          = store_statevars.cur_col - block * array_dim_bv;                                                            // Equiv to cur_col % ARRAY_DIM
        auto spad_row          = src_base_address.ZExt(32) + ZExt(store_statevars.cur_row, 32) + ZExt(block, 32) * ZExt(array_dim_bv, 32);
    
        // Load data
        auto src_mem  = from_accumulator ?  memory.accumulator : memory.spad;
        auto src_elmt = GetMemElmt(src_mem, spad_row, spad_col, load_elmt_size);

        if(from_accumulator && !acctype){
            
            // Apply scale
            src_elmt = ScaleAccType(src_elmt, store_statevars.accScale);

            // Apply activation
            // Note only NONE and ReLU are implemented as activation functions
            src_elmt = ApplyActivation(src_elmt, store_statevars.activation, INPUT_TYPE_WIDTH_BITS);
        
        }

        // Store src_elmt
        auto soc_mem_next = StoreMulti(memory.soc_mem, src_elmt, soc_mem_address);

        // Update state variables
        std::vector<ExprRef> iteration_vars = {store_statevars.cur_row, 
                                                store_statevars.cur_col};
        std::vector<ExprRef> iteration_maxs = {num_rows, 
                                               num_cols};
        
        auto last_pixel = IterateLoopVars(store_elem, iteration_vars, iteration_maxs);
        store_elem.SetUpdate(store_statevars.child_valid, !(last_pixel));
        store_elem.SetUpdate(memory.soc_mem, soc_mem_next);
    }

}
    

}
}