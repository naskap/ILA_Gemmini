// File: main.cc

#include <iostream>

#include <Gemmini/Gemmini.h>

using namespace ilang;

int main() {
  auto m = Gemmini::GetGemminiIla("Gemmini");
  std::cout << "Successfully construct " << m << "\n";
  ExportSysCSim(m,"./sim_model");
  return 0;
}
