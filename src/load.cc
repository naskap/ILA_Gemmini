#include <Gemmini/load.h>

namespace ilang {

namespace Gemmini {

void DefineLoad(Ila& m, command_t& command, gemmini_memory_t memory) {

  load_statevars_t* load_statevars = 
      (load_statevars_t*)malloc(sizeof(load_statevars_t) * 3);

  DefineLoadStateVars(m, load_statevars);
  DefineConfigLoadInstructions(m, command, load_statevars);
  DefineLoadInstructions(m, command, memory, load_statevars);
}

void DefineLoadStateVars(Ila& m, load_statevars_t* load_statevars) {

  for (int i = 0; i < 3; i++) {

    // Load configs
    load_statevars[i].src_stride = 
        m.NewBvState("load" + std::to_string(i) + "_src_stride", 64);
    load_statevars[i].dest_stride = 
        m.NewBvState("load" + std::to_string(i) + "_dest_stride", 16);
    load_statevars[i].scale = 
        m.NewBvState("load" + std::to_string(i) + "_scale", 32);
    load_statevars[i].read_inputType = 
        m.NewBoolState("load" + std::to_string(i) + "_read_inputType");
    load_statevars[i].pixels_per_row = 
        m.NewBvState("load" + std::to_string(i) + "_pixels_per_row", 8);

    // Load child helper variables
    load_statevars[i].child_valid = 
        m.NewBoolState("load" + std::to_string(i) + "_child_valid");
    load_statevars[i].cur_row = 
        m.NewBvState("load" + std::to_string(i) + "_cur_row", 16);
    load_statevars[i].cur_col = 
        m.NewBvState("load" + std::to_string(i) + "_cur_col", 16);
    load_statevars[i].cur_pixel = 
        m.NewBvState("load" + std::to_string(i) + "_cur_pixel", 8);
  }
}

void DefineConfigLoadInstructions(Ila& m, command_t command,
                                  load_statevars_t* load_statevars) {

  InstrRef* config_load = (InstrRef*)malloc(sizeof(InstrRef) * 3);
  for (int i = 0; i < 3; i++) {

    // Declare instruction
    config_load[i] = m.NewInstr("CONFIG_LOAD" + std::to_string(i));

    // Define decode
    config_load[i].SetDecode((command.funct == 0) &
                             (Extract(command.rs1, 1, 0) == 1) &
                             (Extract(command.rs1, 4, 3) == i));

    // Define update functions
    config_load[i].SetUpdate(load_statevars[i].read_inputType, Extract(command.rs1, 2, 2) == 1);
    config_load[i].SetUpdate(load_statevars[i].dest_stride, Extract(command.rs1, 31, 16));
    config_load[i].SetUpdate(load_statevars[i].scale, Extract(command.rs1, 63, 32));
    config_load[i].SetUpdate(load_statevars[i].src_stride, command.rs2);
    auto pixels_per_row_raw         = Extract(command.rs1, 15, 8);
    auto pixels_per_row_zero_is_one = Ite(pixels_per_row_raw == 0, BvConst(1,8), pixels_per_row_raw);
    config_load[i].SetUpdate(load_statevars[i].pixels_per_row, pixels_per_row_zero_is_one);
    
  }
}

void DefineLoadInstructions(Ila& m, command_t command, gemmini_memory_t memory,
                            load_statevars_t* load_statevars) {

  InstrRef *load     = (InstrRef*) malloc(sizeof(InstrRef) * 3);
  Ila      *children = (Ila *) malloc(sizeof(Ila)*3);
  for (int i = 0; i < 3; i++) {

    // Parent load instruction defn, decode, and update
    load[i] = m.NewInstr("load" + std::to_string(i));
    load[i].SetDecode(command.funct == load_command_functs[i]);
    load[i].SetUpdate(load_statevars[i].child_valid, BoolConst(1));
    load[i].SetUpdate(load_statevars[i].cur_row, BvConst(0, 16));
    load[i].SetUpdate(load_statevars[i].cur_col, BvConst(0, 16));
    load[i].SetUpdate(load_statevars[i].cur_pixel, BvConst(0, 16));
    
    // Define child ILA
    children[i] = m.NewChild("load" + std::to_string(i));
    children[i].SetValid(load_statevars[i].child_valid == 1);

    // Define child ILA instructions
    DefineLoadChildInstruction(children[i], i, command, memory, load_statevars[i], false, false, false);
    DefineLoadChildInstruction(children[i], i, command, memory, load_statevars[i], true, false, false);
    DefineLoadChildInstruction(children[i], i, command, memory, load_statevars[i], true, false, true);
    DefineLoadChildInstruction(children[i], i, command, memory, load_statevars[i], true, true, false);
    DefineLoadChildInstruction(children[i], i, command, memory, load_statevars[i], true, true, true);
    
  }
}

void DefineLoadChildInstruction(Ila& child, int load_num,
                                command_t command, 
                                gemmini_memory_t memory, 
                                load_statevars_t load_statevars, 
                                bool is_acc_addr, bool read_inputtype, 
                                bool accumulate){
    
    // Disable nonapplicable options if destination address is scratchpad
    if(!is_acc_addr){
        read_inputtype = true;
        accumulate     = false;
    }

    // Declare instruction
    auto load_elem = child.NewInstr(_BuildInstrName(load_num, is_acc_addr, read_inputtype, accumulate));

    // Decode instruction
    if(is_acc_addr){
        load_elem.SetDecode((Extract(command.rs2, 31, 31) == BvConst(is_acc_addr,1)) &
                        (load_statevars.read_inputType == BoolConst(read_inputtype)) &
                        (Extract(command.rs2, 30, 30) == BvConst(accumulate,1))); 
    }else{
        load_elem.SetDecode((Extract(command.rs2, 31, 31) == BvConst(is_acc_addr,1)));
    }
    
    // Extract command args
    auto num_rows = Extract(command.rs2,63,48);
    auto num_cols = Extract(command.rs2,47,32);
    auto soc_mem_base_address = command.rs1;
    auto spad_base_addr = Extract(command.rs2, 28, 0);

    // Load soc mem (src) elmt
    auto src_elmt = _GetSrcElmt(memory.soc_mem, soc_mem_base_address, load_statevars, read_inputtype);

    // Scale and cast src elmt
    if(read_inputtype){
        src_elmt = ScaleInputType(src_elmt, load_statevars.scale);
    }
    if(is_acc_addr && read_inputtype){
        src_elmt = SExt(src_elmt, ACC_TYPE_WIDTH_BITS);
    }

    // Store src_elmt into spad/accumulator
    _StoreSrcElmt(load_elem, memory, load_statevars,
                spad_base_addr, num_cols, src_elmt, 
                is_acc_addr, read_inputtype, accumulate);

    
    // Iterate loop variables
    std::vector<ExprRef> iteration_vars = {load_statevars.cur_row, load_statevars.cur_col, load_statevars.cur_pixel};
    std::vector<ExprRef> iteration_maxs = {num_rows, num_cols, load_statevars.pixels_per_row};
    auto last_pixel                     = IterateLoopVars(load_elem, iteration_vars, iteration_maxs);
    load_elem.SetUpdate(load_statevars.child_valid, !(last_pixel));
}

std::string _BuildInstrName(int load_num, bool is_acc_addr, bool read_inputtype, bool accumulate){
    std::string instr_name;
    if(is_acc_addr){
        std::string inputtype      = read_inputtype  ? "inputtype": "acctype";
        std::string load_operation = accumulate ? "accumulate" : "overwrite";
             instr_name            = "load" + std::to_string(load_num) + "_acc_" + inputtype + "_" + load_operation;
    }else{
        instr_name = "load" + std::to_string(load_num) + "_spad";
    }
    ILA_INFO << instr_name;
    return instr_name;
}

ExprRef _GetSrcElmt(ExprRef soc_mem, ExprRef soc_mem_base_address,load_statevars_t &load_statevars, bool read_inputtype){
    int  src_elmt_size    = read_inputtype ? INPUT_TYPE_WIDTH_BYTES : ACC_TYPE_WIDTH_BYTES;
    auto soc_mem_offset = ZExt(load_statevars.cur_row, 64)
                        * load_statevars.src_stride 
                        + BvConst(src_elmt_size, 64) * ZExt(load_statevars.cur_col, 64);
    auto soc_mem_addr = soc_mem_base_address + soc_mem_offset;
    auto src_elmt = Ite(soc_mem_addr == 0, BvConst(0, src_elmt_size * 8),
                        LoadMulti(soc_mem, soc_mem_addr, src_elmt_size));
    return src_elmt;
}

void _StoreSrcElmt(InstrRef &load_elem, 
                   gemmini_memory_t memory, 
                   load_statevars_t load_statevars,
                   ExprRef &spad_base_addr,
                   ExprRef &num_cols,
                   ExprRef &src_elmt,
                   bool is_acc_addr, bool read_inputtype, 
                   bool accumulate){

    // Compute dest row and col
    auto block            = load_statevars.cur_col / BvConst(ARRAY_DIM,16);
    auto spad_col         = load_statevars.cur_col - block * BvConst(ARRAY_DIM, 16); // represents base row/col of diagonal load                                                    // Equiv to cur_col % ARRAY_DIM
    auto spad_row         = spad_base_addr.ZExt(32) + ZExt(load_statevars.cur_row, 32) + ZExt(block, 32) * ZExt(load_statevars.dest_stride, 32);
    auto dest_row = spad_row - ZExt(load_statevars.cur_pixel, 32);
    auto dest_col = spad_col + load_statevars.cur_pixel.ZExt(16) * num_cols;
    
    // Load the row of data we want to modify
    ExprRef dest_row_data = (ExprRef) NULL;
    if(is_acc_addr){
        dest_row_data = Load(memory.accumulator, dest_row);
    }else{
        dest_row_data = Load(memory.spad, dest_row);
    }

    // Overwrite or accumulate our src_elmt into that row
    ExprRef row_to_store       = (ExprRef) NULL;
    auto    src_elmt_size_bits = read_inputtype ? INPUT_TYPE_WIDTH_BITS : ACC_TYPE_WIDTH_BITS;
    auto    dest_slice_idx     = BvConst(ARRAY_DIM * src_elmt_size_bits, 16) - dest_col * src_elmt_size_bits - 1;
    if(accumulate){
        row_to_store = AccSlice(dest_row_data, src_elmt, dest_slice_idx);
    }else{
        row_to_store = SetSlice(dest_row_data, src_elmt, dest_slice_idx);
    }
    
    // Store the row of data in spad or acc
    if(!is_acc_addr){
        auto spad_next = Store(memory.spad, dest_row, row_to_store);
        load_elem.SetUpdate(memory.spad, spad_next); 
    } else{
        auto acc_next = Store(memory.accumulator, dest_row, row_to_store);
        load_elem.SetUpdate(memory.accumulator, acc_next);
    }
}

} // Gemmini
} // ilang