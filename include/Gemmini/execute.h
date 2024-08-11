#include <Gemmini/gemmini_base.h>

namespace ilang {

namespace Gemmini {

enum dataflow_t {OS, WS};

struct execute_statevars_t {
    ExprRef dataflow = (ExprRef) NULL;
    ExprRef act = (ExprRef) NULL;
    ExprRef sys_shift = (ExprRef) NULL;
    ExprRef sys_acc_shift = (ExprRef) NULL;
    ExprRef a_transpose = (ExprRef) NULL;
    ExprRef b_transpose = (ExprRef) NULL;
    ExprRef c_stride = (ExprRef) NULL;
    ExprRef a_stride = (ExprRef) NULL;

    ExprRef preload_sp_addr = (ExprRef) NULL;
    ExprRef output_sp_addr = (ExprRef) NULL;
    ExprRef preload_cols = (ExprRef) NULL;
    ExprRef preload_rows = (ExprRef) NULL;
    ExprRef output_cols = (ExprRef) NULL;
    ExprRef output_rows = (ExprRef) NULL;

    ExprRef systolic_array = (ExprRef) NULL;
};

void DefineExecuteStatevars(Ila& m, execute_statevars_t &execute_statevars);
void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t &execute_statevars);
void DefineConfigExecute(Ila& m, command_t& command, execute_statevars_t &execute_statevars);
void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t &execute_statevars);
void DefineComputeMatmul(Ila& m, command_t& command, execute_statevars_t &execute_statevars, 
                        gemmini_memory_t memory);
void DefineComputeMatmulInstruction(Ila& m, command_t& command, execute_statevars_t &execute_statevars,
                                    gemmini_memory_t memory,
                                    bool preload, dataflow_t dataflow, bool acc_addr,
                                    bool accumulate_output);
std:: string _BuildInstrName(bool preload, dataflow_t dataflow, bool acc_addr, bool accumulate_output);
ExprRef _PreloadArray(ExprRef &spad, execute_statevars_t &execute_statevars);
ExprRef _InitializeWSResults(ExprRef &spad, execute_statevars_t &execute_statevars, ExprRef &bd_addr, ExprRef &bd_rows, ExprRef &bd_cols);

} // namespace Gemmini

} // namespace ilang