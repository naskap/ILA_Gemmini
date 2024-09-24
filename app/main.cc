// File: main.cc

#include <iostream>
#include <cstdlib> 

#include <Gemmini/gemmini_base.h>

using namespace ilang;

int main() {
  auto m = Gemmini::GetGemminiIla("Gemmini");
  std::cout << "Successfully construct " << m << "\n";

  std::string sim_name = "sim_model";
  ExportSysCSim(m,"./" + sim_name);

  std::string setup_cmd = "python ../script/setup_sim_folder.py --name " + sim_name;
  std::system(setup_cmd.c_str());
  return 0;
}
