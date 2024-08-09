#include <Gemmini/gemmini_base.h>

namespace ilang {

namespace Gemmini {

enum dataflow_t {OS, WS};

struct execute_statevars_t {
    ExprRef dataflow; 
    ExprRef act;
    ExprRef sys_shift;
    ExprRef sys_acc_shift;
    ExprRef a_transpose;
    ExprRef b_transpose;
    ExprRef c_stride;
    ExprRef a_stride;

    ExprRef preload_sp_addr;
    ExprRef output_sp_addr;
    ExprRef preload_cols;
    ExprRef preload_rows;
    ExprRef output_cols;
    ExprRef output_rows;
};

void DefineExecuteStatevars(Ila& m, execute_statevars_t *execute_statevars);
void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t *execute_statevars);
void DefineConfigExecute(Ila& m, command_t& command, execute_statevars_t *execute_statevars);
void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t *execute_statevars);

} // namespace Gemmini

} // namespace ilang