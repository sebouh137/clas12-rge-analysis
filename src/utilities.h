#ifndef UTILS
#define UTILS

#include <math.h>

#include "constants.h"

double to_deg(double radians);
double calc_theta(double px, double py, double pz);
double calc_phi(double px, double py);
double calc_P(double px, double py, double pz);
double calc_Q2(double beam_E, double momentum, double theta);
double calc_nu(double beam_E, double momentum);
double calc_Xb(double beam_E, double momentum, double theta);

#endif
