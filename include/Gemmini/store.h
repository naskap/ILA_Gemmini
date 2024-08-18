#ifndef GEMMINI_LOAD_H
#define GEMMINI_LOAD_H
#include <Gemmini/gemmini_base.h> 

namespace ilang {

namespace Gemmini {




void DefineStoreStateVars(Ila& m, store_statevars_t& store_statevars);
void DefineConfigStoreInstruction(Ila& m, command_t command,
                                  store_statevars_t& store_statevars);
void DefineStoreInstruction(Ila& m, command_t command, gemmini_memory_t memory,
                            store_statevars_t& store_statevars);
void DefineStoreChildInstruction(Ila& child, 
                                command_t command, 
                                gemmini_memory_t memory, 
                                store_statevars_t store_statevars, 
                                bool from_accumulator,
                                bool cast_to_intype,
                                bool maxpool);



} // namespace Gemmini
} // namespace Ilang
#endif