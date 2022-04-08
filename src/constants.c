#include "constants.h"

const char *CALNAME[4] = {"PCAL", "ECIN", "ECOU", "ALL"};
const char *SFARR2D[4] = {"Vp vs E/Vp (PCAL sector ",
                          "Vp vs E/Vp (ECIN sector ",
                          "Vp vs E/Vp (ECOU sector ",
                          "Vp vs E/Vp (CALs sector "};
const char *SFARR1D[4] = {"PCAL Sampling Fraction sector ",
                          "ECIN Sampling Fraction sector ",
                          "ECOU Sampling Fraction sector ",
                          "CALs Sampling Fraction sector "};
const double PLIMITSARR[4][2] = {{0.060, 0.250},
                                 {0.015, 0.120},
                                 {0.000, 0.400},
                                 {0.150, 0.300}};
