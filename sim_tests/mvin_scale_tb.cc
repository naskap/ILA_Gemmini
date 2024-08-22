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

#define N 8

#if (N*DIM) > (BANK_NUM*BANK_ROWS)
#error not enough scratchpad space
#endif


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
  char *argv[] = {const_cast<char*>("./Gemmini_test_mvin_scale")};

  
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      status = 1; return;
    }
#endif

  // printf("Flush\n");
  gemmini_flush(0);

#ifdef HAS_MVIN_SCALE
  elem_t In[N][DIM][DIM] row_align(1);
  elem_t Out[N][DIM][DIM] row_align(1);

  for (size_t n = 0; n < N; ++n)
    for (size_t i = 0; i < DIM; ++i)
      for (size_t j = 0; j < DIM; ++j)
        In[n][i][j] = i*DIM + j + n;

  gemmini_config_st(DIM * sizeof(elem_t));

  for (int n = 0; n < N; ++n) {
    gemmini_extended_config_ld(DIM * sizeof(elem_t), n);
    gemmini_mvin(In[n], n*DIM);
    gemmini_mvout(Out[n], n*DIM);
  }

  gemmini_fence();

  for (int n = 0; n < N; ++n) {
    bool is_correct = true;

    for (size_t i = 0; i < DIM; ++i)
      for (size_t j = 0; j < DIM; ++j) {
         if (Out[n][i][j] != (elem_t)(MVIN_SCALE(In[n][i][j], n))) {
           is_correct = false;
           break;
         }
      }

    if (!is_correct) {
      printf("Matrix %u:\n", n);
      printMatrix(In[n]);
      printf("Matrix %u output:\n", n);
      printMatrix(Out[n]);
      printf("\n");
      printf("Scale: %d", n);

      status = 1; return;
    }
  }
#endif

#ifdef HAS_MVIN_ACC_SCALE
  acc_t In_acc[N][DIM][DIM] row_align_acc(1);
  elem_t Out_acc[N][DIM][DIM] row_align(1);

  const uint64_t acc_addr = (uint64_t)(1) << (ADDR_LEN - 1);

  for (size_t n = 0; n < N; ++n)
    for (size_t i = 0; i < DIM; ++i)
      for (size_t j = 0; j < DIM; ++j)
        In_acc[n][i][j] = i*DIM + j + n;

  gemmini_config_st(DIM * sizeof(elem_t));

  for (int n = 0; n < N; ++n) {
    gemmini_extended_config_ld(DIM * sizeof(acc_t), (n+1));
    gemmini_mvin(In_acc[n], acc_addr | (n*DIM));
    gemmini_mvout(Out_acc[n], acc_addr | (n*DIM));
  }

  gemmini_fence();

  for (int n = 0; n < N; ++n) {
    bool is_correct = true;

    for (size_t i = 0; i < DIM; ++i)
      for (size_t j = 0; j < DIM; ++j) {
         // acc_t gold = (n+1) * In_acc[n][i][j];
         acc_t gold = ACC_SCALE(In_acc[n][i][j], n+1);
         if (gold > elem_t_max) {
             gold = elem_t_max;
         } else if (gold < elem_t_min) {
             gold = elem_t_min;
         }

         if (Out_acc[n][i][j] != gold) {
           is_correct = false;
           break;
         }
      }

    if (!is_correct) {
      printf("Accumulator matrix %u:\n", n);
      printMatrixAcc(In_acc[n]);
      printf("Accumulator matrix %u output:\n", n);
      printMatrix(Out_acc[n]);
      printf("\n");

      status = 1; return;
    }
  }
#endif

  status = 0; return;
}


  
};

int sc_main(int argc, char* argv[]) {
  assert(__BYTE_ORDER == __LITTLE_ENDIAN);
  Testbench h("h");
  sc_start(10000000000.0,SC_SEC);
  return h.status.read(); 
}

    