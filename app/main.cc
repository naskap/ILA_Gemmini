// File: main.cc

#include <iostream>

#include <Gemmini/Gemmini.h>

using namespace ilang;

int main() {
  auto m = Gemmini::GetGemminiIla("Gemmini");
  std::cout << "Successfully construct " << m << "\n";
  ExportSysCSim(m,"./sim_model2");
  return 0;
}
