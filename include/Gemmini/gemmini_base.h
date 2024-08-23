// File: Gemmini.h

#ifndef GEMMINI_H__
#define GEMMINI_H__

#include <string>
#include <ilang/util/log.h>
#include <malloc.h>
#include <stdio.h>
#include <ilang/ilang++.h>
#include <Gemmini/utils.h>
#include <Gemmini/statevars.h>

#define INSTR_FUNCT_WIDTH 7
#define INSTR_RS2_WIDTH 64
#define INSTR_RS1_WIDTH 64
#define INSTR_OPCODE_WIDTH 7

#define INPUT_TYPE_WIDTH_BITS 8 // inputType is int8 by default
#define INPUT_TYPE_WIDTH_BYTES (INPUT_TYPE_WIDTH_BITS/8)
#define ARRAY_DIM 16

#define SPAD_ADDRESS_WIDTH 32
#define SPAD_ROW_WIDTH (INPUT_TYPE_WIDTH_BITS * ARRAY_DIM)
#define SPAD_CAPACITY 524288
#define SPAD_ROWS (SPAD_CAPACITY / SPAD_ROW_WIDTH)
#define SPAD_BANKS 4

#define ACC_TYPE_WIDTH_BITS 32
#define ACC_TYPE_WIDTH_BYTES (ACC_TYPE_WIDTH_BITS / 8)
#define ACC_ROW_WIDTH (ACC_TYPE_WIDTH_BITS * ARRAY_DIM)

#define OUTPUT_TYPE_WIDTH_BITS 16

#define SOC_MEM_ADDRESS_WIDTH 64
#define SOC_MEM_ELEMENT_BITS 8

// Gemmini CMDs (funct field values)
#define CONFIG_CMD 0
#define LOAD2_CMD 1
#define LOAD_CMD 2
#define STORE_CMD 3
#define COMPUTE_PRELOADED_CMD 4
#define COMPUTE_ACCUMULATE_CMD 5
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

// Config types
#define CONFIG_EX 0
#define CONFIG_LOAD 1
#define CONFIG_STORE 2
#define CONFIG_NORM 3

#define SPAD_ENTRIES 1024 * 256
#define ACC_ENTRIES 1024 * 64




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

enum Activation {NONE, ReLU};

enum compute_child_states {
  COMPUTE_INACTIVE, 
  PRELOAD, 
  INITIALIZE_WS_RESULTS, 
  WS_COMPUTE, 
  OS_COMPUTE, 
  OUTPUT_RESULTS
};


Ila GetGemminiIla(const std::string& model_name = "Gemmini");
static void DefineCommand(Ila &m, command_t *c);
static void DefineMemory(Ila &m, gemmini_memory_t *mem);
extern void DefineLoad(Ila& m, command_t& command, gemmini_memory_t memory, load_statevars_t load_statevars_t[NUM_MVIN_CONFIG_SETS]);
extern void DefineStore(Ila& m, command_t& command, gemmini_memory_t memory, store_statevars_t &store_statevars);
extern void DefineExecute(Ila& m, command_t& command, gemmini_memory_t memory, execute_statevars_t &execute_statevars);
extern void DefineLoopWS(Ila& m, command_t& command, gemmini_memory_t memory, gemmini_statevars_t &svs);
extern void DefineLoopConv(Ila& m, command_t& command, gemmini_memory_t memory, gemmini_statevars_t &svs);
extern void DefineLoopConvWS(Ila& m, command_t& command, gemmini_memory_t memory, gemmini_statevars_t &svs);

// Uninterpreted functions
static auto scale = SortRef::BV(32);
static auto inputtype_sort = SortRef::BV(INPUT_TYPE_WIDTH_BITS);
static auto acctype_sort = SortRef::BV(ACC_TYPE_WIDTH_BITS);
static FuncRef ScaleInputType("ScaleInputType", inputtype_sort, inputtype_sort, scale);
static FuncRef ScaleAccType("ScaleAccType", acctype_sort, acctype_sort, scale);

}; // namespace Gemmini

}; // namespace ilang

#endif // GEMMINI_H__
