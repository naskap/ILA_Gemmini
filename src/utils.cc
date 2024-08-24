#include <Gemmini/utils.h>
#include <Gemmini/gemmini_base.h>

namespace ilang {

namespace Gemmini {




ExprRef GetBvSlice(ExprRef const &src, ExprRef const &idx_hi, int length){

    // Shift to get rid of low bits
    ExprRef idx_lo = idx_hi - BvConst(length -1,idx_hi.bit_width());
    ExprRef slice  = src  >>  idx_lo.ZExt(src.bit_width());

    // Extract low bits
    return Extract(slice, length - 1, 0);
}

ExprRef GetBvElmt(ExprRef const &src_bv, ExprRef const &idx, int elmt_size){
    ILA_ASSERT(src_bv.bit_width() % elmt_size == 0);
    auto slice_idx_hi = BvConst(src_bv.bit_width(), idx.bit_width()) - idx * elmt_size - 1;
    return GetBvSlice(src_bv, slice_idx_hi, elmt_size);
}

ExprRef GetMemElmt(ExprRef const &src_mem, ExprRef const &row, ExprRef const &col, int elmt_size){
    return GetBvElmt(src_mem.Load(row), col, elmt_size);
}

ExprRef SetBvSlice(ExprRef const &dest_bv, ExprRef const &start_index_high, ExprRef const &src_bv){
    
    ExprRef start_index_low = start_index_high - BvConst(src_bv.bit_width() - 1, start_index_high.bit_width());

    // Create mask
    auto mask = ~BvConst(0, src_bv.bit_width());
         mask = mask.ZExt(dest_bv.bit_width());
         mask = mask << start_index_low.ZExt(dest_bv.bit_width());
         mask = ~mask;
    
    auto to_return = dest_bv & mask;
         to_return = to_return | (src_bv.ZExt(mask.bit_width()) << start_index_low.ZExt(mask.bit_width()));
    return to_return;
    
}

ExprRef SetBvElmt(ExprRef const &dest_bv, ExprRef const &idx, ExprRef const &src_bv){
    ILA_ASSERT(dest_bv.bit_width() % src_bv.bit_width() == 0);
    auto slice_idx_hi = BvConst(dest_bv.bit_width(), idx.bit_width()) - idx * src_bv.bit_width() - 1;
    return SetBvSlice(dest_bv, slice_idx_hi, src_bv);
}
ExprRef SetMemElmt(ExprRef const &dest_mem, ExprRef const &row, ExprRef const &col, ExprRef const &src_bv){
    auto updated_bv = SetBvElmt(dest_mem.Load(row), col, src_bv);
    return dest_mem.Store(row, updated_bv);
}

extern ExprRef WrappingAdd(ExprRef &num1, ExprRef &num2, ExprRef &max, ExprRef const &reset){

    auto unwrapped_result = num1 + num2;
    auto max_extended     = max.ZExt(unwrapped_result.bit_width());
    return Ite(unwrapped_result >= max_extended, reset, unwrapped_result);
}





extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_init_values, std::vector<ExprRef> &loop_increments, std::vector<ExprRef> &loop_maximums){
    ILA_ASSERT(loop_vars.size() == loop_maximums.size());
    ILA_ASSERT(loop_increments.size() == loop_maximums.size());
    ILA_ASSERT(loop_increments.size() == loop_init_values.size());

    ExprRef iterate_next = BoolConst(true);
    for(int idx=loop_vars.size() - 1; idx >= 0; idx--){
        
        auto loop_var = loop_vars.at(idx);
        auto init_value = loop_init_values.at(idx);
        auto increment = loop_increments.at(idx);
        auto max = loop_maximums.at(idx);

        // Update iteration var
        instr.SetUpdate(loop_var, Ite(iterate_next, WrappingAdd(loop_var, increment, max, init_value), loop_var));

        // Iterate the next state variable if current loop is wrapping around
        iterate_next = iterate_next & (loop_var >= max.ZExt(loop_var.bit_width()) - increment);

    }

    return iterate_next;
}



extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_increments, std::vector<ExprRef> &loop_maximums){

    std::vector<ExprRef> loop_init_values;
    for(int i = 0; i < loop_vars.size(); i++){
        loop_init_values.emplace_back(BvConst(0, loop_vars.at(i).bit_width()));
    }

    return IterateLoopVars(instr, loop_vars, loop_init_values, loop_increments, loop_maximums);
}


// Note that all loop vars need to be the same size
extern ExprRef IterateLoopVars(InstrRef &instr, std::vector<ExprRef> &loop_vars, std::vector<ExprRef> &loop_maximums){
    std::vector<ExprRef> loop_increments;
    for(int i = 0; i < loop_vars.size(); i++){
        loop_increments.emplace_back(BvConst(1, loop_vars.at(i).bit_width()));
    }

    return IterateLoopVars(instr, loop_vars, loop_increments, loop_maximums);
}


// Assumes soc_mem is little endian and we want to store our bitvector 
//       with the most significant bits being at the highest addresses
extern ExprRef LoadMulti(ExprRef memory, ExprRef addr, int num_addrs){
    // Load the first byte of data
    auto src_elmt = Load(memory, addr);

    // Load the rest of the bytes 
    for(int i = 1; i < num_addrs; i++){
        src_elmt = Concat(Load(memory, addr+i), src_elmt);
        
    }
    return src_elmt;

}

// Assumes soc_mem is little endian and we want to store our bitvector 
//       with the most significant bits being at the highest addresses
extern ExprRef StoreMulti(ExprRef &memory, ExprRef &to_store, ExprRef &start_addr){
    auto memory_next = memory;
    auto data_width  = memory.data_width();
    int addresses_to_store = (to_store.bit_width() / data_width);
    
    for(int i = 0; i < addresses_to_store; i++){
        memory_next = Store(memory_next, start_addr + BvConst(i, start_addr.bit_width()), 
                            Extract(to_store, data_width*(i+1) - 1, data_width*i));
    }
    return memory_next;
}

ExprRef _clamp(ExprRef &to_clamp, ExprRef lo, ExprRef hi){
    return Ite(Sgt(to_clamp, hi), hi,
           Ite(Slt(to_clamp, lo), lo, to_clamp));
}

// Note: Assumes signed types
extern ExprRef CastBv(ExprRef &src, unsigned int out_width){
    
    // Clamp element to inputtype range
    auto input_type_max = BvConst((1 << (out_width -1))- 1, src.bit_width());
    auto input_type_min = BvConst(- 1 << (out_width -1), src.bit_width());
    auto elmt_clamped   = _clamp(src, input_type_min, input_type_max);
    
    auto elmt_casted = Extract(elmt_clamped, out_width - 1, 0);
    return elmt_casted;

}


extern ExprRef ReLUCast(ExprRef &x, unsigned int out_width){
    
    auto input_type_max = BvConst((1 << (out_width -1)) - 1, x.bit_width());
    auto clamped_elmt   = _clamp(x, BvConst(0, x.bit_width()), input_type_max);
    
    auto elmt_casted = Extract(clamped_elmt, out_width - 1, 0);
    return elmt_casted;
}

extern ExprRef Mod(ExprRef const &x, ExprRef const &y){

    return x - ((x/y)*y);

}

extern ExprRef Min(ExprRef const &num1, ExprRef const &num2){

    return Ite(num1 < num2, num1, num2);

}

ExprRef ApplyActivation(ExprRef &x, ExprRef &act, unsigned int out_width){
    return Ite(act == BvConst(Activation::NONE, 2), CastBv(x, out_width),
           Ite(act == BvConst(Activation::ReLU, 2), ReLUCast(x, out_width), BvConst(-1, out_width)));
}

void CallMvin(InstrRef &caller, load_statevars_t &load_svs, ExprRef &dram_addr, ExprRef &sp_addr, ExprRef &rows, ExprRef &cols){

    caller.SetUpdate(load_svs.soc_mem_base_address, dram_addr);
    caller.SetUpdate(load_svs.spad_base_address, Extract(sp_addr,28,0));
    caller.SetUpdate(load_svs.acc_or_spad, SelectBit(sp_addr, 31) == 1);
    caller.SetUpdate(load_svs.accumulate, SelectBit(sp_addr, 30) == 1);
    caller.SetUpdate(load_svs.num_rows, rows);
    caller.SetUpdate(load_svs.num_cols, cols);
    caller.SetUpdate(load_svs.child_valid, BoolConst(true));

}


}
}