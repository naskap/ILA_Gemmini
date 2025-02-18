#include <gemmini_testbench.h>
// See LICENSE for license details.

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef BAREMETAL
#include <sys/mman.h>
#endif
#include "include/gemmini_testutils.h"

#ifndef BAREMETAL
#define MAT_DIM_I 60
#define MAT_DIM_K 40
#define MAT_DIM_J 30
#else
#define MAT_DIM_I 33
#define MAT_DIM_K 28
#define MAT_DIM_J 32
#endif

void full_matmul(elem_t A[MAT_DIM_I][MAT_DIM_K], elem_t B[MAT_DIM_K][MAT_DIM_J], acc_t D[MAT_DIM_I][MAT_DIM_J],
  full_t C_full[MAT_DIM_I][MAT_DIM_J], bool repeating_bias)
{
  for (size_t r = 0; r < MAT_DIM_I; r++)
    for (size_t c = 0; c < MAT_DIM_J; c++) {
      C_full[r][c] = D[repeating_bias ? 0 : r][c];
      for (size_t k = 0; k < MAT_DIM_K; k++)
        C_full[r][c] += A[r][k]*B[k][c];
    }
}

void full_matmul_At(elem_t A[MAT_DIM_K][MAT_DIM_I], elem_t B[MAT_DIM_K][MAT_DIM_J], acc_t D[MAT_DIM_I][MAT_DIM_J],
  full_t C_full[MAT_DIM_I][MAT_DIM_J], bool repeating_bias)
{
  for (size_t r = 0; r < MAT_DIM_I; r++)
    for (size_t c = 0; c < MAT_DIM_J; c++) {
      C_full[r][c] = D[repeating_bias ? 0 : r][c];
      for (size_t k = 0; k < MAT_DIM_K; k++)
        C_full[r][c] += A[k][r]*B[k][c];
    }
}

void full_matmul_Bt(elem_t A[MAT_DIM_I][MAT_DIM_K], elem_t B[MAT_DIM_J][MAT_DIM_K], acc_t D[MAT_DIM_I][MAT_DIM_J],
  full_t C_full[MAT_DIM_I][MAT_DIM_J], bool repeating_bias)
{
  for (size_t r = 0; r < MAT_DIM_I; r++)
    for (size_t c = 0; c < MAT_DIM_J; c++) {
      C_full[r][c] = D[repeating_bias ? 0 : r][c];
      for (size_t k = 0; k < MAT_DIM_K; k++)
        C_full[r][c] += A[r][k]*B[c][k];
    }
}

void full_matmul_At_Bt(elem_t A[MAT_DIM_K][MAT_DIM_I], elem_t B[MAT_DIM_J][MAT_DIM_K], acc_t D[MAT_DIM_I][MAT_DIM_J],
  full_t C_full[MAT_DIM_I][MAT_DIM_J], bool repeating_bias)
{
  for (size_t r = 0; r < MAT_DIM_I; r++)
    for (size_t c = 0; c < MAT_DIM_J; c++) {
      C_full[r][c] = D[repeating_bias ? 0 : r][c];
      for (size_t k = 0; k < MAT_DIM_K; k++)
        C_full[r][c] += A[k][r]*B[c][k];
    }
}

void full_printMatrix(elem_t m[MAT_DIM_I][MAT_DIM_J]) {
  for (size_t i = 0; i < MAT_DIM_I; ++i) {
    for (size_t j = 0; j < MAT_DIM_J; ++j)
      printf("%d ", m[i][j]);
    printf("\n");
  }
}

void full_printMatrix64Bit(full_t m[MAT_DIM_I][MAT_DIM_J]) {
  for (size_t i = 0; i < MAT_DIM_I; ++i) {
    for (size_t j = 0; j < MAT_DIM_J; ++j)
      printf("%lld ", m[i][j]);
    printf("\n");
  }
}

void full_matscale(full_t full[MAT_DIM_I][MAT_DIM_J], elem_t out[MAT_DIM_I][MAT_DIM_J], acc_scale_t scale) {
  for (size_t r = 0; r < MAT_DIM_I; r++)                             
    for (size_t c = 0; c < MAT_DIM_J; c++) {
      // Bitshift and round element
      full_t scaled = ACC_SCALE(full[r][c], scale);

      // Saturate and cast element
#ifndef ELEM_T_IS_FLOAT
      full_t elem = scaled > elem_t_max ? elem_t_max : (scaled < elem_t_min ? elem_t_min : scaled);
      out[r][c] = elem;
#else
      out[r][c] = scaled; // TODO should we also saturate when using floats?
#endif
    }
}

void full_matrelu(elem_t in[MAT_DIM_I][MAT_DIM_J], elem_t out[MAT_DIM_I][MAT_DIM_J]) {
  for (size_t r = 0; r < MAT_DIM_I; r++)
    for (size_t c = 0; c < MAT_DIM_J; c++)
      out[r][c] = in[r][c] > 0 ? in[r][c] : 0;
}

int full_is_equal(elem_t x[MAT_DIM_I][MAT_DIM_J], elem_t y[MAT_DIM_I][MAT_DIM_J]) {
  for (size_t i = 0; i < MAT_DIM_I; ++i)
    for (size_t j = 0; j < MAT_DIM_J; ++j)
      if (x[i][j] != y[i][j])
        return 0;
  return 1;
}


SC_MODULE(Testbench){
  sc_clock clk;
  sc_signal<sc_uint<8>> status;
  SC_CTOR(Testbench) : clk("clk",1,SC_SEC){
    SC_THREAD(tb_thread)
    g.Gemmini_instr_funct_in(funct_write);
    g.Gemmini_instr_rs1_in(rs1);
    g.Gemmini_instr_rs2_in(rs2);
    g.Gemmini_instr_opcode_in(opcode);
    g.instr_log.open("./instr_log.txt",std::ofstream::out);
    g.instr_update_log.open("./instr_update_log", std::ofstream::out);
    status = test_status::UNFINISHED;
  }
  void tb_thread(){
  
  int argc = 1;
  char *argv[] = {const_cast<char*>("./Gemmini_test_tiled_matmul_option")};

  
#ifndef BAREMETAL
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
    perror("mlockall failed");
    status = 1; return;
  }
#endif

  gemmini_flush(0);

#ifdef BAREMETAL
  for (enum tiled_matmul_type_t option = OS; option <= WS; option++) {
    for (int activation = 0; activation <= 1; activation++) {
      for (int scale = 0; scale <= 1; scale += 1) {
#else
  for (int option_int = OS; option_int <= CPU; option_int++) {
    enum tiled_matmul_type_t option = tiled_matmul_type_t(option_int);
    for (int activation = 0; activation <= 2; activation++) {
      for (int scale = 0; scale <= 12; scale += 6) {
#endif
        for (int no_bias = 0; no_bias < 2; no_bias++) {
          for (int repeating_bias = 0; repeating_bias < 2; repeating_bias++) {
            for (int a_transpose = 0; a_transpose < 2; a_transpose++) {
              for (int b_transpose = 0; b_transpose < 2; b_transpose++) {

                if (((option == OS || option == CPU) && (a_transpose || b_transpose)) ||
                  (option == WS && a_transpose && b_transpose)) {
                  continue;
                }

                static elem_t full_A[MAT_DIM_I][MAT_DIM_K] row_align(1);
                static elem_t full_B[MAT_DIM_K][MAT_DIM_J] row_align(1);
                static elem_t full_C[MAT_DIM_I][MAT_DIM_J] row_align(1);
                static acc_t full_D[MAT_DIM_I][MAT_DIM_J] row_align_acc(1);

                static full_t gold_full[MAT_DIM_I][MAT_DIM_J];
                static elem_t gold[MAT_DIM_I][MAT_DIM_J];

                // printf("Init A\n");
                for (size_t i = 0; i < MAT_DIM_I; ++i) {
                  for (size_t j = 0; j < MAT_DIM_K; ++j) {
                    full_A[i][j] = (rand() % 3) - 1;
                  }
                }

                // printf("Init B\n");
                for (size_t i = 0; i < MAT_DIM_K; ++i) {
                  for (size_t j = 0; j < MAT_DIM_J; ++j) {
                    full_B[i][j] = (rand() % 3) - 1;
                  }
                }

                // printf("Init D\n");
                for (size_t i = 0; i < (repeating_bias ? 1 : MAT_DIM_I); ++i) {
                  for (size_t j = 0; j < MAT_DIM_J; ++j) {
                    full_D[i][j] = no_bias ? 0 : ((rand() % 3) - 1);
                  }
                }

#pragma G++ diagnostic push
#pragma G++ diagnostic ignored "-Wincompatible-pointer-types"
                printf("Starting CPU matmul\n");
                if (!a_transpose && !b_transpose) {
                  full_matmul(full_A, full_B, full_D, gold_full, repeating_bias);
                } else if (a_transpose && !b_transpose) {
                  full_matmul_At(reinterpret_cast<elem_t (*)[MAT_DIM_I]>(full_A), full_B, full_D, gold_full, repeating_bias);
                } else if (!a_transpose && b_transpose) {
                  full_matmul_Bt(full_A, reinterpret_cast<elem_t (*)[MAT_DIM_K]>(full_B), full_D, gold_full, repeating_bias);
                } else if (a_transpose && b_transpose) {
                  full_matmul_At_Bt(reinterpret_cast<elem_t (*)[MAT_DIM_I]>(full_A), reinterpret_cast<elem_t (*)[MAT_DIM_K]>(full_B), full_D, gold_full, repeating_bias);
                }
                full_matscale(gold_full, gold, scale);
#pragma G++ diagnostic pop

                if (activation == RELU) {
                  full_matrelu(gold, gold);
                }

                size_t stride_A = a_transpose ? MAT_DIM_I : MAT_DIM_K;
                size_t stride_B = b_transpose ? MAT_DIM_K : MAT_DIM_J;

                printf("Starting gemmini matmul\n");
                tiled_matmul_auto(MAT_DIM_I, MAT_DIM_J, MAT_DIM_K,
                        (elem_t*)full_A, (elem_t*)full_B, no_bias ? NULL : &full_D[0][0], (elem_t*)full_C,
                        stride_A, stride_B, MAT_DIM_J, MAT_DIM_J,
                        MVIN_SCALE_IDENTITY, MVIN_SCALE_IDENTITY, MVIN_SCALE_IDENTITY,
                        activation, scale, 0, repeating_bias,
                        a_transpose, b_transpose,
                        false, false,
                        0,
                        option);

                if (!full_is_equal(full_C, gold)) {
                  printf("\nINCORRECT!\n");
                  printf("option: %d\n", option);
                  printf("activation: %d\n", activation);
                  printf("scale: %d\n", scale);
                  printf("no_bias: %d\n", no_bias);
                  printf("repeating_bias: %d\n", repeating_bias);
                  printf("a_transpose: %d\n", a_transpose);
                  printf("b_transpose: %d\n", b_transpose);

                  printf("C:\n");
                  full_printMatrix(full_C);
                  printf("Gold:\n");
                  full_printMatrix(gold);
                  printf("Gold full:\n");
                  full_printMatrix64Bit(gold_full);
                  printf("\n");

                  status = 1; return;
                }
              }
            }
          }
        }
      }
    }
  }

  status = 0; return;
}


  
};

int sc_main(int argc, char* argv[]) {
  assert(__BYTE_ORDER == __LITTLE_ENDIAN);
  Testbench h("h");
  sc_start(10000000000.0,SC_SEC);
  return h.status.read(); 
}

    