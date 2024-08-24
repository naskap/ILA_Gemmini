#include <Gemmini/gemmini_base.h>

namespace ilang {

namespace Gemmini {
void DefineLoopWSStatevars(Ila &m, loop_ws_statevars_t &loop_svs);
void DefineLoopWSConfigBounds(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs);
void DefineLoopWSConfigAddrsAB(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs);
void DefineLoopWSConfigAddrsDC(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs);
void DefineLoopWSConfigStridesAB(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs);
void DefineLoopWSConfigStridesDC(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs);
void DefineLoopWSInstruction(Ila &m, command_t &command, gemmini_statevars_t &svs); 
void DefineLoadD(Ila &child, gemmini_statevars_t &svs);
void DefineComputeLoopStart(Ila &child, gemmini_statevars_t &svs);
void DefineMvinA(Ila &child, gemmini_statevars_t &svs);
void DefineMvinB(Ila &child, gemmini_statevars_t &svs);
void DefineConfigPreload(Ila &child, gemmini_statevars_t &svs);
void DefineCompute(Ila &child, gemmini_statevars_t &svs);
void DefineMvoutC(Ila &child, gemmini_statevars_t &svs);
void DefineIterate(Ila &child, gemmini_statevars_t &svs);


enum loop_ws_child_states {
    LOOP_WS_INACTIVE, 
    LOAD_D, 
    COMPUTE_LOOP_START,
    MVIN_A, 
    MVIN_B, 
    CONFIG_PRELOAD, 
    COMPUTE, 
    MVOUT_C, 
    ITERATE
};

}

}





