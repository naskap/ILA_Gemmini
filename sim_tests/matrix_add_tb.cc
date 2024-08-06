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
#include <time.h>
#include "include/gemmini_testutils.h"


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
    status = test_status::UNFINISHED;
  }
  void tb_thread(){
  
  int argc = 1;
  char *argv[] = {const_cast<char*>("./Gemmini_test_matrix_add")};

  
  elem_t A[DIM][DIM];
  elem_t B[DIM][DIM];
  elem_t C[DIM][DIM];
  elem_t gold[DIM][DIM];

  for (size_t i = 0; i < DIM; i++)
    for (size_t j = 0; j < DIM; j++) {
      A[i][j] = (rand() % 16) - 8;
      B[i][j] = (rand() % 16) - 8;
    }

  for (int ascale = -2; ascale < 2; ascale++) {
    for (int bscale = -2; bscale < 2; bscale++) {
      for (size_t i = 0; i < DIM; i++)
        for (size_t j = 0; j < DIM; j++) {
          acc_t sum = MVIN_SCALE(A[i][j], ascale) + MVIN_SCALE(B[i][j], bscale);
          gold[i][j] = sum > elem_t_max ? elem_t_max :
            (sum < elem_t_min ? elem_t_min : sum);
        }

      uint32_t A_acc_addr = 1 << (ADDR_LEN - 1);
      uint32_t B_acc_addr = (1 << (ADDR_LEN - 1)) | (1 << (ADDR_LEN - 2));
      uint32_t C_acc_addr = 1 << (ADDR_LEN - 1);

      gemmini_extended2_config_ld(DIM * sizeof(elem_t), ascale, true);
      gemmini_mvin(A, A_acc_addr);

      gemmini_extended2_config_ld(DIM * sizeof(elem_t), bscale, true);
      gemmini_mvin(B, B_acc_addr);

      gemmini_config_ex(0, NO_ACTIVATION, 0);
      gemmini_config_st(DIM * sizeof(elem_t));
      gemmini_mvout(C, C_acc_addr);

      gemmini_fence();

      if (!is_equal(C, gold)) {
        printf("Wrong (ascale: %d, bscale: %d)\n", ascale, bscale);
        printf("\"C\" matrix:\n");
        printMatrix(C);
        printf("\n");
        printf("\"Gold\" matrix:\n");
        printMatrix(gold);
        printf("\n");
        printf("\"A\" matrix:\n");
        printMatrix(A);
        printf("\n");
        printf("\"B\" matrix:\n");
        printMatrix(B);
        printf("\n");
        printf("Wrong (ascale: %d, bscale: %d)\n", ascale, bscale);
        status = 1; return;
      }
    }
  }

  status = 0; return;
}


  
};

int sc_main(int argc, char* argv[]) {
  Testbench h("h");
  sc_start(10000,SC_SEC);
  return h.status.read(); 
}

    