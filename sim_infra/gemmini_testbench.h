#ifndef LOCAL_TEST_UTILS_H
#define LOCAL_TEST_UTILS_H

// Needed for all tests

#include <Gemmini.h>

static Gemmini g("g");
static sc_signal<sc_biguint<7>>  funct_write;
static sc_signal<sc_biguint<64>> rs1;
static sc_signal<sc_biguint<64>> rs2;
static sc_signal<sc_biguint<7>>  opcode;


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


enum test_status {SUCCESS, FAIL, UNFINISHED};


#include <systemc.h>
#include <include/gemmini_testutils.h>
#include <endian.h>


#endif