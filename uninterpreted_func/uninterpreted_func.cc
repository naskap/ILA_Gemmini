#include <Gemmini.h>
#include <systemc.h>
#include <ac_int.h>
#include <ac_std_float.h>


ac_ieee_float32 _bitcast_to_float(sc_biguint<32> to_cast){
  
  // Cast to uint32
  uint32_t to_cast_uint32 = to_cast.to_uint();
  static_assert(sizeof(float) == sizeof(to_cast_uint32), "`float` has a weird size.");
  
  // Memcpy to float
  float to_cast_float;
  std:: memcpy(&to_cast_float, &to_cast_uint32, sizeof(float));

  // Cast to ac_float
  ac_ieee_float32 scale_ac_float(to_cast_float);
  return scale_ac_float;

}


sc_biguint<8> Gemmini::ScaleInputType(sc_biguint<8> input,sc_biguint<32> scale){

  // Bit-cast scale to float
  ac_ieee_float32 scale_ac_float = _bitcast_to_float(scale);

  // Convert input to float
  ac_ieee_float32 input_ac_float(input.to_int());

  // Perform multiply
  auto result_ac_float = input_ac_float * scale_ac_float;

  // Cast to int with rounding to the nearest even number
  auto result_float = result_ac_float.to_float();
  auto result_int   = static_cast<int8_t>(std::nearbyint(result_float));

  // Bit-cast back to sc_biguint8
  uint8_t result_uint;
  std:: memcpy(&result_uint, &result_int, sizeof(uint8_t));
  sc_biguint<8> to_return(result_uint);

  return to_return;
}


sc_biguint<32> Gemmini::ScaleAccType(sc_biguint<32> input,sc_biguint<32> scale){
  
  // bitcast scale to float
  ac_ieee_float32 scale_ac_float = _bitcast_to_float(scale);

  // Cast scale and input to 64 bit floats
  ac_ieee_float64 scale_ac_float64(scale_ac_float);
  ac_ieee_float64 input_float64(input.to_int());

  // Perform multiply
  ac_ieee_float64 result_float64 = scale_ac_float64 * input_float64;

  // Cast to int with rounding to the nearest integer
  auto result_double = result_float64.to_double();
  auto result_int    = static_cast<int32_t>(std::nearbyint(result_double));
  
  // Cast back to sc_biguint
  sc_biguint<32> out(result_int);
  return out;
}

