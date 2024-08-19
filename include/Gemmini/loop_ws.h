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
void _CallMvin(InstrRef &caller, load_statevars_t &load_svs, ExprRef &dram_addr, ExprRef &sp_addr, ExprRef &rows, ExprRef &cols);
void DefineGetSPAddrs(Ila &child, gemmini_statevars_t &svs);
void DefineMvinA(Ila &child, gemmini_statevars_t &svs);
void DefineMvinB(Ila &child, gemmini_statevars_t &svs);
void DefineCompute(Ila &child, gemmini_statevars_t &svs);
void DefineMvoutC(Ila &child, gemmini_statevars_t &svs);
void DefineIterate(Ila &child, gemmini_statevars_t &svs);


enum loop_ws_child_states {INACTIVE, LOAD_D, GET_SP_ADDRS, MVIN_A, MVIN_B, COMPUTE, MVOUT_C, ITERATE};

}

}





