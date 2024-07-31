#include <Gemmini/Gemmini.h>

namespace ilang {

namespace Gemmini {


extern ExprRef GetSlice(ExprRef &src, ExprRef idx_hi, int length){

    // Shift to get rid of low bits
    ExprRef idx_lo = idx_hi - BvConst(length -1,idx_hi.bit_width());
    ExprRef slice = src  >>  idx_lo.ZExt(src.bit_width());

    // Extract low bits
    return Extract(slice, length - 1, 0);
}

// Could also implement this by 1) masking out slice range 2) bitwise or with slice
extern ExprRef SetSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high){
    
    ExprRef start_index_low = start_index_high - BvConst(src_bv.bit_width() - 1, start_index_high.bit_width());

    // Create mask
    auto mask = ~BvConst(0, src_bv.bit_width());
    mask = mask.ZExt(dest_bv.bit_width());
    mask = mask << start_index_low.ZExt(dest_bv.bit_width());
    
    auto to_return = dest_bv & mask; 
    to_return = to_return | (src_bv.ZExt(mask.bit_width()) << start_index_low.ZExt(mask.bit_width()));
    return to_return;
    
}

extern ExprRef AccSlice(ExprRef &dest_bv, ExprRef src_bv, ExprRef start_index_high){
    
    // Compute slice to store
    auto dest_slice = GetSlice(dest_bv, start_index_high, src_bv.bit_width());
    auto to_store   = dest_slice + src_bv;

    return SetSlice(dest_bv, to_store, start_index_high);
    
}

extern ExprRef WrappingAdd(ExprRef &num1, ExprRef &num2, ExprRef &max){

    auto unwrapped_result = num1 + num2;
    auto max_extended = max.ZExt(unwrapped_result.bit_width());
    return Ite(unwrapped_result >= max_extended, unwrapped_result - max_extended, unwrapped_result);
}

// Note that all loop vars need to be the same size
extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_maximums){
    ILA_ASSERT(loop_vars.size() == loop_maximums.size());

    auto iter_loop_vars = loop_vars.end() - 1;
    auto iter_loop_maxs = loop_maximums.end() - 1;

    ExprRef iterate_next = BoolConst(true);
    while(iter_loop_vars != loop_vars.begin() - 1){
        
        ILA_INFO << "Starting loop iter";
        // Read from iterators 
        auto loop_var = *iter_loop_vars;
        auto max = *iter_loop_maxs;

        ILA_INFO << loop_var;
        ILA_INFO << max;
        

        // Update iteration var
        auto const_one = BvConst(1, (*iter_loop_vars).bit_width());
        instr.SetUpdate(loop_var, Ite(iterate_next, WrappingAdd(loop_var, const_one, max), loop_var));

        // Iterate the next state variable if current loop is wrapping around
        iterate_next = iterate_next & (loop_var == max.ZExt(loop_var.bit_width()) - const_one);

        // Update iterators
        iter_loop_vars -= 1;
        iter_loop_maxs -= 1;

    }

    return iterate_next;
}


extern ExprRef LoadMulti(ExprRef memory, ExprRef addr, int addresses){
        // Load the first byte of data
    auto src_elmt = Load(memory, addr);

    // Load the rest of the bytes
    for(int i = 1; i < addresses; i++){
        src_elmt = Concat(src_elmt, Load(memory, addr+i));
        
    }
    return src_elmt;

}


}
}