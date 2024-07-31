#include <iostream>
#include "functions_RC.h"

int main(int argc, char** argv) {
  double ScatteredElectron_Energy = 3.3;  //example
  double ScatteredPolarAngle_deg  = 15.0; //example
  double RC_factor = Get_RC_RATIO("carbon12", ScatteredElectron_Energy, ScatteredPolarAngle_deg); 
  double CC_factor = Get_CC_RATIO("carbon12", ScatteredElectron_Energy, ScatteredPolarAngle_deg);

  std::cout << "The RC factor is " << RC_factor << std::endl;
  std::cout << "The CC factor is " << CC_factor << std::endl;

  return 0;
}
