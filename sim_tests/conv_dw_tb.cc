#include <gemmini_testbench.h>
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

#define BATCH_SIZE 3
#define IN_ROW_DIM 112
#define IN_COL_DIM 112
#define CHANNELS 17
#define KERNEL_DIM 3
#define PADDING 1
#define STRIDE 2

#else

#ifdef FAST

#define IN_ROW_DIM 9
#define IN_COL_DIM 9
#define CHANNELS 5

#else

#define IN_ROW_DIM 17
#define IN_COL_DIM 17
#define CHANNELS 15

#endif

#define BATCH_SIZE 3
#define KERNEL_DIM 3
#define PADDING 1
#define STRIDE 2

#endif

#define NO_BIAS false

#define OUT_ROW_DIM ((IN_ROW_DIM + 2*PADDING - KERNEL_DIM) / STRIDE + 1)
#define OUT_COL_DIM ((IN_COL_DIM + 2*PADDING - KERNEL_DIM) / STRIDE + 1)

bool vec_is_equal(elem_t * a, elem_t * b, int len) {
    for (int i = 0; i < len; i++)
        if (a[i] != b[i])
            return false;
    return true;
}

void init_random(elem_t * buf, int len) {
    elem_t i = 0;
    for (elem_t * ptr = buf; ptr < buf + len; ptr++) {
        // *ptr = (rand() % 32) - 16;
#ifdef FAST
      *ptr = 1;
#else
      *ptr = (rand() % 5) - 2;
#endif
    }
}

void init_random_acc(acc_t * buf, int len) {
    elem_t i = 0;
    for (acc_t * ptr = buf; ptr < buf + len; ptr++) {
        // *ptr = (rand() % 32) - 16;
#ifdef FAST
      *ptr = 1;
#else
      *ptr = (rand() % 5) - 2;
#endif
    }
}

void init_zeros_acc(acc_t * buf, int len) {
    for (acc_t * ptr = buf; ptr < buf + len; ptr++) {
        *ptr = 0;
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
    status = test_status::UNFINISHED;
  }
  void tb_thread(){
  
  int argc = 1;
  char *argv[] = {const_cast<char*>("./Gemmini_test_conv_dw")};

  
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      status = 1; return;
    }
#endif

    gemmini_flush(0);

    // assert((in_dim + 2*padding - kernel_dim) % stride == 0);

    printf("Input dimensions: %u by %u\n", IN_ROW_DIM, IN_COL_DIM);
    printf("Output dimensions: %u by %u\n\n", OUT_ROW_DIM, OUT_COL_DIM);

    static elem_t input[BATCH_SIZE][IN_ROW_DIM][IN_COL_DIM][CHANNELS];
    static elem_t weights[CHANNELS][KERNEL_DIM][KERNEL_DIM];
    static acc_t bias[CHANNELS];
    static elem_t output[BATCH_SIZE][OUT_ROW_DIM][OUT_COL_DIM][CHANNELS];

    printf("Randomize inputs...\n");
    init_random(&input[0][0][0][0], sizeof(input) / sizeof(elem_t));

    printf("Randomize weights...\n");
    init_random(&weights[0][0][0], sizeof(weights) / sizeof(elem_t));

    printf("Randomize bias...\n");
    if (NO_BIAS)
        init_zeros_acc(&bias[0], sizeof(bias) / sizeof(acc_t));
    else
        init_random_acc(&bias[0], sizeof(bias) / sizeof(acc_t));

    printf("CPU conv...\n");
    uint64_t start_cpu = read_cycles();
#ifndef FAST
    tiled_conv_dw_auto(BATCH_SIZE, IN_ROW_DIM, IN_COL_DIM,
            CHANNELS, OUT_ROW_DIM, OUT_COL_DIM,
            STRIDE, PADDING, KERNEL_DIM,

            (elem_t*)input,
            (elem_t*)weights,
            (acc_t*)bias,
            (elem_t*)output,

            NO_ACTIVATION, ACC_SCALE_IDENTITY, 1, 0, 0,

            CPU);
#endif
    uint64_t end_cpu = read_cycles();
    printf("CPU conv took %llu cycles\n", end_cpu - start_cpu);

    static elem_t output_mat[BATCH_SIZE][OUT_ROW_DIM][OUT_COL_DIM][CHANNELS];

    printf("Gemmini conv...\n");
    uint64_t start_gemmini = read_cycles();
    tiled_conv_dw_auto(BATCH_SIZE, IN_ROW_DIM, IN_COL_DIM,
            CHANNELS, OUT_ROW_DIM, OUT_COL_DIM,
            STRIDE, PADDING, KERNEL_DIM,

            (elem_t*)input,
            (elem_t*)weights,
            (acc_t*)bias,
            (elem_t*)output_mat,

            NO_ACTIVATION, ACC_SCALE_IDENTITY, 1, 0, 0,

            WS);
    uint64_t end_gemmini = read_cycles();
    printf("Gemmini conv took %llu cycles\n", end_gemmini - start_gemmini);

    assert(sizeof(output_mat) == sizeof(output));

#ifdef FAST
    bool success = true;
    for (int i = 0; i < sizeof(output_mat) / sizeof(output_mat[0]); i++) {
      elem_t v = *((elem_t*)output_mat+i);
      if (v != 5 && v != 7 && v != 10) {
        success = false;
        break;
      }
    }
#else
    bool success = vec_is_equal(&output[0][0][0][0], &output_mat[0][0][0][0], sizeof(output) / sizeof(elem_t));
#endif

    if (!success) {
        // status = 1; return;

        printf("bias:\n");
        for (int och = 0; och < CHANNELS; och++) {
            printf("%d,", bias[och]);
        }
        printf("\b\n\n");

        printf("weights:\n");
        for (int och = 0; och < CHANNELS; och++) {
            printf("[");
            for (int wrow = 0; wrow < KERNEL_DIM; wrow++) {
                printf("[");
                for (int wcol = 0; wcol < KERNEL_DIM; wcol++) {
                    printf("%d,", weights[och][wrow][wcol]);
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");

        printf("input:\n");
        for (int batch = 0; batch < BATCH_SIZE; batch++) {
            printf("[");
            for (int irow = 0; irow < IN_ROW_DIM; irow++) {
                printf("[");
                for (int icol = 0; icol < IN_COL_DIM; icol++) {
                    printf("[");
                    for (int ich = 0; ich < CHANNELS; ich++) {
                        printf("%d,", input[batch][irow][icol][ich]);
                    }
                    printf("\b],");
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");

        printf("output:\n");
        for (int batch = 0; batch < BATCH_SIZE; batch++) {
            printf("[");
            for (int orow = 0; orow < OUT_ROW_DIM; orow++) {
                printf("[");
                for (int ocol = 0; ocol < OUT_COL_DIM; ocol++) {
                    printf("[");
                    for (int och = 0; och < CHANNELS; och++) {
                        printf("%d,", output[batch][orow][ocol][och]);
                    }
                    printf("\b],");
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");

        printf("output_mat:\n");
        for (int batch = 0; batch < BATCH_SIZE; batch++) {
            printf("[");
            for (int orow = 0; orow < OUT_ROW_DIM; orow++) {
                printf("[");
                for (int ocol = 0; ocol < OUT_COL_DIM; ocol++) {
                    printf("[");
                    for (int och = 0; och < CHANNELS; och++) {
                        printf("%d,", output_mat[batch][orow][ocol][och]);
                    }
                    printf("\b],");
                }
                printf("\b],\n");
            }
            printf("\b],");
        }
        printf("\b\n\n");

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

    