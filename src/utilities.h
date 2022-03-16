#ifndef UTILS
#define UTILS

#include <math.h>

#include "constants.h"

double to_deg(double radians);
double calc_Q2(double beam_E, double momentum, double theta);
double calc_nu(double beam_E, double momentum);
double calc_Xb(double beam_E, double momentum, double theta);

#endif
