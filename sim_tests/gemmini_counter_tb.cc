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
    status = test_status::UNFINISHED;
  }
  void tb_thread(){
  
  int argc = 1;
  char *argv[] = {const_cast<char*>("./Gemmini_test_gemmini_counter")};

  
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      status = 1; return;
    }
#endif

  gemmini_flush(0);

  // Set counter and reset
  counter_configure(0, LOAD_ACTIVE_CYCLE);
  if(counter_read(0) != 0) {
    printf("Counter Reset Failed (not equal to 0)\n");
    status = 1; return;
  }

  // Initial matrix
  gemmini_config_ld(DIM * sizeof(elem_t));
  gemmini_config_st(DIM * sizeof(elem_t));

  static elem_t In[N][DIM][DIM] row_align(1);
  static elem_t Out[N][DIM][DIM] row_align(1);

  for (size_t n = 0; n < N; ++n)
    for (size_t i = 0; i < DIM; ++i)
      for (size_t j = 0; j < DIM; ++j)
        In[n][i][j] = i*DIM + j + n;

  // Move in
  for (size_t i = 0; i < N; i++) {
    gemmini_mvin(In[i], i*DIM);
    gemmini_mvout(Out[i], i*DIM);
  }

  // Check value (should be increasing right now as Gemmini executes in the background)
  int counter_val = counter_read(0);

  // Take a snapshot
  counter_snapshot_take();
  int snapshot_val = counter_read(0);

  // Print first counter value (Syscall takes a lot of time, and we might not capture a snapshot when
  // the instructions are still being executed)
  printf("Read DMA cycles: %d\n", counter_val);
  if (counter_val == 0) {
    printf("Counter Value failed to increase\n");
    status = 1; return;
  } 

  // Wait till the operation finish
  gemmini_fence();

  // Check again
  counter_val = counter_read(0);
  printf("Cycle when taking snapshot: %d, Cycle read after operation finished: %d\n",
    snapshot_val, counter_val);
  if (counter_val != snapshot_val) {
    printf("Snapshot changed after taken; test failed\n");
    status = 1; return;
  }

  // Reset snapshot, and check if cycles changed
  counter_snapshot_reset();
  counter_val = counter_read(0);
  printf("Cycles after snapshot is reset: %d\n", counter_val);
  if (counter_val < snapshot_val + 10) {
    printf("Counter values changed too little after snapshot reset; check if counter continues properly\n");
    status = 1; return;
  }

  // Global reset
  counter_reset();
  counter_val = counter_read(0);
  printf("Cycles after counter reset: %d\n", counter_val);
  if (counter_val != 0) {
    printf("Cycles did not reset after global reset inst\n");
    status = 1; return;
  }

  // Check external counter
  counter_configure(7, RESERVATION_STATION_LD_COUNT);
  for (size_t i = 0; i < N; i++) {
    gemmini_mvin(In[i], i*DIM);
    gemmini_mvout(Out[i], i*DIM);
  }

  // Fused read and take snapshot command
  uint32_t custom_command = (7 & 0x7) << 4 | 0x4;
  gemmini_counter_access(counter_val, custom_command);

  printf("RESERVATION_STATION # of load insts after executing %d mvin and mvout insts: %d\n", N-1, counter_val);
  if (counter_val < 3) {
    printf("The load RESERVATION_STATION counter value is too small\n");
    status = 1; return;
  }

  snapshot_val = counter_read(7);
  if (counter_val != snapshot_val) {
    printf("Snapshot value doesn't match the raw value read before snapshot taken\n");
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

    