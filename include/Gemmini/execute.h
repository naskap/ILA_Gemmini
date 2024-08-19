#include <Gemmini/gemmini_base.h>

namespace ilang {

namespace Gemmini {

enum dataflow_t {OS, WS};

enum compute_child_states {INACTIVE, PRELOAD, INITIALIZE_WS_RESULTS, WS_COMPUTE, OS_COMPUTE, OUTPUT_RESULTS};


void DefineExecuteStatevars(Ila& m, execute_statevars_t &execute_statevars);
void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t &execute_statevars);
void DefineConfigExecute(Ila& m, command_t& command, execute_statevars_t &execute_statevars);
void DefineMatmulPreload(Ila& m, command_t& command, execute_statevars_t &execute_statevars);
void DefineComputeMatmul(Ila& m, command_t& command, execute_statevars_t &execute_statevars, 
                        gemmini_memory_t memory);


void DefineComputeMatmulInstruction(Ila& m, command_t& command, execute_statevars_t &execute_statevars,
                                    gemmini_memory_t memory, bool preload, dataflow_t dataflow);
std:: string _BuildComputeMatmulInstrName(bool preload, dataflow_t dataflow);

void DefinePreload(Ila &child, ExprRef &spad, execute_statevars_t &execute_statevars);
void DefineInitializeWSResults(Ila &child, ExprRef &spad, execute_statevars_t &execute_statevars, tile_compute_args_t &bd_args);
void DefineMatmulWS(Ila &child, ExprRef &spad, execute_statevars_t &execute_statevars, compute_args_t &compute_args);
void DefineMatmulOS(Ila &child, ExprRef &spad, execute_statevars_t &execute_statevars, compute_args_t &compute_args);

ExprRef _GetTileAElmt(ExprRef &spad, execute_statevars_t &execute_statevars, tile_compute_args_t &a_args, ExprRef &i_bv, ExprRef &k_bv);

void DefineStoreOutputChild(Ila &m, command_t &command, execute_statevars_t &execute_statevars, gemmini_memory_t &memory);
void DefineStoreOutputInstruction(Ila &store_output, execute_statevars_t &execute_statevars, 
                        gemmini_memory_t &memory, dataflow_t const &dataflow, bool acc_address, bool accumulate_output);
std::string _BuildStoreOutputInstrName(dataflow_t const &dataflow, bool acc_address, bool accumulate_output);
ExprRef _RoundingRightShift(ExprRef const &x, ExprRef const &shift);

} // namespace Gemmini

} // namespace ilang