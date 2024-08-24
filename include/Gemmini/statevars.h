#ifndef STATEVARS_H__
#define STATEVARS_H__



#include <ilang/ilang++.h>

namespace ilang {

namespace Gemmini {

#define NUM_MVIN_CONFIG_SETS 3


struct load_statevars_t {
  // Config state vars
  ExprRef src_stride     = (ExprRef)NULL;  // Main memory stride in bytes
  ExprRef dest_stride    = (ExprRef)NULL;  // spad stride in bytes
  ExprRef scale          = (ExprRef)NULL;  // mvin scale (ignored if gemmini not configured with this ability)
  ExprRef read_inputType = (ExprRef)NULL;
  ExprRef pixels_per_row = (ExprRef)NULL;
  ExprRef child_valid    = (ExprRef)NULL;
  
  // Command args (shared across configs)
  ExprRef num_rows             = (ExprRef) NULL;
  ExprRef num_cols             = (ExprRef) NULL;
  ExprRef soc_mem_base_address = (ExprRef) NULL;
  ExprRef acc_or_spad          = (ExprRef) NULL;
  ExprRef accumulate           = (ExprRef) NULL;
  ExprRef spad_base_address    = (ExprRef) NULL;

  // Child ILA helpers (shared across configs)
  ExprRef cur_row   = (ExprRef)NULL;
  ExprRef cur_col   = (ExprRef)NULL;
  ExprRef cur_pixel = (ExprRef)NULL;
};


struct maxpool_params_t {
  ExprRef enable_and_stride = (ExprRef)NULL;
  ExprRef window_size       = (ExprRef)NULL;
  ExprRef upper_pad         = (ExprRef)NULL;
  ExprRef left_pad          = (ExprRef)NULL;
  ExprRef out_dim           = (ExprRef)NULL;
  ExprRef out_rows          = (ExprRef)NULL;
  ExprRef out_cols          = (ExprRef)NULL;
  ExprRef in_rows           = (ExprRef)NULL;
  ExprRef in_cols           = (ExprRef)NULL;
};

struct store_statevars_t {
  ExprRef accScale   = (ExprRef)NULL;
  ExprRef stride     = (ExprRef)NULL;
  ExprRef activation = (ExprRef)NULL;
  maxpool_params_t maxpool_params;

  //Command args
  ExprRef num_rows             = (ExprRef) NULL;
  ExprRef num_cols             = (ExprRef) NULL;
  ExprRef soc_mem_base_address = (ExprRef) NULL;
  ExprRef is_acc_addr          = (ExprRef) NULL;
  ExprRef read_acctype         = (ExprRef) NULL;
  ExprRef src_base_address     = (ExprRef) NULL;

  // Helper variables
  ExprRef child_valid = (ExprRef) NULL;
  ExprRef cur_row     = (ExprRef) NULL;
  ExprRef cur_col     = (ExprRef) NULL;
  ExprRef cur_ch      = (ExprRef) NULL;
  ExprRef cur_wrow    = (ExprRef) NULL;
  ExprRef cur_wcol    = (ExprRef) NULL;
  ExprRef cur_max     = (ExprRef) NULL;
};


struct tile_compute_args_t {
    ExprRef addr = (ExprRef) NULL;
    ExprRef rows = (ExprRef) NULL;
    ExprRef cols = (ExprRef) NULL;
};


struct compute_args_t {
    tile_compute_args_t a;
    tile_compute_args_t bd;
};

struct execute_statevars_t {
    ExprRef dataflow      = (ExprRef) NULL;
    ExprRef act           = (ExprRef) NULL;
    ExprRef sys_shift     = (ExprRef) NULL;
    ExprRef sys_acc_shift = (ExprRef) NULL;
    ExprRef a_transpose   = (ExprRef) NULL;
    ExprRef b_transpose   = (ExprRef) NULL;
    ExprRef c_stride      = (ExprRef) NULL;
    ExprRef a_stride      = (ExprRef) NULL;

    ExprRef preload_sp_addr = (ExprRef) NULL;
    ExprRef output_sp_addr  = (ExprRef) NULL;
    ExprRef preload_cols    = (ExprRef) NULL;
    ExprRef preload_rows    = (ExprRef) NULL;
    ExprRef output_cols     = (ExprRef) NULL;
    ExprRef output_rows     = (ExprRef) NULL;

    ExprRef systolic_array = (ExprRef) NULL;
    ExprRef ws_results     = (ExprRef) NULL;
    ExprRef child_state    = (ExprRef) NULL;

    compute_args_t args;

    ExprRef i = (ExprRef) NULL;
    ExprRef j = (ExprRef) NULL;
    ExprRef k = (ExprRef) NULL;


};



struct loop_ws_statevars_t {

    // Configs
    ExprRef I        = (ExprRef) NULL;
    ExprRef J        = (ExprRef) NULL;
    ExprRef K        = (ExprRef) NULL;
    ExprRef pad_I    = (ExprRef) NULL;
    ExprRef pad_J    = (ExprRef) NULL;
    ExprRef pad_K    = (ExprRef) NULL;
    ExprRef A        = (ExprRef) NULL;
    ExprRef B        = (ExprRef) NULL;
    ExprRef D        = (ExprRef) NULL;
    ExprRef C        = (ExprRef) NULL;
    ExprRef A_stride = (ExprRef) NULL;
    ExprRef B_stride = (ExprRef) NULL;
    ExprRef D_stride = (ExprRef) NULL;
    ExprRef C_stride = (ExprRef) NULL;

    // Command args
    ExprRef ex_accumulate = (ExprRef) NULL;
    ExprRef full_C        = (ExprRef) NULL;
    ExprRef low_D         = (ExprRef) NULL;
    ExprRef act           = (ExprRef) NULL;
    ExprRef a_transpose   = (ExprRef) NULL;
    ExprRef b_transpose   = (ExprRef) NULL;

    // Helpers
    ExprRef i               = (ExprRef) NULL;
    ExprRef j               = (ExprRef) NULL;
    ExprRef k               = (ExprRef) NULL;
    ExprRef A_sp_addr_start = (ExprRef) NULL;
    ExprRef B_sp_addr_start = (ExprRef) NULL;
    ExprRef D_sp_addr_start = (ExprRef) NULL;
    ExprRef C_sp_addr_start = (ExprRef) NULL;
    ExprRef A_sp_addr       = (ExprRef) NULL;
    ExprRef B_sp_addr       = (ExprRef) NULL;
    ExprRef D_sp_addr       = (ExprRef) NULL;
    ExprRef C_sp_addr       = (ExprRef) NULL;
    ExprRef child_state     = (ExprRef) NULL;

};

struct loop_conv_ws_statevars_t {

  // Configs
  ExprRef batch_size      = (ExprRef) NULL;
  ExprRef in_dim          = (ExprRef) NULL;
  ExprRef in_channels     = (ExprRef) NULL;
  ExprRef out_channels    = (ExprRef) NULL;
  ExprRef out_dim         = (ExprRef) NULL;
  ExprRef pool_out_dim    = (ExprRef) NULL;
  ExprRef stride          = (ExprRef) NULL;
  ExprRef padding         = (ExprRef) NULL;
  ExprRef kernel_dim      = (ExprRef) NULL;
  ExprRef pool_size       = (ExprRef) NULL;
  ExprRef pool_stride     = (ExprRef) NULL;
  ExprRef pool_padding    = (ExprRef) NULL;
  ExprRef batches         = (ExprRef) NULL;
  ExprRef porows          = (ExprRef) NULL;
  ExprRef pocols          = (ExprRef) NULL;
  ExprRef ochs           = (ExprRef) NULL;
  ExprRef krows           = (ExprRef) NULL;
  ExprRef kcols           = (ExprRef) NULL;
  ExprRef kchs            = (ExprRef) NULL;
  ExprRef lpad            = (ExprRef) NULL;
  ExprRef rpad            = (ExprRef) NULL;
  ExprRef upad            = (ExprRef) NULL;
  ExprRef dpad            = (ExprRef) NULL;
  ExprRef plpad           = (ExprRef) NULL;
  ExprRef orows           = (ExprRef) NULL;
  ExprRef prad            = (ExprRef) NULL;
  ExprRef pupad           = (ExprRef) NULL;
  ExprRef pdpad           = (ExprRef) NULL;
  ExprRef kernel_dilation = (ExprRef) NULL;
  ExprRef ocols           = (ExprRef) NULL;
  ExprRef weights         = (ExprRef) NULL;
  ExprRef output          = (ExprRef) NULL;
  ExprRef bias            = (ExprRef) NULL;
  ExprRef input           = (ExprRef) NULL;

  // Command args
  ExprRef no_bias            = (ExprRef) NULL;
  ExprRef wrot180            = (ExprRef) NULL;
  ExprRef trans_output_1203  = (ExprRef) NULL;
  ExprRef trans_weight_1203  = (ExprRef) NULL;
  ExprRef trans_weight_0132  = (ExprRef) NULL;
  ExprRef trans_input_3120   = (ExprRef) NULL;
  ExprRef dw                 = (ExprRef) NULL;
  ExprRef max_pixels_per_row = (ExprRef) NULL;
  ExprRef no_pool            = (ExprRef) NULL;
  ExprRef downsample         = (ExprRef) NULL;
  ExprRef input_dilated      = (ExprRef) NULL;
  ExprRef activation         = (ExprRef) NULL;

  // Helpers
  ExprRef b                               = (ExprRef) NULL;
  ExprRef dilated_krows                   = (ExprRef) NULL;
  ExprRef dilated_kcols                   = (ExprRef) NULL;
  ExprRef irows_without_dilation          = (ExprRef) NULL;
  ExprRef icols_without_dilation          = (ExprRef) NULL;
  ExprRef irows_unpadded_without_dilation = (ExprRef) NULL;
  ExprRef icols_unpadded_without_dilation = (ExprRef) NULL;
  ExprRef irows_unpadded                  = (ExprRef) NULL;
  ExprRef icols_unpadded                  = (ExprRef) NULL;
  ExprRef irows                           = (ExprRef) NULL;
  ExprRef icols                           = (ExprRef) NULL;
  ExprRef out_channels_per_bank           = (ExprRef) NULL;
  ExprRef in_channels_per_bank            = (ExprRef) NULL;
  ExprRef B_rows                          = (ExprRef) NULL;
  // ExprRef D_sp_addr_row                   = (ExprRef) NULL;
  // ExprRef C_sp_addr_row                   = (ExprRef) NULL;
  ExprRef A_sp_addr_start                 = (ExprRef) NULL;
  ExprRef B_sp_addr_start                 = (ExprRef) NULL;
  ExprRef D_sp_addr_start                 = (ExprRef) NULL;
  ExprRef C_sp_addr_start                 = (ExprRef) NULL;
  ExprRef new_weights                     = (ExprRef) NULL;
  ExprRef max_ochs_per_mvin               = (ExprRef) NULL;
  ExprRef max_chs_per_mvin                = (ExprRef) NULL;
  ExprRef spad_stride                     = (ExprRef) NULL;
  ExprRef orow                            = (ExprRef) NULL;
  ExprRef ocol                            = (ExprRef) NULL;
  ExprRef och                             = (ExprRef) NULL;
  ExprRef irow                            = (ExprRef) NULL;
  ExprRef icol                            = (ExprRef) NULL;
  ExprRef ich                             = (ExprRef) NULL;
  ExprRef krow                            = (ExprRef) NULL;
  ExprRef kcol                            = (ExprRef) NULL;
  ExprRef kch                             = (ExprRef) NULL;
  ExprRef child_state                     = (ExprRef) NULL;

};

struct gemmini_statevars_t {
    load_statevars_t load[NUM_MVIN_CONFIG_SETS];
    store_statevars_t store;
    execute_statevars_t exec;
    loop_ws_statevars_t loop_ws;
    loop_conv_ws_statevars_t loop_conv;

};

}

}

#endif