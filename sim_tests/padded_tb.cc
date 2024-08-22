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

void print_matrix(size_t rows, size_t cols, elem_t **mat) {
    elem_t (*mat_casted)[cols] = reinterpret_cast<elem_t (*)[cols]>(mat);
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++)
#ifndef ELEM_T_IS_FLOAT
            printf("%d ", mat_casted[r][c]);
#else
            printf("%x ", elem_t_to_elem_t_bits(mat[r][c]));
#endif
        printf("\n");
    }
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
  char *argv[] = {const_cast<char*>("./Gemmini_test_padded")};

  
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      status = 1; return;
    }
#endif

    // Test padded mvins
    {
        const size_t rows = rand() % (DIM-1) + 1;
        const size_t cols = rand() % (DIM-1) + 1;
        elem_t input[rows][cols];
        elem_t output[DIM][DIM];

        for (size_t r = 0; r < rows; r++)
            for (size_t c = 0; c < cols; c++)
#ifndef ELEM_T_IS_FLOAT
                input[r][c] = rand() % elem_t_max;
#else
                input[r][c] = rand_double();
#endif

        const size_t sp_addr = 0;

        gemmini_config_ld(cols * sizeof(elem_t));
        gemmini_config_st(DIM * sizeof(elem_t));

        gemmini_extended_mvin(input, sp_addr, cols, rows);
        gemmini_mvout(output, sp_addr);
        gemmini_fence();

        for (size_t r = 0; r < rows; r++)
            for (size_t c = 0; c < cols; c++)
                if (input[r][c] != output[r][c]) {
                    printf("Matrices don't match!\n");

                    printf("input:\n");
                    print_matrix(rows, cols, (elem_t **) input);

                    printf("output:\n");
                    printMatrix(output);

                    status = 1; return;
                }
    }

    // Test padded mvins and padded mvouts
    {
        const size_t rows = rand() % (DIM-1) + 1;
        const size_t cols = rand() % (DIM-1) + 1;
        elem_t input[rows][cols];
        elem_t output[rows][cols];

        for (size_t r = 0; r < rows; r++)
            for (size_t c = 0; c < cols; c++)
#ifndef ELEM_T_IS_FLOAT
                input[r][c] = rand() % elem_t_max;
#else
                input[r][c] = rand_double();
#endif

        const size_t sp_addr = 0;

        gemmini_config_ld(cols * sizeof(elem_t));
        gemmini_config_st(cols * sizeof(elem_t));

        gemmini_extended_mvin(input, sp_addr, cols, rows);
        gemmini_extended_mvout(output, sp_addr, cols, rows);
        gemmini_fence();

        for (size_t r = 0; r < rows; r++)
            for (size_t c = 0; c < cols; c++)
                if (input[r][c] != output[r][c]) {
                    printf("Matrices don't match!\n");

                    printf("input:\n");
                    print_matrix(rows, cols, (elem_t **) input);

                    printf("output:\n");
                    print_matrix(rows, cols, (elem_t **) output);

                    status = 1; return;
                }
    }

    // Test padded matmuls
    for (int dataflow = 0; dataflow <= 1; dataflow++) {
        const size_t I = rand() % (DIM-1) + 1;
        const size_t J = rand() % (DIM-1) + 1;
        const size_t K = rand() % (DIM-1) + 1;
        elem_t A[I][K];
        elem_t B[K][J];
        elem_t D[I][J];
        elem_t C[I][J];
        elem_t gold[I][J];

        for (size_t i = 0; i < I; i++)
            for (size_t k = 0; k < K; k++)
                A[i][k] = rand() % 5;

        for (size_t k = 0; k < K; k++)
            for (size_t j = 0; j < J; j++)
                B[k][j] = rand() % 5;

        for (size_t i = 0; i < I; i++)
            for (size_t j = 0; j < J; j++)
                D[i][j] = rand() % 5;

        for (size_t i = 0; i < I; i++)
            for (size_t j = 0; j < J; j++) {
                acc_t result = D[i][j];
                for (size_t k = 0; k < K; k++)
                    result += A[i][k] * B[k][j];

                gold[i][j] = result < elem_t_min ? elem_t_min : (result > elem_t_max ? elem_t_max : result);
            }

        const size_t A_sp_addr = 0;
        const size_t B_sp_addr = DIM;
        const size_t D_sp_addr = 2*DIM;
        const size_t C_sp_addr = 3*DIM;

        gemmini_config_ex(dataflow, NO_ACTIVATION, 0);
        gemmini_config_st(J * sizeof(elem_t));

        gemmini_config_ld(K * sizeof(elem_t));
        gemmini_extended_mvin(A, A_sp_addr, K, I);

        gemmini_config_ld(J * sizeof(elem_t));
        gemmini_extended_mvin(B, B_sp_addr, J, K);

        gemmini_config_ld(J * sizeof(elem_t));
        gemmini_extended_mvin(D, D_sp_addr, J, I);

        if (dataflow == OUTPUT_STATIONARY) {
            gemmini_extended_preload(D_sp_addr, C_sp_addr, J, I, J, I);
            gemmini_extended_compute_preloaded(A_sp_addr, B_sp_addr, K, I, J, K);
        } else {
            gemmini_extended_preload(B_sp_addr, C_sp_addr, J, K, J, I);
            gemmini_extended_compute_preloaded(A_sp_addr, D_sp_addr, K, I, J, I);
        }

        gemmini_extended_mvout(C, C_sp_addr, J, I);

        gemmini_fence();

        for (size_t r = 0; r < I; r++)
            for (size_t c = 0; c < J; c++)
                if (C[r][c] != gold[r][c]) {
                    printf("Matrices don't match! (dataflow == %d)\n", dataflow);

                    printf("C:\n");
                    print_matrix(I, J, (elem_t **) C);

                    printf("gold:\n");
                    print_matrix(I, J, (elem_t **) gold);

                    status = 1; return;
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

    