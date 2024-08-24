#include <Gemmini/gemmini_base.h>

namespace ilang {

namespace Gemmini {

enum loop_conv_ws_child_states {
    LOOP_CONV_WS_INACTIVE, 
    CONFIG_MVIN_BIAS,
    MVIN_BIAS, 
    CONFIG_MVIN_INPUT,
    MVIN_INPUT, 
    CONFIG_MVIN_WEIGHTS,
    MVIN_WEIGHTS, 
    CONFIG_COMPUTE, 
    ITERATE_OCOL, 
    CONFIG_PRELOAD_CONV, 
    COMPUTE_CONV,
    MVOUT_RESULTS_NOPOOL,
    CONFIG_MVOUT_RESULTS_POOL,
    MVOUT_RESULTS_POOL,
    CONFIG_RESET_MVOUT_RESULTS
};


void DefineLoopConvWSStatevars(Ila &m, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig1(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig2(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig3(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig4(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig5(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig6(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSInstruction(Ila &m, command_t &command, gemmini_statevars_t &svs); 
ExprRef _UNDILATED(ExprRef &x, ExprRef &input_dilated);
void DefineConfigMvinBias(Ila &child, gemmini_statevars_t &svs);
void DefineMvinBias(Ila &child, gemmini_statevars_t &svs);
void DefineConfigMvinInput(Ila &child, gemmini_statevars_t &svs);
void DefineMvinInput(Ila &child, gemmini_statevars_t &svs);
void DefineConfigMvinWeights(Ila &child, gemmini_statevars_t &svs);
void DefineMvinWeights(Ila &child, gemmini_statevars_t &svs);
void DefineConfigCompute(Ila &child, gemmini_statevars_t &svs);
void DefineIterateOcol(Ila &child, gemmini_statevars_t &svs);
void DefineConfigPreloadCol(Ila &child, gemmini_statevars_t &svs);
void DefineComputeConv(Ila &child, gemmini_statevars_t &svs);
void DefineMvoutResultsNoPool(Ila &child, gemmini_statevars_t &svs);
void DefineConfigMvoutResultsPool(Ila &child, gemmini_statevars_t &svs);
void DefineMvoutResultsPool(Ila &child, gemmini_statevars_t &svs);
void DefineConfigResetMvoutResults(Ila &child, gemmini_statevars_t &svs);

}

}