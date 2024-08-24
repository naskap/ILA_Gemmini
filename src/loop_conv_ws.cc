#include <Gemmini/loop_conv_ws.h>


namespace ilang {

namespace Gemmini {

void DefineLoopConvWS(Ila& m, command_t& command, gemmini_memory_t memory, gemmini_statevars_t &svs){

    DefineLoopConvWSStatevars(m, svs.loop_conv);
    DefineLoopConvWSConfig1(m, command, svs.loop_conv);
    DefineLoopConvWSConfig2(m, command, svs.loop_conv);
    DefineLoopConvWSConfig3(m, command, svs.loop_conv);
    DefineLoopConvWSConfig4(m, command, svs.loop_conv);
    DefineLoopConvWSConfig5(m, command, svs.loop_conv);
    DefineLoopConvWSConfig6(m, command, svs.loop_conv);
    DefineLoopConvWSInstruction(m, command, svs); 

}

void DefineLoopConvWSStatevars(Ila &m, loop_conv_ws_statevars_t &svs){

    // Configs
    svs.batch_size      = m.NewBvState("loop_conv_ws_batch_size", 16);
    svs.in_dim          = m.NewBvState("loop_conv_ws_in_dim", 16);
    svs.in_channels     = m.NewBvState("loop_conv_ws_in_channels", 16);
    svs.out_channels    = m.NewBvState("loop_conv_ws_out_channels", 16);
    svs.out_dim         = m.NewBvState("loop_conv_ws_out_dim", 16);
    svs.pool_out_dim    = m.NewBvState("loop_conv_ws_pool_out_dim", 16);
    svs.stride          = m.NewBvState("loop_conv_ws_stride", 16);
    svs.padding         = m.NewBvState("loop_conv_ws_padding", 16);
    svs.kernel_dim      = m.NewBvState("loop_conv_ws_kernel_dim", 16);
    svs.pool_size       = m.NewBvState("loop_conv_ws_pool_size", 16);
    svs.pool_stride     = m.NewBvState("loop_conv_ws_pool_stride", 16);
    svs.pool_padding    = m.NewBvState("loop_conv_ws_pool_padding", 16);
    svs.batches         = m.NewBvState("loop_conv_ws_batches", 16);
    svs.porows          = m.NewBvState("loop_conv_ws_porows", 16);
    svs.pocols          = m.NewBvState("loop_conv_ws_pocols", 16);
    svs.ochs            = m.NewBvState("loop_conv_ws_pochs", 16);
    svs.krows           = m.NewBvState("loop_conv_ws_krows", 16);
    svs.kcols           = m.NewBvState("loop_conv_ws_kcols", 16);
    svs.kchs            = m.NewBvState("loop_conv_ws_kchs", 16);
    svs.lpad            = m.NewBvState("loop_conv_ws_lpad", 16);
    svs.rpad            = m.NewBvState("loop_conv_ws_rpad", 16);
    svs.upad            = m.NewBvState("loop_conv_ws_upad", 16);
    svs.dpad            = m.NewBvState("loop_conv_ws_dpad", 16);
    svs.plpad           = m.NewBvState("loop_conv_ws_plpad", 16);
    svs.orows           = m.NewBvState("loop_conv_ws_orows", 16);
    svs.prad            = m.NewBvState("loop_conv_ws_prad", 16);
    svs.pupad           = m.NewBvState("loop_conv_ws_pupad", 16);
    svs.pdpad           = m.NewBvState("loop_conv_ws_pdpad", 16);
    svs.kernel_dilation = m.NewBvState("loop_conv_ws_kernel_dilation", 16);
    svs.ocols           = m.NewBvState("loop_conv_ws_ocols", 16);
    svs.weights         = m.NewBvState("loop_conv_ws_weights", 64);
    svs.output          = m.NewBvState("loop_conv_ws_output", 64);
    svs.bias            = m.NewBvState("loop_conv_ws_bias", 64);
    svs.input           = m.NewBvState("loop_conv_ws_input", 64);

    // Command args
    svs.no_bias            = m.NewBoolState("loop_conv_ws_no_bias");
    svs.wrot180            = m.NewBoolState("loop_conv_ws_wrot180");
    svs.trans_output_1203  = m.NewBoolState("loop_conv_ws_trans_output_1203");
    svs.trans_weight_1203  = m.NewBoolState("loop_conv_ws_trans_weight_1203");
    svs.trans_weight_0132  = m.NewBoolState("loop_conv_ws_trans_weight_0132");
    svs.trans_input_3120   = m.NewBoolState("loop_conv_ws_trans_input_3120");
    svs.dw                 = m.NewBoolState("loop_conv_ws_dw");
    svs.max_pixels_per_row = m.NewBvState("loop_conv_ws_max_pixels_per_row", 8);
    svs.no_pool            = m.NewBoolState("loop_conv_ws_no_pool");
    svs.downsample         = m.NewBvState("loop_conv_ws_downsample", 1);
    svs.input_dilated      = m.NewBvState("loop_conv_ws_input_dilated", 1);
    svs.activation         = m.NewBvState("loop_conv_ws_activation", 2);

    // Helpers
    svs.b                               = m.NewBvState("loop_conv_ws_b", 16);
    svs.dilated_krows                   = m.NewBvState("loop_conv_ws_dilated_krows", 16);
    svs.dilated_kcols                   = m.NewBvState("loop_conv_ws_dilated_kcols", 16);
    svs.irows_without_dilation          = m.NewBvState("loop_conv_ws_irows_without_dilation", 16);
    svs.icols_without_dilation          = m.NewBvState("loop_conv_ws_icols_without_dilation", 16);
    svs.irows_unpadded_without_dilation = m.NewBvState("loop_conv_ws_irows_unpadded_without_dilation", 16);
    svs.icols_unpadded_without_dilation = m.NewBvState("loop_conv_ws_icols_unpadded_without_dilation", 16);
    svs.irows_unpadded                  = m.NewBvState("loop_conv_ws_irows_unpadded", 16);
    svs.icols_unpadded                  = m.NewBvState("loop_conv_ws_icols_unpadded", 16);
    svs.irows                           = m.NewBvState("loop_conv_ws_irows", 16);
    svs.icols                           = m.NewBvState("loop_conv_ws_icols", 16);
    svs.out_channels_per_bank           = m.NewBvState("loop_conv_ws_out_channels_per_bank", 32);
    svs.in_channels_per_bank            = m.NewBvState("loop_conv_ws_in_channels_per_bank", 32);
    svs.B_rows                          = m.NewBvState("loop_conv_ws_B_rows", 32);
    // svs.D_sp_addr_row                   = m.NewBvState("loop_conv_ws_D_sp_addr_row", 32);
    // svs.C_sp_addr_row                   = m.NewBvState("loop_conv_ws_C_sp_addr_row", 32);
    svs.A_sp_addr_start   = m.NewBvState("loop_conv_ws_A_sp_addr_start", 32);
    svs.B_sp_addr_start   = m.NewBvState("loop_conv_ws_B_sp_addr_start", 32);
    svs.D_sp_addr_start   = m.NewBvState("loop_conv_ws_D_sp_addr_start", 32);
    svs.C_sp_addr_start   = m.NewBvState("loop_conv_ws_C_sp_addr_start", 32);
    svs.new_weights       = m.NewBvState("loop_conv_ws_new_weights", 32);
    svs.max_ochs_per_mvin = m.NewBvState("loop_conv_ws_max_ochs_per_mvin", 16);
    svs.max_chs_per_mvin  = m.NewBvState("loop_conv_ws_max_chs_per_mvin", 16);
    svs.spad_stride       = m.NewBvState("loop_conv_ws_spad_stride", 32);
    svs.orow              = m.NewBvState("loop_conv_ws_orow", 16);
    svs.ocol              = m.NewBvState("loop_conv_ws_ocol", 16);
    svs.och               = m.NewBvState("loop_conv_ws_och", 16);
    svs.irow              = m.NewBvState("loop_conv_ws_irow", 16);
    svs.icol              = m.NewBvState("loop_conv_ws_icol", 16);
    svs.ich               = m.NewBvState("loop_conv_ws_ich", 16);
    svs.krow              = m.NewBvState("loop_conv_ws_krow", 16);
    svs.kcol              = m.NewBvState("loop_conv_ws_kcol", 16);
    svs.kch               = m.NewBvState("loop_conv_ws_kch", 16);
    svs.child_state       = m.NewBvState("loop_conv_ws_child_state", 32);

}

void DefineLoopConvWSConfig1(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs){

    auto config1 = m.NewInstr("loop_conv_ws_config1");
    config1.SetDecode(command.funct == BvConst(LOOP_CONV_WS_CONFIG_1, INSTR_FUNCT_WIDTH));

    config1.SetUpdate(svs.batch_size, Extract(command.rs1, 15, 0));
    config1.SetUpdate(svs.in_dim, Extract(command.rs1, 31, 16));
    config1.SetUpdate(svs.in_channels, Extract(command.rs1, 47, 32));
    config1.SetUpdate(svs.out_channels, Extract(command.rs1, 63, 48));
    config1.SetUpdate(svs.out_dim, Extract(command.rs2, 15, 0));
    config1.SetUpdate(svs.pool_out_dim, Extract(command.rs2, 31, 16));
    config1.SetUpdate(svs.stride, Extract(command.rs2, 47, 32));
    config1.SetUpdate(svs.padding, Extract(command.rs2, 63, 48));

}
void DefineLoopConvWSConfig2(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs){

    auto config2 = m.NewInstr("loop_conv_ws_config2");
    config2.SetDecode(command.funct == BvConst(LOOP_CONV_WS_CONFIG_2, INSTR_FUNCT_WIDTH));

    config2.SetUpdate(svs.kernel_dim, Extract(command.rs1, 63, 48));
    config2.SetUpdate(svs.pool_size, Extract(command.rs1, 47, 32));
    config2.SetUpdate(svs.pool_stride, Extract(command.rs1, 31, 16));
    config2.SetUpdate(svs.pool_padding, Extract(command.rs1, 15, 0));
    config2.SetUpdate(svs.batches, Extract(command.rs2, 63, 48));
    config2.SetUpdate(svs.porows, Extract(command.rs2, 47, 32));
    config2.SetUpdate(svs.pocols, Extract(command.rs2, 31, 16));
    config2.SetUpdate(svs.ochs, Extract(command.rs2, 15, 0));

}
void DefineLoopConvWSConfig3(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs){

    auto config3 = m.NewInstr("loop_conv_ws_config3");
    config3.SetDecode(command.funct == BvConst(LOOP_CONV_WS_CONFIG_3, INSTR_FUNCT_WIDTH));

    config3.SetUpdate(svs.krows, Extract(command.rs1, 63, 48));
    config3.SetUpdate(svs.kcols, Extract(command.rs1, 47, 32));
    config3.SetUpdate(svs.kchs, Extract(command.rs1, 31, 16));
    config3.SetUpdate(svs.lpad, Extract(command.rs1, 15, 0));
    config3.SetUpdate(svs.rpad, Extract(command.rs2, 63, 48));
    config3.SetUpdate(svs.upad, Extract(command.rs2, 47, 32));
    config3.SetUpdate(svs.dpad, Extract(command.rs2, 31, 16));
    config3.SetUpdate(svs.plpad, Extract(command.rs2, 15, 0));

}
void DefineLoopConvWSConfig4(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs){

    auto config4 = m.NewInstr("loop_conv_ws_config4");
    config4.SetDecode(command.funct == BvConst(LOOP_CONV_WS_CONFIG_4, INSTR_FUNCT_WIDTH));

    config4.SetUpdate(svs.orows, Extract(command.rs1, 63, 48));
    config4.SetUpdate(svs.prad, Extract(command.rs1, 47, 32));
    config4.SetUpdate(svs.pupad, Extract(command.rs1, 31, 16));
    config4.SetUpdate(svs.pdpad, Extract(command.rs1, 15, 0));
    config4.SetUpdate(svs.kernel_dilation, Extract(command.rs2, 31, 16));
    config4.SetUpdate(svs.ocols, Extract(command.rs2, 15, 0));

}
void DefineLoopConvWSConfig5(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs){

    auto config5 = m.NewInstr("loop_conv_ws_config5");
    config5.SetDecode(command.funct == BvConst(LOOP_CONV_WS_CONFIG_5, INSTR_FUNCT_WIDTH));

    config5.SetUpdate(svs.weights, command.rs1);
    config5.SetUpdate(svs.output, command.rs2);

}
void DefineLoopConvWSConfig6(Ila &m, command_t &command, loop_conv_ws_statevars_t &svs){

    auto config6 = m.NewInstr("loop_conv_ws_config6");
    config6.SetDecode(command.funct == BvConst(LOOP_CONV_WS_CONFIG_6, INSTR_FUNCT_WIDTH));

    config6.SetUpdate(svs.bias, command.rs1);
    config6.SetUpdate(svs.input, command.rs2);

}

ExprRef _UNDILATED(ExprRef &x, ExprRef &input_dilated){
    auto input_dilated_extended = input_dilated.ZExt(x.bit_width());
    return (((x) + (input_dilated_extended)) >> (input_dilated_extended));
}

void DefineLoopConvWSInstruction(Ila &m, command_t &command, gemmini_statevars_t &all_svs){

    auto loop_conv = m.NewInstr("loop_conv_ws");
    loop_conv.SetDecode(command.funct == BvConst(LOOP_CONV_WS, INSTR_FUNCT_WIDTH));

    auto conv_svs = all_svs.loop_conv;

    // Store command args in state vars
    loop_conv.SetUpdate(conv_svs.no_bias, SelectBit(command.rs1, 0) == 1);
    loop_conv.SetUpdate(conv_svs.wrot180, SelectBit(command.rs1, 1) == 1);
    loop_conv.SetUpdate(conv_svs.trans_output_1203, SelectBit(command.rs1, 2) == 1);
    loop_conv.SetUpdate(conv_svs.trans_weight_1203, SelectBit(command.rs1, 3) == 1);
    loop_conv.SetUpdate(conv_svs.trans_weight_0132, SelectBit(command.rs1, 4) == 1);
    loop_conv.SetUpdate(conv_svs.trans_input_3120, SelectBit(command.rs1, 5) == 1);
    loop_conv.SetUpdate(conv_svs.dw, SelectBit(command.rs1, 6) == 1);
    
    auto max_pixels_per_row = Extract(command.rs1, 15, 8);
    auto mppr_zero_is_one   = Ite(max_pixels_per_row == 0, BvConst(1,8), max_pixels_per_row);
    loop_conv.SetUpdate(conv_svs.max_pixels_per_row, mppr_zero_is_one);
    
    loop_conv.SetUpdate(conv_svs.no_pool, SelectBit(command.rs2, 0) == 1);
    loop_conv.SetUpdate(conv_svs.downsample, SelectBit(command.rs2, 1));

    auto input_dilated_next = SelectBit(command.rs2, 2);
    loop_conv.SetUpdate(conv_svs.input_dilated, input_dilated_next);
    loop_conv.SetUpdate(conv_svs.activation, Extract(command.rs2, 4, 3));

    auto dilated_krows                   = conv_svs.krows + (conv_svs.kernel_dilation - 1)*(conv_svs.krows - 1);
    auto dilated_kcols                   = conv_svs.kcols + (conv_svs.kernel_dilation - 1)*(conv_svs.kcols - 1);
    auto irows_without_dilation          = conv_svs.orows * conv_svs.stride + dilated_krows - 1;
    auto icols_without_dilation          = conv_svs.ocols * conv_svs.stride + dilated_kcols - 1;
    auto irows_unpadded_without_dilation = conv_svs.irows_without_dilation - conv_svs.upad - conv_svs.dpad;
    auto icols_unpadded_without_dilation = conv_svs.icols_without_dilation - conv_svs.lpad - conv_svs.rpad;

    auto irows_unpadded_next = Ite(input_dilated_next == 1, (irows_unpadded_without_dilation + 1)/BvConst(2,16), irows_unpadded_without_dilation);
    auto icols_unpadded_next = Ite(input_dilated_next == 1, (icols_unpadded_without_dilation + 1)/BvConst(2,16), icols_unpadded_without_dilation);
    loop_conv.SetUpdate(conv_svs.irows_unpadded, irows_unpadded_next);
    loop_conv.SetUpdate(conv_svs.icols_unpadded, icols_unpadded_next);

    loop_conv.SetUpdate(conv_svs.irows, Ite(input_dilated_next == 1, irows_unpadded_next + _UNDILATED(conv_svs.upad, input_dilated_next) + _UNDILATED(conv_svs.dpad, input_dilated_next), irows_without_dilation));
    loop_conv.SetUpdate(conv_svs.icols, Ite(input_dilated_next == 1, icols_unpadded_next + _UNDILATED(conv_svs.lpad, input_dilated_next) + _UNDILATED(conv_svs.rpad, input_dilated_next), icols_without_dilation));

    auto array_dim_const       = BvConst(ARRAY_DIM, 16);
    auto out_channels_per_bank = conv_svs.ochs / array_dim_const + Mod(conv_svs.ochs, array_dim_const);
    auto in_channels_per_bank  = conv_svs.kchs / array_dim_const + Mod(conv_svs.kchs, array_dim_const);
    auto B_rows                = Ite(conv_svs.trans_weight_0132, in_channels_per_bank * conv_svs.kcols * conv_svs.krows * conv_svs.ochs, 
                                                    out_channels_per_bank * conv_svs.kcols * conv_svs.krows * conv_svs.kchs);

    loop_conv.SetUpdate(conv_svs.A_sp_addr_start, BvConst(0,32));
    loop_conv.SetUpdate(conv_svs.B_sp_addr_start, BvConst((SPAD_BANKS * SPAD_ROWS), 32) - B_rows.ZExt(32));
    loop_conv.SetUpdate(conv_svs.D_sp_addr_start, BvConst(1 << (SPAD_ADDRESS_WIDTH - 1),32));
    loop_conv.SetUpdate(conv_svs.C_sp_addr_start, BvConst((3 << (SPAD_ADDRESS_WIDTH - 2)), 32));

    auto const_zero = BvConst(0, 32);
    loop_conv.SetUpdate(conv_svs.b, const_zero);
    loop_conv.SetUpdate(conv_svs.orow, const_zero);     
    loop_conv.SetUpdate(conv_svs.ocol, const_zero);     
    loop_conv.SetUpdate(conv_svs.och, const_zero);      
    loop_conv.SetUpdate(conv_svs.irow, const_zero);     
    loop_conv.SetUpdate(conv_svs.icol, const_zero);     
    loop_conv.SetUpdate(conv_svs.ich, const_zero);      
    loop_conv.SetUpdate(conv_svs.krow, const_zero);     
    loop_conv.SetUpdate(conv_svs.kcol, const_zero);     
    loop_conv.SetUpdate(conv_svs.kch, const_zero);  


    loop_conv.SetUpdate(conv_svs.child_state, Ite(conv_svs.bias != 0, BvConst(loop_conv_ws_child_states::CONFIG_MVIN_BIAS, 32), 
                                                                BvConst(loop_conv_ws_child_states::CONFIG_MVIN_INPUT, 32))); 

    // Define Child Ila
    auto child = m.NewChild("loop_conv_ws");

    auto loads_inactive = BoolConst(true);
    for(int i=0; i<NUM_MVIN_CONFIG_SETS; i++){
        loads_inactive = loads_inactive & (all_svs.load[i].child_valid == false);
    }
    child.SetValid((all_svs.loop_ws.child_state != loop_conv_ws_child_states::LOOP_CONV_WS_INACTIVE) & loads_inactive & (all_svs.store.child_valid == false) & (all_svs.exec.child_state == 0));   

    // Define child instructions
    DefineConfigMvinBias(child, all_svs);
    DefineMvinBias(child, all_svs);
    DefineConfigMvinInput(child, all_svs);
    DefineMvinInput(child, all_svs);
    DefineConfigMvinWeights(child, all_svs);
    DefineMvinWeights(child, all_svs);
    DefineConfigCompute(child, all_svs);
    DefineIterateOcol(child, all_svs);
    DefineConfigPreloadCol(child, all_svs);
    DefineComputeConv(child, all_svs);
    DefineMvoutResultsNoPool(child, all_svs);
    DefineConfigMvoutResultsPool(child, all_svs);
    DefineMvoutResultsPool(child, all_svs);
    DefineConfigResetMvoutResults(child, all_svs);
}

void _ConfigMvinUpdateStatevars(InstrRef &caller, load_statevars_t &load_svs, ExprRef const &read_inputType, ExprRef const &dest_stride, 
                                                                    ExprRef const &scale, ExprRef const &src_stride, ExprRef const &pixels_per_row){
    caller.SetUpdate(load_svs.read_inputType, read_inputType == 1);
    caller.SetUpdate(load_svs.dest_stride, dest_stride);
    caller.SetUpdate(load_svs.scale, scale);
    caller.SetUpdate(load_svs.src_stride, src_stride);
    auto pixels_per_row_zero_is_one = Ite(pixels_per_row == 0, BvConst(1,8), pixels_per_row);
    caller.SetUpdate(load_svs.pixels_per_row, pixels_per_row_zero_is_one);  
}

void DefineConfigMvinBias(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_CONFIG_MVIN_BIAS");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::CONFIG_MVIN_BIAS, svs.loop_conv.child_state.bit_width()));
    
    // Set max_ochs_per_mvin
    auto dim_blocks_len    = BvConst(MAX_BLOCK_LEN_ACC * ARRAY_DIM, 16);
    auto max_ochs_per_mvin = Min(svs.loop_conv.ochs, dim_blocks_len);
    instr.SetUpdate(svs.loop_conv.max_ochs_per_mvin, max_ochs_per_mvin);

    // Set mvin configs
    auto read_inputtype = BoolConst(false);
    auto spad_stride    = svs.loop_conv.batches * svs.loop_conv.orows * svs.loop_conv.ocols;
    auto scale          = BvConst(_bit_cast<uint32_t, float>(1), 32);                         // Bitcast a float scale to an int
    auto src_stride     = BvConst(0, 64);
    auto pixels_per_row = BvConst(1, 8);
    _ConfigMvinUpdateStatevars(instr, svs.load[2], read_inputtype, spad_stride, scale, src_stride, pixels_per_row);

    instr.SetUpdate(svs.loop_conv.child_state,  BvConst(loop_conv_ws_child_states::MVIN_BIAS, svs.loop_conv.child_state.bit_width()));

}

void DefineMvinBias(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_MVIN_BIAS");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::MVIN_BIAS, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
    auto array_dim_const = BvConst(ARRAY_DIM, 16);
    auto ocols           = svs.loop_conv.ocols;
    auto ocol            = svs.loop_conv.ocol;
    auto I               = Min(ocols - ocol, array_dim_const);

    auto ochs = svs.loop_conv.ochs;
    auto och  = svs.loop_conv.och;
    auto J    = Min(ochs - och, array_dim_const);

    auto D_sp_addr = svs.loop_conv.D_sp_addr_start + ((och/array_dim_const).ZExt(32) * svs.loop_conv.batches.ZExt(32) * svs.loop_conv.orows.ZExt(32) * ocols.ZExt(32))
                                                   + svs.loop_conv.b.ZExt(32) * svs.loop_conv.orows.ZExt(32) * ocols.ZExt(32)
                                                   + ocol.ZExt(32);

    auto dram_addr = Ite(svs.loop_conv.no_bias, BvConst(0, 64), 
                                                svs.loop_conv.bias.ZExt(64) + och.ZExt(64) * ACC_TYPE_WIDTH_BYTES);


    CallMvin(instr, svs.load[2], dram_addr, D_sp_addr, I, J);

    auto const_one = BvConst(1, 32);
    std::vector<ExprRef> iteration_vars       = {svs.loop_conv.b, svs.loop_conv.orow, svs.loop_conv.ocol, svs.loop_conv.och};
    std::vector<ExprRef> iteration_increments = {const_one, const_one, array_dim_const.ZExt(32), svs.loop_conv.max_ochs_per_mvin};
    std::vector<ExprRef> iteration_maxs       = {svs.loop_conv.batches, svs.loop_conv.orows, svs.loop_conv.ocols, svs.loop_conv.ochs};
    auto last_pixel                           = IterateLoopVars(instr, iteration_vars, iteration_maxs);
    instr.SetUpdate(svs.loop_conv.child_state, Ite(last_pixel, BvConst(loop_conv_ws_child_states::CONFIG_MVIN_INPUT, svs.loop_conv.child_state.bit_width()),
                                                               BvConst(loop_conv_ws_child_states::MVIN_BIAS, svs.loop_conv.child_state.bit_width())));
                                                               
}

#define DS(x) ((x) >> (svs.loop_conv.downsample.ZExt(x.bit_width())))
#define US(x) ((x) << (svs.loop_conv.downsample.ZExt(x.bit_width())))

void DefineConfigMvinInput(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_CONFIG_MVIN_INPUT");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::CONFIG_MVIN_INPUT, svs.loop_conv.child_state.bit_width()));
    

    auto dim_blocks_max_len = BvConst(ARRAY_DIM * MAX_BLOCK_LEN, 16);
    auto max_chs_per_mvin = Ite(svs.loop_conv.trans_input_3120, 
                                Min(svs.loop_conv.batches, dim_blocks_max_len),
                                Min(svs.loop_conv.kchs, dim_blocks_max_len));
    instr.SetUpdate(svs.loop_conv.max_chs_per_mvin, max_chs_per_mvin);
    
    auto dram_stride = Ite(svs.loop_conv.trans_input_3120, svs.loop_conv.batch_size * INPUT_TYPE_WIDTH_BYTES,
                                                            svs.loop_conv.in_channels * INPUT_TYPE_WIDTH_BYTES);
                                    
    auto spad_stride = Ite(svs.loop_conv.trans_input_3120, svs.loop_conv.kchs * DS(svs.loop_conv.irows) * DS(svs.loop_conv.icols),
                                                            svs.loop_conv.batches * DS(svs.loop_conv.irows) * DS(svs.loop_conv.icols));

    auto read_inputtype = BoolConst(false);
    auto scale          = BvConst(_bit_cast<uint32_t, float>(1), 32);   // Bitcast a float scale to an int
    _ConfigMvinUpdateStatevars(instr, svs.load[0], read_inputtype, spad_stride, scale, US(dram_stride), svs.loop_conv.max_pixels_per_row);

    instr.SetUpdate(svs.loop_conv.irow, _UNDILATED(svs.loop_conv.upad, svs.loop_conv.input_dilated) * -1);
    instr.SetUpdate(svs.loop_conv.icol, _UNDILATED(svs.loop_conv.lpad, svs.loop_conv.input_dilated) * -1);

    instr.SetUpdate(svs.loop_conv.child_state, BvConst(loop_conv_ws_child_states::MVIN_INPUT,16));
   
}
void DefineMvinInput(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_MVIN_INPUT");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::MVIN_INPUT, svs.loop_conv.child_state.bit_width()));
    

    // Put some statevars in named variabled for simplicity
    auto irow = svs.loop_conv.irow;
    auto irows = svs.loop_conv.irows;
    auto irows_unpadded = svs.loop_conv.irows_unpadded;
    auto icol = svs.loop_conv.icol;
    auto icols = svs.loop_conv.icols;
    auto icols_unpadded = svs.loop_conv.icols_unpadded;


    auto irow_padded = irow + _UNDILATED(svs.loop_conv.upad, svs.loop_conv.input_dilated);
    
    auto array_dim_const = BvConst(ARRAY_DIM, 16);
    auto I = Ite(icol < 0, Min(icol * -1, array_dim_const), 
                 Ite(icol >= icols_unpadded, 
                    Min(icols_unpadded + _UNDILATED(svs.loop_conv.rpad, svs.loop_conv.input_dilated) + icol, array_dim_const),
                Min(icols_unpadded - icol, US(array_dim_const))));
    
    auto icol_padded = icol + _UNDILATED(svs.loop_conv.lpad, svs.loop_conv.input_dilated);

    auto K = Ite(svs.loop_conv.trans_input_3120, Min(svs.loop_conv.batches - svs.loop_conv.b, svs.loop_conv.max_chs_per_mvin),
                        Min(svs.loop_conv.kchs - svs.loop_conv.ich, svs.loop_conv.max_chs_per_mvin));
    
    auto A_sp_addr = svs.loop_conv.A_sp_addr_start + (svs.loop_conv.ich / array_dim_const).ZExt(32) * svs.loop_conv.spad_stride.ZExt(32) 
                                                    + svs.loop_conv.b.ZExt(32) * DS(irows).ZExt(32) * DS(icols).ZExt(32) 
                                                    + DS(irow_padded).ZExt(32) * DS(icols).ZExt(32) * DS(icol_padded).ZExt(32);

    auto is_zeros = (irow < 0) | (irow >= irows_unpadded) 
                               | (icol < 0)
                               | (icol >= icols_unpadded);

    
    auto in = svs.loop_conv.input + ((svs.loop_conv.b.ZExt(32) * svs.loop_conv.in_dim.ZExt(32) * svs.loop_conv.in_dim.ZExt(32) 
                                        + irow.ZExt(32) * svs.loop_conv.in_dim.ZExt(32) 
                                        + icol.ZExt(32))  
                                    * svs.loop_conv.in_channels.ZExt(32) +svs.loop_conv.ich.ZExt(32)).ZExt(64) * INPUT_TYPE_WIDTH_BYTES;

    in = Ite(is_zeros, BvConst(0, 64), in);
    in = Ite(svs.loop_conv.trans_input_3120, 
            svs.loop_conv.input + ((svs.loop_conv.ich.ZExt(32) * svs.loop_conv.in_dim.ZExt(32) * svs.loop_conv.in_dim.ZExt(32) 
                                        + irow.ZExt(32) * svs.loop_conv.in_dim.ZExt(32) 
                                        + icol.ZExt(32))  
                                    * svs.loop_conv.batch_size.ZExt(32) +svs.loop_conv.b.ZExt(32)).ZExt(64) * INPUT_TYPE_WIDTH_BYTES,
            in);   


    auto const_one = BvConst(1, 16);
    auto b_it    = Ite(svs.loop_conv.trans_input_3120, svs.loop_conv.max_chs_per_mvin, const_one);  
    auto ich_it  = Ite(svs.loop_conv.trans_input_3120, const_one, svs.loop_conv.max_chs_per_mvin);                      

    
    std::vector<ExprRef> iteration_vars        = {svs.loop_conv.b, irow, icol, svs.loop_conv.ich};
    std::vector<ExprRef> iteration_init_values = {BvConst(0,16), _UNDILATED(svs.loop_conv.upad, svs.loop_conv.input_dilated) * -1,
                                                  _UNDILATED(svs.loop_conv.lpad, svs.loop_conv.input_dilated) * -1, BvConst(0,16)};
    std::vector<ExprRef> iteration_increments  = {b_it,  US(const_one), I, ich_it};
    std::vector<ExprRef> iteration_maxs        = {svs.loop_conv.batches, irows_unpadded + _UNDILATED(svs.loop_conv.dpad, svs.loop_conv.input_dilated), 
                                                icols_unpadded + _UNDILATED(svs.loop_conv.rpad, svs.loop_conv.input_dilated), svs.loop_conv.kchs};
    auto last_pixel                           = IterateLoopVars(instr, iteration_vars, iteration_maxs);

    instr.SetUpdate(svs.loop_conv.child_state, Ite(last_pixel, BvConst(loop_conv_ws_child_states::CONFIG_MVIN_WEIGHTS,16), 
                                                            BvConst(loop_conv_ws_child_states::MVIN_INPUT,16)));
}
void DefineConfigMvinWeights(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_CONFIG_MVIN_WEIGHTS");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::CONFIG_MVIN_WEIGHTS, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineMvinWeights(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_MVIN_WEIGHTS");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::MVIN_WEIGHTS, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineConfigCompute(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_CONFIG_COMPUTE");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::CONFIG_COMPUTE, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineIterateOcol(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_ITERATE_OCOL");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::ITERATE_OCOL, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineConfigPreloadCol(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_CONFIG_PRELOAD_CONV");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::CONFIG_PRELOAD_CONV, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineComputeConv(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_COMPUTE_CONV");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::COMPUTE_CONV, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineMvoutResultsNoPool(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_MVOUT_RESULTS_NOPOOL");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::MVOUT_RESULTS_NOPOOL, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineConfigMvoutResultsPool(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_CONFIG_MVOUT_RESULTS_POOL");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::CONFIG_MVOUT_RESULTS_POOL, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineMvoutResultsPool(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_MVOUT_RESULTS_POOL");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::MVOUT_RESULTS_POOL, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}
void DefineConfigResetMvoutResults(Ila &child, gemmini_statevars_t &svs){

    // Define instruction
    auto instr = child.NewInstr("loop_conv_ws_CONFIG_RESET_MVOUT_RESULTS");
    instr.SetDecode(svs.loop_conv.child_state == BvConst(loop_conv_ws_child_states::CONFIG_RESET_MVOUT_RESULTS, svs.loop_conv.child_state.bit_width()));
    
    // Update statevars
}


}

}