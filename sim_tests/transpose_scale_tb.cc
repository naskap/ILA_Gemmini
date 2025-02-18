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

#define KDIM 6


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
  char *argv[] = {const_cast<char*>("./Gemmini_test_transpose_scale")};

  
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      status = 1; return;
    }
#endif

  printf("Flush Gemmini TLB of stale virtual addresses\n");
  gemmini_flush(0);

  printf("Initialize our input and output matrices in main memory\n");
  elem_t InA[KDIM][DIM];
  elem_t InB[KDIM][DIM];
  elem_t Out[DIM][DIM];
  elem_t OutGold[DIM][DIM];

  // elem_t A_scale_factor = (rand() % 2) + 1;
  const elem_t A_scale_factor = 0;

  for (size_t i = 0; i < KDIM; i++)
    for (size_t j = 0; j < DIM; j++) {
      InA[i][j] = rand() % 5;
      InB[i][j] = rand() % 5;
    }
  
  for (size_t i = 0; i < DIM; i++) {
    for (size_t j = 0; j < DIM; j++) {
      OutGold[i][j] = 0; 
      for (size_t k = 0; k < KDIM; k++) {
        // OutGold[i][j] += A_scale_factor*InA[k][i]*InB[k][j];
        OutGold[i][j] += InA[k][i]*InB[k][j];
    }}}

  printf("Calculate the scratchpad addresses of all our matrices\n");
  printf("  Note: The scratchpad is \"row-addressed\", where each address contains one matrix row\n");
  size_t InA_sp_addr = 0;
  size_t Out_sp_addr = 2*KDIM;
  size_t InB_sp_addr = 4*KDIM;

  for (size_t K0 = 0; K0 < KDIM; K0+=DIM) {

    printf("Move \"InA\" matrix from main memory into Gemmini's scratchpad\n");
    gemmini_extended_config_ld(DIM * sizeof(elem_t), A_scale_factor);
    //gemmini_config_ld(DIM * sizeof(elem_t));
    gemmini_mvin(InA+K0, InA_sp_addr+K0);
  
    printf("Move \"InB\" matrix from main memory into Gemmini's scratchpad\n");
    gemmini_config_ld(DIM * sizeof(elem_t));
    gemmini_mvin(InB+K0, InB_sp_addr+K0);
  
    printf("Multiply \"InA\" transposed matrix with \"InB\" matrix with a bias of 0\n");
    gemmini_extended_config_ex(OUTPUT_STATIONARY, 0, 0, 0, 0, 1, true, false)
    
    gemmini_preload_zeros(K0 + DIM >= KDIM ? Out_sp_addr : GARBAGE_ADDR);
    if (K0 == 0) { // First iteration
        gemmini_extended_compute_preloaded(InA_sp_addr+K0, InB_sp_addr+K0, (K0 + DIM <= KDIM) ? DIM : KDIM - K0, DIM,
                                                                                   DIM, (K0 + DIM <= KDIM) ? DIM : KDIM - K0);
    } else { // All other iterations
        gemmini_extended_compute_accumulated(InA_sp_addr+K0, InB_sp_addr+K0, (K0 + DIM <= KDIM) ? DIM : KDIM - K0, DIM,
                                                                                 DIM, (K0 + DIM <= KDIM) ? DIM : KDIM - K0);
    }

  }

  printf("Move \"Out\" matrix from Gemmini's scratchpad into main memory\n");
  gemmini_mvout(Out, Out_sp_addr);

  printf("Fence till Gemmini completes all memory operations\n");
  gemmini_fence();

  printf("Check whether \"Out\" and \"Gold\" matrices are identical\n");

  if (!is_equal(Out, OutGold)) {
    printf("Ouput and Gold matrices are different!\n");
    printf("\"Out\" matrix:\n");
    printMatrix(Out);
    printf("\"OutGold\" matrix:\n");
    printMatrix(OutGold);
    printf("\n");

    status = 1; return;
  }

  printf("Output and Gold matrices are identical, as expected\n");
  status = 0; return;
}


  
};

int sc_main(int argc, char* argv[]) {
  assert(__BYTE_ORDER == __LITTLE_ENDIAN);
  Testbench h("h");
  sc_start(10000000000.0,SC_SEC);
  return h.status.read(); 
}

    