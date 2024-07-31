
#include <Gemmini/gemmini_store.h>

namespace ilang {

namespace Gemmini {


void DefineStore(Ila& m, command_t& command, gemmini_memory_t memory) {

  store_statevars_t store_statevars;
  DefineStoreStateVars(m, store_statevars);
  DefineConfigStoreInstruction(m, command, store_statevars);
  DefineStoreInstruction(m, command, memory, store_statevars);
}

void DefineStoreStateVars(Ila& m, store_statevars_t& store_statevars) {

    // Store configs
    store_statevars.stride                           = m.NewBvState("store_stride", 64);
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
    config_store.SetUpdate(store_statevars.stride, command.rs2);
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
        
        
        auto channels = num_cols;
        auto orow     = store_statevars.cur_row.ZExt(32) * maxp_params.enable_and_stride.ZExt(32) + store_statevars.cur_wrow.ZExt(32) - maxp_params.upper_pad.ZExt(32);
        auto ocol     = store_statevars.cur_col.ZExt(32) * maxp_params.enable_and_stride.ZExt(32) + store_statevars.cur_wcol.ZExt(32) - maxp_params.left_pad.ZExt(32);
        auto row_addr = src_base_address.ZExt(32) + orow* maxp_params.out_cols.ZExt(32) + ocol;


        int     elem_size      = from_accumulator && acctype ? ACC_TYPE_WIDTH_BITS : INPUT_TYPE_WIDTH_BITS;
        int     row_size       = elem_size * ARRAY_DIM;
        ExprRef elem           = (ExprRef) BvConst(0, elem_size);
        ExprRef elem_offset_hi = BvConst(row_size, 16) - store_statevars.cur_ch * BvConst(elem_size, 16);
        
        // Check if element is a pad
        ExprRef cond_out_of_bounds = orow < BvConst(0,orow.bit_width()) | ocol < BvConst(0,ocol.bit_width()) | orow >= maxp_params.out_rows.ZExt(32) | ocol >= maxp_params.out_cols.ZExt(32);

        // Handle accumulator case
        if(from_accumulator){
            auto acc_row   = Load(memory.accumulator,row_addr);
            auto acc_value = GetSlice(acc_row, elem_offset_hi, elem_size);
            // < Insert scaling and activation code> 
            elem = Ite((!cond_out_of_bounds), acc_value, elem);
        }else{
            // Handle spad case
            auto spad_row   = Load(memory.spad, row_addr);
            auto spad_value = GetSlice(spad_row, elem_offset_hi, elem_size);
                elem       = Ite((!cond_out_of_bounds), spad_value, elem);
        }

        //  Update max
        auto elem_extended = elem.SExt(ACC_TYPE_WIDTH_BITS);
        auto updated_max = Ite(elem_extended > store_statevars.cur_max, elem_extended, store_statevars.cur_max);
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
        auto element_size   = Ite(read_acctype, BvConst(ACC_TYPE_WIDTH_BYTES,64), BvConst(INPUT_TYPE_WIDTH_BYTES, 64));
        auto soc_mem_offset = (ZExt(store_statevars.cur_row, 64)*store_statevars.stride)
                                + (element_size * ZExt(store_statevars.cur_col, 64));
        auto soc_mem_address = soc_mem_base_address + soc_mem_offset;

        // Compute src_address
        auto submatrix   = store_statevars.cur_col / BvConst(ARRAY_DIM,16);
        auto src_offset  = ZExt(store_statevars.cur_row, 32) + ZExt(submatrix, 32) * ZExt(num_rows, 32);
        auto src_address = ZExt(src_base_address, 32) + src_offset;

        // Load a row of data
        auto src_mem = from_accumulator ?  memory.accumulator : memory.spad;
        auto src_row = Load(src_mem, src_address);

        // Cast accType to inType if necessary
        auto to_store = (ExprRef) NULL;
        if(from_accumulator && !acctype){
            to_store = Extract(src_row, INPUT_TYPE_WIDTH_BITS - 1, 0);
            for(int i = 1; i < ARRAY_DIM; i++){
                to_store = Concat(Extract(src_row, INPUT_TYPE_WIDTH_BITS*(i+1) - 1, INPUT_TYPE_WIDTH_BITS*i), to_store);
            }
        }else{
            to_store = src_row;
        }
        
        // Store row
        auto soc_mem_next = memory.soc_mem;
        for(int i = 0; i < (to_store.bit_width() / 8); i++){
            soc_mem_next = Store(soc_mem_next, soc_mem_address + i, 
                                Extract(to_store, 8*(i+1) -1, 8*i));
        }

        // Update state variables
        std::vector<ExprRef> iteration_vars = {store_statevars.cur_row, 
                                                store_statevars.cur_col};
        std::vector<ExprRef> iteration_maxs = {num_rows, 
                                               num_cols};
        
        auto last_pixel = IterateLoopVars(store_elem, iteration_vars, iteration_maxs);
        store_elem.SetUpdate(store_statevars.child_valid, !(last_pixel));
    }

}
    

}
}