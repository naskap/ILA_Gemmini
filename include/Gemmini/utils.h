
#ifndef GEMMINI_UTILS_H
#define GEMMINI_UTILS_H
#include <ilang/ilang++.h>
#include <string.h>
#include <Gemmini/statevars.h>

namespace ilang {

namespace Gemmini {
extern ExprRef GetBvSlice(ExprRef const &src, ExprRef const &idx_hi, int length);
extern ExprRef GetBvElmt(ExprRef const &src_bv, ExprRef const &idx, int elmt_size);
extern ExprRef GetMemElmt(ExprRef const &src_mem, ExprRef const &row, ExprRef const &col, int elmt_size);
extern ExprRef SetBvSlice(ExprRef const &dest_bv, ExprRef const &start_index_high, ExprRef const &src_bv);
extern ExprRef SetBvElmt(ExprRef const &dest_bv, ExprRef const &idx, ExprRef const &src_bv);
extern ExprRef SetMemElmt(ExprRef const &dest_mem, ExprRef const &row, ExprRef const &col, ExprRef const &src_bv);
extern ExprRef LoadMulti(ExprRef memory, ExprRef addr, int addresses);
extern ExprRef StoreMulti(ExprRef &memory, ExprRef &to_store, ExprRef &start_addr);
extern ExprRef WrappingAdd(ExprRef &num1, ExprRef &num2, ExprRef &max, ExprRef const &reset);
extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_init_values, std::vector<ExprRef> &loop_increments, std::vector<ExprRef> &loop_maximums);
extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_maximums);
extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_increments, std::vector<ExprRef> &loop_maximums);
extern ExprRef Mod(ExprRef const &x, ExprRef const &y);
extern ExprRef Min(ExprRef const &num1, ExprRef const &num2);
extern ExprRef ApplyActivation(ExprRef &src, ExprRef &act, unsigned int out_width);
extern ExprRef CastBv(ExprRef &src, unsigned int out_width);
extern ExprRef ReLUCast(ExprRef &accTypeElmt, unsigned int out_width);
extern void CallMvin(InstrRef &caller, load_statevars_t &load_svs, ExprRef &dram_addr, ExprRef &sp_addr, ExprRef &rows, ExprRef &cols);

template<class To, class From>
To _bit_cast(const From& src) noexcept
{ 
    To dst;
    memcpy(&dst, &src, sizeof(To));
    return dst;
}


}
}

#endif