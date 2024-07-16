#include <Gemmini/gemmini_load.h>

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
    load_statevars[i].acc_type = 
        m.NewBoolState("load" + std::to_string(i) + "_acc_mvin_type");

    // Load child helper variables
    load_statevars[i].child_valid = 
        m.NewBoolState("load" + std::to_string(i) + "_child_valid");
    load_statevars[i].cur_row = 
        m.NewBvState("load" + std::to_string(i) + "_cur_row", 16);
    load_statevars[i].cur_col = 
        m.NewBvState("load" + std::to_string(i) + "_cur_col", 16);
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
    config_load[i].SetUpdate(load_statevars[i].acc_type, Extract(command.rs1, 2, 2) == 1);
    config_load[i].SetUpdate(load_statevars[i].dest_stride, Extract(command.rs1, 31, 16));
    config_load[i].SetUpdate(load_statevars[i].scale, Extract(command.rs1, 63, 32));
    config_load[i].SetUpdate(load_statevars[i].src_stride, command.rs2);
    
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
                                bool is_acc_addr, bool acctype_inputs, 
                                bool accumulate){
    
    // Disable nonapplicable options if destination address is scratchpad
    if(!is_acc_addr){
        acctype_inputs = false;
        accumulate     = false;
    }

    // Compute instruction name
    std:: string instr_name;
    if(is_acc_addr){
        std::string inputtype      = acctype_inputs  ? "acctype" : "inputtype";
        std::string load_operation = accumulate ? "accumulate" : "overwrite";
             instr_name            = "load" + std::to_string(load_num) + "_acc_" + inputtype + "_" + load_operation;
    }else{
        instr_name = "load" + std::to_string(load_num) + "_spad";
    }
    ILA_INFO << instr_name;

    // Declare instruction
    auto load_row = child.NewInstr(instr_name);

    // Decode instruction
    if(is_acc_addr){
        load_row.SetDecode((Extract(command.rs2, 31, 31) == BvConst(is_acc_addr,1)) &
                        (load_statevars.acc_type == BoolConst(acctype_inputs)) &
                        (Extract(command.rs2, 30, 30) == BvConst(accumulate,1))); 
    }else{
        load_row.SetDecode((Extract(command.rs2, 31, 31) == BvConst(is_acc_addr,1)));
    }

    // Compute src element size
    int  src_elmt_size    = acctype_inputs ? ACC_TYPE_WIDTH_BYTES : INPUT_TYPE_WIDTH_BYTES;
    auto src_elmt_size_bv = BvConst(src_elmt_size, 64);


    // Compute soc mem address (aka src address) for current row
    auto soc_mem_offset = CastUnsigned(load_statevars.cur_row, 64)
                        * load_statevars.src_stride 
                        + src_elmt_size_bv * CastUnsigned(load_statevars.cur_col, 64);
    auto soc_mem_addr = command.rs1 + soc_mem_offset;
    
    // Compute destination address
    auto submatrix   = load_statevars.cur_col / BvConst(ARRAY_DIM,16);
    auto dest_offset = CastUnsigned(load_statevars.cur_row, 32) +
                         (CastUnsigned(submatrix, 32) *  CastUnsigned(load_statevars.dest_stride, 32));
    auto dest_addr = CastUnsigned(Extract(command.rs2, 28, 0), 32) + dest_offset;

    

    // Calculate values to help load data from soc mem
    bool cast_to_acctype   = is_acc_addr && !acctype_inputs;
    int  zero_pad_per_elmt = ACC_TYPE_WIDTH_BITS - INPUT_TYPE_WIDTH_BITS;
    int  num_bytes_per_row_src = ARRAY_DIM * src_elmt_size;
    
    // Note: Undefined what happens when num_src_cols is not a multiple of DIM 
    // Right now we're just transferring the whole submatrix


    // Load the first data entry 
    auto src_row               = Load(memory.soc_mem, soc_mem_addr);
    if(cast_to_acctype){
        src_row = Concat(BvConst(0,zero_pad_per_elmt), src_row);
    }

    // Load the rest of the data entries
    for(int i = 1; i < num_bytes_per_row_src; i++){
        src_row = Concat(src_row, Load(memory.soc_mem, soc_mem_addr+i));
        
        if(cast_to_acctype){
            src_row = Concat(BvConst(0,zero_pad_per_elmt), src_row);
        }
    }

    // Store the row of data in spad or acc
    if(!is_acc_addr){
        auto spad_next = Store(memory.spad, dest_addr, src_row);
        load_row.SetUpdate(memory.spad, spad_next); 
    } else if(accumulate){
        auto row_to_accumulate_onto = Load(memory.accumulator, dest_addr);
        auto to_store               = row_to_accumulate_onto + src_row;
        auto acc_next               = Store(memory.accumulator, dest_addr, src_row);
        load_row.SetUpdate(memory.accumulator, acc_next);
    } else{
        auto acc_next = Store(memory.accumulator, dest_addr, src_row);
        load_row.SetUpdate(memory.accumulator, acc_next);
    }
    
    // Update state variables to transfer entire data array
    auto num_rows      = Extract(command.rs2,63,48);
    auto num_cols      = Extract(command.rs2,47,32);
    auto new_submatrix = load_statevars.cur_row == (num_rows - BvConst(1,16));
    load_row.SetUpdate(load_statevars.cur_row, 
            Ite(new_submatrix, 
                BvConst(0, 16), load_statevars.cur_row + 1));
    load_row.SetUpdate(load_statevars.cur_col, 
            Ite(new_submatrix, 
                load_statevars.cur_col + BvConst(ARRAY_DIM,16), 
                load_statevars.cur_col));
    auto last_col = Uge(load_statevars.cur_col, num_cols - BvConst(ARRAY_DIM,16));
    load_row.SetUpdate(load_statevars.child_valid, !(new_submatrix & last_col));
}

} // Gemmini
} // ilang