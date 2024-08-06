#include <Gemmini.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "include/gemmini_testutils.h"

#define PG_SIZE (4*1024)
#define OFFSET 1

#define FUNCT_CONFIG 0
#define FUNCT_LOAD1 2

#define CONFIG_TYPE_MVIN 0b01
#define ACC_MVIN_ACCTYPE 0
#define ACC_MVIN_INPUTTYPE 1
#define CONFIG_MVIN1 0
#define CONFIG_MVIN2 1
#define CONFIG_MVIN3 2
#define OPCODE_CUSTOM3 0b1111011

#define ARRAY_DIM 16

struct offset_buffer {
  elem_t garbage[OFFSET];
  elem_t data[DIM][DIM];
} __attribute__((__packed__));

using namespace sc_core;

SC_MODULE(Host){
  sc_clock clk;
  sc_signal<sc_biguint<7>>  funct_write;
  sc_signal<sc_biguint<64>> rs1;
  sc_signal<sc_biguint<64>> rs2;
  sc_signal<sc_biguint<7>>  opcode;

  SC_CTOR(Host) : clk("clk",1,SC_SEC) {
    SC_THREAD(host_thread)
  }
  void host_thread(){
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

  std::cout << In.data << "\n";

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

    exit(1);
  }

  exit(0);


  }


};
int sc_main(int argc, char* argv[]) {
  Gemmini g("g");
  Host h("h");

  g.Gemmini_instr_funct_in(h.funct_write);
  g.Gemmini_instr_rs1_in(h.rs1);
  g.Gemmini_instr_rs2_in(h.rs2);
  g.Gemmini_instr_opcode_in(h.opcode);

  g.instr_log.open("./instr_log.txt",std::ofstream::out);
  sc_start(100,SC_SEC);
  return 0; 
}
