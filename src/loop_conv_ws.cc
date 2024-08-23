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
    svs.batch_size = m.NewBvState("loop_conv_ws_batch_size", 16);
    svs.in_dim = m.NewBvState("loop_conv_ws_in_dim", 16);
    svs.in_channels = m.NewBvState("loop_conv_ws_in_channels", 16);
    svs.out_channels = m.NewBvState("loop_conv_ws_out_channels", 16);
    svs.out_dim = m.NewBvState("loop_conv_ws_out_dim", 16);
    svs.pool_out_dim = m.NewBvState("loop_conv_ws_pool_out_dim", 16);
    svs.stride = m.NewBvState("loop_conv_ws_stride", 16);
    svs.padding = m.NewBvState("loop_conv_ws_padding", 16);
    svs.kernel_dim = m.NewBvState("loop_conv_ws_kernel_dim", 16);
    svs.pool_size = m.NewBvState("loop_conv_ws_pool_size", 16);
    svs.pool_stride = m.NewBvState("loop_conv_ws_pool_stride", 16);
    svs.pool_padding = m.NewBvState("loop_conv_ws_pool_padding", 16);
    svs.batches = m.NewBvState("loop_conv_ws_batches", 16);
    svs.porows = m.NewBvState("loop_conv_ws_porows", 16);
    svs.pocols = m.NewBvState("loop_conv_ws_pocols", 16);
    svs.pochs = m.NewBvState("loop_conv_ws_pochs", 16);
    svs.krows = m.NewBvState("loop_conv_ws_krows", 16);
    svs.kcols = m.NewBvState("loop_conv_ws_kcols", 16);
    svs.kchs = m.NewBvState("loop_conv_ws_kchs", 16);
    svs.lpad = m.NewBvState("loop_conv_ws_lpad", 16);
    svs.rpad = m.NewBvState("loop_conv_ws_rpad", 16);
    svs.upad = m.NewBvState("loop_conv_ws_upad", 16);
    svs.dpad = m.NewBvState("loop_conv_ws_dpad", 16);
    svs.plpad = m.NewBvState("loop_conv_ws_plpad", 16);
    svs.orows = m.NewBvState("loop_conv_ws_orows", 16);
    svs.prad = m.NewBvState("loop_conv_ws_prad", 16);
    svs.pupad = m.NewBvState("loop_conv_ws_pupad", 16);
    svs.pdpad = m.NewBvState("loop_conv_ws_pdpad", 16);
    svs.kernel_dilation = m.NewBvState("loop_conv_ws_kernel_dilation", 16);
    svs.ocols = m.NewBvState("loop_conv_ws_ocols", 16);
    svs.weights = m.NewBvState("loop_conv_ws_weights", 64);
    svs.output = m.NewBvState("loop_conv_ws_output", 64);
    svs.bias = m.NewBvState("loop_conv_ws_bias", 64);
    svs.input = m.NewBvState("loop_conv_ws_input", 64);

    // Command args
    svs.no_bias = m.NewBoolState("loop_conv_ws_no_bias");
    svs.wrot180 = m.NewBoolState("loop_conv_ws_wrot180");
    svs.trans_output_1203 = m.NewBoolState("loop_conv_ws_trans_output_1203");
    svs.trans_weight_1203 = m.NewBoolState("loop_conv_ws_trans_weight_1203");
    svs.trans_weight_0132 = m.NewBoolState("loop_conv_ws_trans_weight_0132");
    svs.trans_input_3120 = m.NewBoolState("loop_conv_ws_trans_input_3120");
    svs.dw = m.NewBoolState("loop_conv_ws_dw");
    svs.max_pixels_per_row = m.NewBvState("loop_conv_ws_max_pixels_per_row", 8);
    svs.no_pool = m.NewBoolState("loop_conv_ws_no_pool");
    svs.downsample = m.NewBoolState("loop_conv_ws_downsample");
    svs.input_dilated = m.NewBoolState("loop_conv_ws_input_dilated");
    svs.activation = m.NewBvState("loop_conv_ws_activation", 2);

    // Helpers
    svs.b = m.NewBvState("loop_conv_ws_b", 32);
    svs.dilated_krows = m.NewBvState("loop_conv_ws_dilated_krows", 32);
    svs.dilated_kcols = m.NewBvState("loop_conv_ws_dilated_kcols", 32);
    svs.irows_without_dilation = m.NewBvState("loop_conv_ws_irows_without_dilation", 32);
    svs.icols_without_dilation = m.NewBvState("loop_conv_ws_icols_without_dilation", 32);
    svs.irows_unpadded_without_dilation = m.NewBvState("loop_conv_ws_irows_unpadded_without_dilation", 32);
    svs.icols_unpadded_without_dilation = m.NewBvState("loop_conv_ws_icols_unpadded_without_dilation", 32);
    svs.irows_unpadded = m.NewBvState("loop_conv_ws_irows_unpadded", 32);
    svs.icols_unpadded = m.NewBvState("loop_conv_ws_icols_unpadded", 32);
    svs.irows = m.NewBvState("loop_conv_ws_irows", 32);
    svs.icols = m.NewBvState("loop_conv_ws_icols", 32);
    svs.out_channels_per_bank = m.NewBvState("loop_conv_ws_out_channels_per_bank", 32);
    svs.in_channels_per_bank = m.NewBvState("loop_conv_ws_in_channels_per_bank", 32);
    svs.B_rows = m.NewBvState("loop_conv_ws_B_rows", 32);
    svs.D_sp_addr_row = m.NewBvState("loop_conv_ws_D_sp_addr_row", 32);
    svs.C_sp_addr_row = m.NewBvState("loop_conv_ws_C_sp_addr_row", 32);
    svs.A_sp_addr_start = m.NewBvState("loop_conv_ws_A_sp_addr_start", 32);
    svs.B_sp_addr_start = m.NewBvState("loop_conv_ws_B_sp_addr_start", 32);
    svs.D_sp_addr_start = m.NewBvState("loop_conv_ws_D_sp_addr_start", 32);
    svs.C_sp_addr_start = m.NewBvState("loop_conv_ws_C_sp_addr_start", 32);
    svs.new_weights = m.NewBvState("loop_conv_ws_new_weights", 32);
    svs.max_ochs_per_mvin = m.NewBvState("loop_conv_ws_max_ochs_per_mvin", 32);
    svs.max_chs_per_mvin = m.NewBvState("loop_conv_ws_max_chs_per_mvin", 32);
    svs.spad_stride = m.NewBvState("loop_conv_ws_spad_stride", 32);
    svs.orow = m.NewBvState("loop_conv_ws_orow", 32);
    svs.ocol = m.NewBvState("loop_conv_ws_ocol", 32);
    svs.och = m.NewBvState("loop_conv_ws_och", 32);
    svs.irow = m.NewBvState("loop_conv_ws_irow", 32);
    svs.icol = m.NewBvState("loop_conv_ws_icol", 32);
    svs.ich = m.NewBvState("loop_conv_ws_ich", 32);
    svs.krow = m.NewBvState("loop_conv_ws_krow", 32);
    svs.kcol = m.NewBvState("loop_conv_ws_kcol", 32);
    svs.kch = m.NewBvState("loop_conv_ws_kch", 32);
    svs.child_state = m.NewBvState("loop_conv_ws_child_state", 32);

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
    config2.SetUpdate(svs.pochs, Extract(command.rs2, 15, 0));

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


}

}