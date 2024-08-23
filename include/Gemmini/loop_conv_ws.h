#include <Gemmini/gemmini_base.h>

namespace ilang {

namespace Gemmini {

void DefineLoopConvWSStatevars(Ila &m, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig1(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig2(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig3(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig4(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig5(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSConfig6(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs);
void DefineLoopConvWSInstruction(Ila &m, command_t &command, gemmini_statevars_t &svs); 

}

}