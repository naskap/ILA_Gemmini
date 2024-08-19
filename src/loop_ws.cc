#include <Gemmini/loop_ws.h>


namespace ilang {

namespace Gemmini {

void DefineLoopWS(Ila& m, command_t& command, gemmini_memory_t memory, gemmini_statevars_t &svs){

    DefineLoopWSStatevars(m, svs.loop_ws);
    DefineLoopWSConfigBounds(m, command, svs.loop_ws);
    DefineLoopWSConfigAddrsAB(m, command, svs.loop_ws);
    DefineLoopWSConfigAddrsDC(m, command, svs.loop_ws);
    DefineLoopWSConfigStridesAB(m, command, svs.loop_ws);
    DefineLoopWSConfigStridesDC(m, command, svs.loop_ws);
    DefineLoopWSInstruction(m, command, svs); 

}

void DefineLoopWSStatevars(Ila &m, loop_ws_statevars_t &loop_svs){

    loop_svs.I     = m.NewBvState("loop_ws_I", 16);
    loop_svs.J     = m.NewBvState("loop_ws_J", 16);
    loop_svs.K     = m.NewBvState("loop_ws_K", 16);
    loop_svs.pad_I = m.NewBvState("loop_ws_pad_I", 16);
    loop_svs.pad_J = m.NewBvState("loop_ws_pad_J", 16);
    loop_svs.pad_K = m.NewBvState("loop_ws_pad_K", 16);

    loop_svs.A = m.NewBvState("loop_ws_A", 64);
    loop_svs.B = m.NewBvState("loop_ws_B", 64);
    loop_svs.D = m.NewBvState("loop_ws_D", 64);
    loop_svs.C = m.NewBvState("loop_ws_C", 64);
    
    loop_svs.A_stride = m.NewBvState("loop_ws_A_stride", 64);
    loop_svs.B_stride = m.NewBvState("loop_ws_B_stride", 64);
    loop_svs.D_stride = m.NewBvState("loop_ws_D_stride", 64);
    loop_svs.C_stride = m.NewBvState("loop_ws_C_stride", 64);

    loop_svs.ex_accumulate = m.NewBoolState("loop_ws_ex_accumulate");
    loop_svs.full_C        = m.NewBoolState("loop_ws_full_C");
    loop_svs.low_D         = m.NewBoolState("loop_ws_low_D");
    loop_svs.act           = m.NewBvState("loop_ws_act", 3);
    loop_svs.a_transpose   = m.NewBoolState("loop_ws_a_transpose");
    loop_svs.b_transpose   = m.NewBoolState("loop_ws_b_transpose");

    loop_svs.i               = m.NewBvState("loop_ws_i", 16);
    loop_svs.j               = m.NewBvState("loop_ws_j", 16);
    loop_svs.k               = m.NewBvState("loop_ws_k", 16);
    loop_svs.A_sp_addr_start = m.NewBvState("loop_ws_A_sp_addr_start", 32);
    loop_svs.B_sp_addr_start = m.NewBvState("loop_ws_B_sp_addr_start", 32);
    loop_svs.D_sp_addr_start = m.NewBvState("loop_ws_D_sp_addr_start", 32);
    loop_svs.C_sp_addr_start = m.NewBvState("loop_ws_C_sp_addr_start", 32);
    loop_svs.A_sp_addr       = m.NewBvState("loop_ws_A_sp_addr", 32);
    loop_svs.B_sp_addr       = m.NewBvState("loop_ws_B_sp_addr", 32);
    loop_svs.D_sp_addr       = m.NewBvState("loop_ws_D_sp_addr", 32);
    loop_svs.C_sp_addr       = m.NewBvState("loop_ws_C_sp_addr", 32);
    loop_svs.child_state     = m.NewBvState("loop_ws_child_state", 16);

}

void DefineLoopWSConfigBounds(Ila &m, command_t &command, loop_ws_statevars_t &loop_svs){

    auto config_bounds = m.NewInstr("loop_ws_config_bounds");

    config_bounds.SetDecode(command.funct == BvConst(LOOP_WS_CONFIG_BOUNDS, INSTR_FUNCT_WIDTH));

    config_bounds.SetUpdate(loop_svs.I, Extract(command.rs2, 15, 0));
    config_bounds.SetUpdate(loop_svs.J, Extract(command.rs2, 31, 16));
    config_bounds.SetUpdate(loop_svs.K, Extract(command.rs2, 47, 32));
    config_bounds.SetUpdate(loop_svs.pad_I, Extract(command.rs1, 15, 0));
    config_bounds.SetUpdate(loop_svs.pad_J, Extract(command.rs1, 31, 16));
    config_bounds.SetUpdate(loop_svs.pad_K, Extract(command.rs1, 47, 32));
    
}
void DefineLoopWSConfigAddrsAB(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs){
    
    auto config_addrs_ab = m.NewInstr("loop_ws_config_addrs_ab");

    config_addrs_ab.SetDecode(command.funct == BvConst(LOOP_WS_CONFIG_ADDRS_AB, INSTR_FUNCT_WIDTH));

    config_addrs_ab.SetUpdate(loop_svs.A, command.rs1);
    config_addrs_ab.SetUpdate(loop_svs.B, command.rs2);
    

}
void DefineLoopWSConfigAddrsDC(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs){

    auto config_addrs_dc = m.NewInstr("loop_ws_config_addrs_dc");

    config_addrs_dc.SetDecode(command.funct == BvConst(LOOP_WS_CONFIG_ADDRS_DC, INSTR_FUNCT_WIDTH));

    config_addrs_dc.SetUpdate(loop_svs.D, command.rs1);
    config_addrs_dc.SetUpdate(loop_svs.C, command.rs2);

}
void DefineLoopWSConfigStridesAB(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs){

    auto config_strides_ab = m.NewInstr("loop_ws_config_strides_ab");

    config_strides_ab.SetDecode(command.funct == BvConst(LOOP_WS_CONFIG_STRIDES_AB, INSTR_FUNCT_WIDTH));

    config_strides_ab.SetUpdate(loop_svs.A_stride, command.rs1);
    config_strides_ab.SetUpdate(loop_svs.B_stride, command.rs2);

}
void DefineLoopWSConfigStridesDC(Ila &m,command_t &command, loop_ws_statevars_t &loop_svs){

    auto config_strides_dc = m.NewInstr("loop_ws_config_strides_dc");

    config_strides_dc.SetDecode(command.funct == BvConst(LOOP_WS_CONFIG_STRIDES_DC, INSTR_FUNCT_WIDTH));

    config_strides_dc.SetUpdate(loop_svs.D_stride, command.rs1);
    config_strides_dc.SetUpdate(loop_svs.C_stride, command.rs2);

}

void DefineLoopWSInstruction(Ila &m, command_t &command, gemmini_statevars_t &svs){

    auto loop_ws = m.NewInstr("loop_ws");

    loop_ws.SetDecode(command.funct == BvConst(LOOP_WS, INSTR_FUNCT_WIDTH));

    // Initialize statevars for computation
    loop_ws.SetUpdate(svs.loop_ws.ex_accumulate, SelectBit(command.rs1,0));
    loop_ws.SetUpdate(svs.loop_ws.full_C, SelectBit(command.rs1,1));
    loop_ws.SetUpdate(svs.loop_ws.low_D, SelectBit(command.rs1,2));
    loop_ws.SetUpdate(svs.loop_ws.act, Extract(command.rs1,10,8));
    loop_ws.SetUpdate(svs.loop_ws.a_transpose, SelectBit(command.rs2,0));
    loop_ws.SetUpdate(svs.loop_ws.b_transpose, SelectBit(command.rs2,1));
    loop_ws.SetUpdate(svs.loop_ws.i, BvConst(0,16));
    loop_ws.SetUpdate(svs.loop_ws.j, BvConst(0,16));
    loop_ws.SetUpdate(svs.loop_ws.k, BvConst(0,16));
    loop_ws.SetUpdate(svs.loop_ws.A_sp_addr_start, BvConst(0,32));
    loop_ws.SetUpdate(svs.loop_ws.B_sp_addr_start, BvConst((SPAD_BANKS * SPAD_ROWS / 2), 32) - svs.loop_ws.K.ZExt(32) * svs.loop_ws.J.ZExt(32) * ARRAY_DIM);
    loop_ws.SetUpdate(svs.loop_ws.D_sp_addr_start, BvConst(1 << (SPAD_ADDRESS_WIDTH - 1),32));
    loop_ws.SetUpdate(svs.loop_ws.C_sp_addr_start, BvConst((3 << (SPAD_ADDRESS_WIDTH - 2)), 32) | (Extract(command.rs1, 1, 1).ZExt(32) << (SPAD_ADDRESS_WIDTH - 3)));
    loop_ws.SetUpdate(svs.loop_ws.child_state, Ite(svs.loop_ws.D == 0,
                                                    BvConst(loop_ws_child_states::GET_SP_ADDRS, svs.loop_ws.child_state.bit_width()),
                                                    BvConst(loop_ws_child_states::LOAD_D, svs.loop_ws.child_state.bit_width())));


    // Define child ILA
    auto child = m.NewChild("loop_ws_child");
    child.SetValid((svs.loop_ws.child_state != loop_ws_child_states::INACTIVE) & (svs.load[0].child_valid == false) & (svs.store.child_valid == false) & (svs.exec.child_state == 0));
    DefineLoadD(child, svs);
    DefineGetSPAddrs(child, svs);
    DefineMvinA(child, svs);
    // DefineMvinB(child, svs);
    // DefineCompute(child, svs);
    // DefineMvoutC(child, svs);
    // DefineIterate(child, svs);

}

void DefineLoadD(Ila &child, gemmini_statevars_t &svs){

    auto load_d = child.NewInstr("loop_ws_load_d");
    
    load_d.SetDecode(svs.loop_ws.child_state == loop_ws_child_states::LOAD_D);

    auto sizeof_D  = Ite(svs.loop_ws.low_D, BvConst(INPUT_TYPE_WIDTH_BITS, 32), BvConst(ACC_TYPE_WIDTH_BITS, 32));
    auto dram_addr = svs.loop_ws.D + (svs.loop_ws.i.ZExt(64) * svs.loop_ws.D_stride + svs.loop_ws.j.ZExt(64)) * ARRAY_DIM * sizeof_D.ZExt(64);

    auto sp_addr = svs.loop_ws.D_sp_addr_start + (svs.loop_ws.i.ZExt(32) * svs.loop_ws.J.ZExt(32) + svs.loop_ws.j.ZExt(32)) * ARRAY_DIM;

    auto cols = BvConst(ARRAY_DIM, 16) - Ite(svs.loop_ws.j == (svs.loop_ws.J - 1), svs.loop_ws.pad_J, BvConst(0, 16));
    auto rows = BvConst(ARRAY_DIM, 16) - Ite(svs.loop_ws.i == (svs.loop_ws.I - 1), svs.loop_ws.pad_I, BvConst(0, 16));

    _CallMvin(load_d, svs.load[2], dram_addr, sp_addr, rows, cols);

    // Iterate loop vars
    std::vector<ExprRef> iteration_vars = {svs.loop_ws.i, svs.loop_ws.j};
    std::vector<ExprRef> iteration_maxs = {svs.loop_ws.I, svs.loop_ws.J};
    auto last_pixel                     = IterateLoopVars(load_d, iteration_vars, iteration_maxs);
    auto next_child_state               = Ite(last_pixel, BvConst(loop_ws_child_states::COMPUTE, svs.loop_ws.child_state.bit_width()),
                                                              BvConst(loop_ws_child_states::LOAD_D, svs.loop_ws.child_state.bit_width()));
    load_d.SetUpdate(svs.loop_ws.child_state, next_child_state);

}


void _CallMvin(InstrRef &caller, load_statevars_t &load_svs, ExprRef &dram_addr, ExprRef &sp_addr, ExprRef &rows, ExprRef &cols){

    caller.SetUpdate(load_svs.soc_mem_base_address, dram_addr);
    caller.SetUpdate(load_svs.spad_base_address, sp_addr);
    caller.SetUpdate(load_svs.num_rows, rows);
    caller.SetUpdate(load_svs.num_cols, cols);
    caller.SetUpdate(load_svs.child_valid, BoolConst(true));

}

void DefineGetSPAddrs(Ila &child, gemmini_statevars_t &svs){

    auto get_sp_addrs = child.NewInstr("loop_ws_get_sp_addrs");
    get_sp_addrs.SetDecode(svs.loop_ws.child_state == loop_ws_child_states::GET_SP_ADDRS);
    
    auto a_row      = Ite(svs.loop_ws.a_transpose, svs.loop_ws.k, svs.loop_ws.i).ZExt(32);
    auto a_num_cols = Ite(svs.loop_ws.a_transpose, svs.loop_ws.I, svs.loop_ws.K).ZExt(32);
    auto a_col      = Ite(svs.loop_ws.a_transpose, svs.loop_ws.i, svs.loop_ws.k).ZExt(32);
    get_sp_addrs.SetUpdate(svs.loop_ws.A_sp_addr, svs.loop_ws.A_sp_addr_start + (a_row * a_num_cols + a_col) * ARRAY_DIM);

    auto b_row      = Ite(svs.loop_ws.b_transpose, svs.loop_ws.j, svs.loop_ws.k).ZExt(32);
    auto b_num_cols = Ite(svs.loop_ws.b_transpose, svs.loop_ws.K, svs.loop_ws.J).ZExt(32);
    auto b_col      = Ite(svs.loop_ws.b_transpose, svs.loop_ws.k, svs.loop_ws.j).ZExt(32);
    get_sp_addrs.SetUpdate(svs.loop_ws.B_sp_addr, svs.loop_ws.B_sp_addr_start + (b_row * b_num_cols + b_col) * ARRAY_DIM);

    get_sp_addrs.SetUpdate(svs.loop_ws.C_sp_addr, svs.loop_ws.C_sp_addr_start + (svs.loop_ws.i.ZExt(32) * svs.loop_ws.J.ZExt(32) + svs.loop_ws.j.ZExt(32)) * ARRAY_DIM);

    auto next_child_state = Ite(svs.loop_ws.j == 0, BvConst(loop_ws_child_states::MVIN_A, svs.loop_ws.child_state.bit_width()),
                            Ite(svs.loop_ws.i == 0, BvConst(loop_ws_child_states::MVIN_B, svs.loop_ws.child_state.bit_width()),
                                                    BvConst(loop_ws_child_states::COMPUTE, svs.loop_ws.child_state.bit_width())));
    get_sp_addrs.SetUpdate(svs.loop_ws.child_state, next_child_state);

}

void DefineMvinA(Ila &child, gemmini_statevars_t &svs){

    auto mvin_a = child.NewInstr("loop_ws_mvin_a");
    mvin_a.SetDecode(svs.loop_ws.child_state == loop_ws_child_states::MVIN_A);

    auto a_row      = Ite(svs.loop_ws.a_transpose, svs.loop_ws.k, svs.loop_ws.i).ZExt(64);
    auto a_col      = Ite(svs.loop_ws.a_transpose, svs.loop_ws.i, svs.loop_ws.k).ZExt(64);
    auto a_num_rows = Ite(svs.loop_ws.a_transpose, svs.loop_ws.K, svs.loop_ws.I).ZExt(64);
    auto a_num_cols = Ite(svs.loop_ws.a_transpose, svs.loop_ws.I, svs.loop_ws.K).ZExt(64);
    
    auto dram_addr = svs.loop_ws.A + (a_row * svs.loop_ws.A_stride + a_col) * ARRAY_DIM * INPUT_TYPE_WIDTH_BYTES;
    
    auto col_dim_pad = Ite(svs.loop_ws.a_transpose, svs.loop_ws.pad_I, svs.loop_ws.pad_K);
    auto pad_cols = Ite(a_col == a_num_cols - 1, col_dim_pad, BvConst(0, 16));
    auto cols = BvConst(ARRAY_DIM, 16) - pad_cols;

    auto row_dim_pad = Ite(svs.loop_ws.a_transpose, svs.loop_ws.pad_K, svs.loop_ws.pad_I);
    auto pad_rows = Ite(a_row == a_num_rows - 1, row_dim_pad, BvConst(0, 16));
    auto rows = BvConst(ARRAY_DIM, 16) - pad_rows;

    _CallMvin(mvin_a, svs.load[0], dram_addr, svs.loop_ws.A_sp_addr, rows, cols);

    auto next_child_state = Ite(svs.loop_ws.i == 0, BvConst(loop_ws_child_states::MVIN_B, svs.loop_ws.child_state.bit_width()),
                                                    BvConst(loop_ws_child_states::COMPUTE, svs.loop_ws.child_state.bit_width()));
    mvin_a.SetUpdate(svs.loop_ws.child_state, next_child_state);
}

void DefineMvinB(Ila &child, gemmini_statevars_t &svs){

    auto mvin_b = child.NewInstr("loop_ws_mvin_b");
    mvin_b.SetDecode(svs.loop_ws.child_state == loop_ws_child_states::MVIN_B);

    auto b_row      = Ite(svs.loop_ws.b_transpose, svs.loop_ws.j, svs.loop_ws.k).ZExt(64);
    auto b_col      = Ite(svs.loop_ws.b_transpose, svs.loop_ws.k, svs.loop_ws.j).ZExt(64);
    auto b_num_rows = Ite(svs.loop_ws.b_transpose, svs.loop_ws.J, svs.loop_ws.K).ZExt(64);
    auto b_num_cols = Ite(svs.loop_ws.b_transpose, svs.loop_ws.K, svs.loop_ws.J).ZExt(64);

    auto dram_addr = svs.loop_ws.B + (b_row * svs.loop_ws.B_stride + b_col) * ARRAY_DIM * INPUT_TYPE_WIDTH_BYTES;
    
    auto col_dim_pad = Ite(svs.loop_ws.b_transpose, svs.loop_ws.pad_K, svs.loop_ws.pad_J);
    auto pad_cols = Ite(b_col == b_num_cols - 1, col_dim_pad, BvConst(0, 16));
    auto cols = BvConst(ARRAY_DIM, 16) - pad_cols;

    auto row_dim_pad = Ite(svs.loop_ws.b_transpose, svs.loop_ws.pad_J, svs.loop_ws.pad_K);
    auto pad_rows = Ite(b_row == b_num_rows - 1, row_dim_pad, BvConst(0, 16));
    auto rows = BvConst(ARRAY_DIM, 16) - pad_rows;

    _CallMvin(mvin_b, svs.load[1], dram_addr, svs.loop_ws.B_sp_addr, rows, cols);

    auto next_child_state = BvConst(loop_ws_child_states::COMPUTE, svs.loop_ws.child_state.bit_width());
    mvin_b.SetUpdate(svs.loop_ws.child_state, next_child_state);

}
// void DefineCompute(Ila &child, gemmini_statevars_t &svs);
// void DefineMvoutC(Ila &child, gemmini_statevars_t &svs);
// void DefineIterate(Ila &child, gemmini_statevars_t &svs);



}

}