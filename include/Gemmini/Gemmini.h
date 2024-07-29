// File: Gemmini.h

#ifndef GEMMINI_H__
#define GEMMINI_H__

#include <string>
#include <ilang/util/log.h>
#include <malloc.h>
#include <stdio.h>
#include <ilang/ilang++.h>

#define INSTR_FUNCT_WIDTH 7
#define INSTR_RS2_WIDTH 64
#define INSTR_RS1_WIDTH 64
// #define INSTR_XD_WIDTH 1
// #define INSTR_XS1_WIDTH 1
// #define INSTR_XS2_WIDTH 1
// #define INSTR_RD_WIDTH 5
#define INSTR_OPCODE_WIDTH 7

#define INPUT_TYPE_WIDTH_BITS 8 // inputType is int8 by default
#define INPUT_TYPE_WIDTH_BYTES (INPUT_TYPE_WIDTH_BITS/8)
#define ARRAY_DIM 16

#define SPAD_ADDRESS_WIDTH 32
#define SPAD_ROW_WIDTH (INPUT_TYPE_WIDTH_BITS * ARRAY_DIM)
#define SPAD_CAPACITY 262144
#define SPAD_ROWS (SPAD_CAPACITY / SPAD_ROW_WIDTH)

#define ACC_TYPE_WIDTH_BITS 32
#define ACC_TYPE_WIDTH_BYTES (ACC_TYPE_WIDTH_BITS / 8)
#define ACC_ROW_WIDTH (ACC_TYPE_WIDTH_BITS * ARRAY_DIM)



#define SOC_MEM_ADDRESS_WIDTH 64
#define SOC_MEM_ELEMENT_BITS 8

#define CONFIG_CMD 0
#define LOAD2_CMD 1
#define LOAD_CMD 2
#define STORE_CMD 3
#define COMPUTE_AND_FLIP_CMD 4
#define COMPUTE_AND_STAY_CMD 5
#define PRELOAD_CMD 6
#define FLUSH_CMD 7
#define LOOP_WS 8
#define LOOP_WS_CONFIG_BOUNDS 9
#define LOOP_WS_CONFIG_ADDRS_AB 10
#define LOOP_WS_CONFIG_ADDRS_DC 11
#define LOOP_WS_CONFIG_STRIDES_AB 12
#define LOOP_WS_CONFIG_STRIDES_DC 13
#define LOAD3_CMD 14
#define LOOP_CONV_WS 15
#define LOOP_CONV_WS_CONFIG_1 16
#define LOOP_CONV_WS_CONFIG_2 17
#define LOOP_CONV_WS_CONFIG_3 18
#define LOOP_CONV_WS_CONFIG_4 19
#define LOOP_CONV_WS_CONFIG_5 20
#define LOOP_CONV_WS_CONFIG_6 21
#define CLKGATE_EN 22
#define CONFIG_EX 0
#define CONFIG_LOAD 1
#define CONFIG_STORE 2
#define CONFIG_NORM 3
#define CISC_CONFIG 10
#define ADDR_AB 11
#define ADDR_CD 12
#define SIZE_MN 13
#define SIZE_K 14
#define RPT_BIAS 15
#define RESET 16
#define COMPUTE_CISC 17
#define COUNTER_OP 126
// #define GARBAGE_ADDR       "hffffffff"
// #define MVIN_RS2_ADDR_WIDTH  32
// #define MVIN_RS2_COLS_WIDTH  16
// #define MVIN_RS2_ROWS_WIDTH  16
// #define MVOUT_RS2_ADDR_WIDTH  32
// #define MVOUT_RS2_COLS_WIDTH  16
// #define MVOUT_RS2_ROWS_WIDTH  16
// #define CONFIG_MVIN_RS1_UNUSED_WIDTH  2
// #define CONFIG_MVIN_RS1_SHRINK_WIDTH  1
// #define CONFIG_MVIN_RS1_STATE_ID_WIDTH  2
// #define CONFIG_MVIN_RS1_SPACER_WIDTH  8 - 2 - 1 - 2
// #define CONFIG_MVIN_RS1_PIXEL_REPEAT_WIDTH  8
// #define CONFIG_MVIN_RS1_STRIDE_WIDTH  16
// #define CONFIG_MVIN_RS1_SCALE_WIDTH  32
// #define CONFIG_MVOUT_RS1_CMD_TYPE_WIDTH  2
// #define CONFIG_MVOUT_RS1_ACTIVATION_WIDTH  2
// #define CONFIG_MVOUT_RS1_MAX_POOLING_STRIDE_WIDTH  2
// #define CONFIG_MVOUT_RS1_MAX_POOLING_WINDOW_SIZE_WIDTH  2
// #define CONFIG_MVOUT_RS1_UPPER_ZERO_PADDING_WIDTH  2
// #define CONFIG_MVOUT_RS1_LEFT_ZERO_PADDING_WIDTH  2
// #define CONFIG_MVOUT_RS1_SPACER_WIDTH  (24 - 2 * 6)
// #define CONFIG_MVOUT_RS1_POOL_OUT_DIM_WIDTH  8
// #define CONFIG_MVOUT_RS1_POOL_OUT_ROWS_WIDTH  8
// #define CONFIG_MVOUT_RS1_POOL_OUT_COLS_WIDTH  8
// #define CONFIG_MVOUT_RS1_OUT_ROWS_WIDTH  8
// #define CONFIG_MVOUT_RS1_OUT_COLS_WIDTH  8
// #define CONFIG_MVOUT_RS2_ACC_SCALE_WIDTH  32
// #define CONFIG_MVOUT_RS2_STRIDE_WIDTH  32
// #define CONFIG_NORM_RS1_Q_CONST_WIDTH  32
// #define CONFIG_NORM_RS1_SPACER1_WIDTH  13
// #define CONFIG_NORM_RS1_Q_CONST_TYPE_WIDTH  1
// #define CONFIG_NORM_RS1_SET_STATS_ID_ONLY_WIDTH  1
// #define CONFIG_NORM_RS1_ACT_MSB_WIDTH  1
// #define CONFIG_NORM_RS1_NORM_STATS_ID_WIDTH  8
// #define CONFIG_NORM_RS1_SPACER0_WIDTH  6
// #define CONFIG_NORM_RS1_CMD_TYPE_WIDTH  2
// #define CONFIG_NORM_RS2_QC_WIDTH  32
// #define CONFIG_NORM_RS2_QB_WIDTH  32
// #define CONFIG_EX_RS1_CMD_TYPE_WIDTH  2
// #define CONFIG_EX_RS1_DATAFLOW_WIDTH  1
// #define CONFIG_EX_RS1_ACTIVATION_WIDTH  2
// #define CONFIG_EX_RS1_SPACER0_WIDTH  (7 - 2 - 1 - 2)
// #define CONFIG_EX_RS1_SET_ONLY_STRIDES_WIDTH  1
// #define CONFIG_EX_RS1_A_TRANSPOSE_WIDTH  1
// #define CONFIG_EX_RS1_B_TRANSPOSE_WIDTH  1
// #define CONFIG_EX_RS1_SPACER1_WIDTH  (16 - 10)
// #define CONFIG_EX_RS1_A_STRIDE_WIDTH  16
// #define CONFIG_EX_RS1_ACC_SCALE_WIDTH  32
// #define CONFIG_EX_RS2_IN_SHIFT_WIDTH  32
// #define CONFIG_EX_RS2_RELU6_SHIFT_WIDTH  16
// #define CONFIG_EX_RS2_C_STRIDE_WIDTH  16
// #define PRELOAD_RS_ADDR_WIDTH  32
// #define PRELOAD_RS_COLS_WIDTH  16
// #define PRELOAD_RS_ROWS_WIDTH  16
// #define COMPUTED_RS_ADDR_WIDTH  32
// #define COMPUTED_RS_COLS_WIDTH  16
// #define COMPUTED_RS_ROWS_WIDTH  16

namespace ilang {

namespace Gemmini {

struct command_t {
  ExprRef funct  = (ExprRef)NULL;
  ExprRef rs1    = (ExprRef)NULL;
  ExprRef rs2    = (ExprRef)NULL;
  ExprRef opcode = (ExprRef)NULL;
};

struct gemmini_memory_t{
  ExprRef spad        = (ExprRef) NULL;
  ExprRef accumulator = (ExprRef) NULL;
  ExprRef soc_mem     = (ExprRef) NULL;
};


Ila GetGemminiIla(const std::string& model_name = "Gemmini");
extern void DefineLoad(Ila& m, command_t& command, gemmini_memory_t memory);
extern void DefineStore(Ila& m, command_t& command, gemmini_memory_t memory);

extern ExprRef SetSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high);
extern ExprRef AccSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high);
extern ExprRef LoadMulti(ExprRef memory, ExprRef addr, int addresses);
extern ExprRef WrappingAdd(ExprRef &num1, ExprRef &num2, ExprRef &max);

}; // namespace Gemmini

}; // namespace ilang

#endif // GEMMINI_H__
