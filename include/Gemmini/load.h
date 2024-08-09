#ifndef GEMMINI_LOAD_H
#define GEMMINI_LOAD_H
#include <Gemmini/gemmini_base.h> 

namespace ilang {

namespace Gemmini {

struct load_statevars_t {
  // Config state vars
  ExprRef src_stride     = (ExprRef)NULL;  // Main memory stride in bytes
  ExprRef dest_stride    = (ExprRef)NULL;  // spad stride in bytes
  ExprRef scale          = (ExprRef)NULL;  // mvin scale (ignored if gemmini not configured with this ability)
  ExprRef read_inputType       = (ExprRef)NULL;
  ExprRef pixels_per_row = (ExprRef)NULL;
  
  // Child ILA helpers
  ExprRef child_valid = (ExprRef)NULL;
  ExprRef cur_row     = (ExprRef)NULL;
  ExprRef cur_col     = (ExprRef)NULL;
  ExprRef cur_pixel   = (ExprRef)NULL;
};

const uint8_t load_command_functs[] = {LOAD_CMD, LOAD2_CMD, LOAD3_CMD};

struct load_command_t {};

// void DefineLoad(Ila& m, command_t& command, gemmini_memory_t memory);
void DefineLoadStateVars(Ila& m, load_statevars_t* load_statevars);
void DefineConfigLoadInstructions(Ila& m, command_t command,
                                  load_statevars_t* load_statevars);
void DefineLoadInstructions(Ila& m, command_t command, gemmini_memory_t memory, load_statevars_t* load_statevars);
void DefineLoadChildInstruction(Ila& child, int load_num, command_t command, gemmini_memory_t memory, load_statevars_t load_statevars, bool is_acc_addr, bool acctype_inputs, bool accumulate);
ExprRef _GetSrcElmt(ExprRef soc_mem, ExprRef soc_mem_base_address,load_statevars_t &load_statevars, bool read_inputtype);
void _StoreSrcElmt(InstrRef &load_elem, gemmini_memory_t memory, load_statevars_t load_statevars,ExprRef &spad_base_addr,ExprRef &num_cols,ExprRef &src_elmt,bool is_acc_addr, bool read_inputtype, bool accumulate);

std::string _BuildInstrName(int load_num, bool is_acc_addr, bool read_inputtype, bool accumulate);

} // namespace Gemmini
} // namespace Ilang
#endif