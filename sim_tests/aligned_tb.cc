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

#define PG_SIZE (4*1024)
#define OFFSET 1

/*struct aligned_buffer {
  char garbage[0];
  elem_t data[DIM][DIM];
} __attribute__((__packed__));*/

struct offset_buffer {
  elem_t garbage[OFFSET];
  elem_t data[DIM][DIM];
} __attribute__((__packed__));


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
  char *argv[] = {const_cast<char*>("./Gemmini_test_aligned")};

  
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      status = 1; return;
    }
#endif

  gemmini_flush(0);

  // static struct aligned_buffer In __attribute__((aligned(PG_SIZE)));
  static struct offset_buffer In __attribute__((aligned(PG_SIZE)));
  static struct offset_buffer Out __attribute__((aligned(PG_SIZE)));

  for (size_t i = 0; i < OFFSET; ++i) {
      In.garbage[i] = ~0;
      Out.garbage[i] = 1;
  }

  for (size_t i = 0; i < DIM; ++i)
    for (size_t j = 0; j < DIM; ++j) {
      In.data[i][j] = i*DIM + j;
      Out.data[i][j] = 1;
    }

  gemmini_config_ld(DIM * sizeof(elem_t));
  gemmini_config_st(DIM * sizeof(elem_t));

  // printf("Mvin\n");
  gemmini_mvin(In.data, 0);
  // printf("Mvout\n");
  gemmini_mvout(Out.data, 0);

  // printf("Fence\n");
  gemmini_fence();

  if (!is_equal(In.data, Out.data)) {
    printf("Matrix:\n");
    printMatrix(In.data);
    printf("Matrix output:\n");
    printMatrix(Out.data);
    printf("\n");

    status = 1; return;
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

    