#ifndef GEMMINI_LOAD_H
#define GEMMINI_LOAD_H
#include <Gemmini/Gemmini.h> 
#include <ilang/ilang++.h>

namespace ilang {

namespace Gemmini {

struct maxpool_params_t {
  ExprRef enable_and_stride = (ExprRef)NULL;
  ExprRef window_size       = (ExprRef)NULL;
  ExprRef upper_pad         = (ExprRef)NULL;
  ExprRef left_pad          = (ExprRef)NULL;
  ExprRef out_dim           = (ExprRef)NULL;
  ExprRef out_rows          = (ExprRef)NULL;
  ExprRef out_cols          = (ExprRef)NULL;
  ExprRef in_rows           = (ExprRef)NULL;
  ExprRef in_cols           = (ExprRef)NULL;
};

struct store_statevars_t {
  ExprRef accScale   = (ExprRef)NULL;
  ExprRef stride     = (ExprRef)NULL;
  ExprRef activation = (ExprRef)NULL;
  maxpool_params_t maxpool_params;

  ExprRef child_valid = (ExprRef) NULL;
  ExprRef cur_row     = (ExprRef) NULL;
  ExprRef cur_col     = (ExprRef) NULL;
  ExprRef cur_ch      = (ExprRef) NULL;
  ExprRef cur_wrow    = (ExprRef) NULL;
  ExprRef cur_wcol    = (ExprRef) NULL;
  ExprRef cur_max     = (ExprRef) NULL;
};

enum Activation {NONE, ReLU};

void DefineStoreStateVars(Ila& m, store_statevars_t& store_statevars);
void DefineConfigStoreInstruction(Ila& m, command_t command,
                                  store_statevars_t& store_statevars);
void DefineStoreInstruction(Ila& m, command_t command, gemmini_memory_t memory,
                            store_statevars_t& store_statevars);
void DefineStoreChildInstruction(Ila& child, 
                                command_t command, 
                                gemmini_memory_t memory, 
                                store_statevars_t store_statevars, 
                                bool from_accumulator,
                                bool cast_to_intype,
                                bool maxpool);



} // namespace Gemmini
} // namespace Ilang
#endif