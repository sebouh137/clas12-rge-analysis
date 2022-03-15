#ifndef UTILS
#define UTILS

#include <math.h>

double to_deg(double radians) {return radians * (180.0 / M_PI);}
double calc_Q2(double beam_E, double momentum, double theta) {
    return 4 * beam_E * momentum * pow(sin(theta/2), 2);
}
double calc_nu(double beam_E, double momentum) {
    return beam_E - momentum;
}
double calc_Xb(double beam_E, double momentum, double theta) {
    return (calc_Q2(beam_E, momentum, theta)/2) / (calc_nu(beam_E, momentum)/PRTMASS);
}

#endif
