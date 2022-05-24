#include "../lib/constants.h"

const char * CALNAME[4] = {
        "PCAL", "ECIN", "ECOU", "ALL"
};
const char * SFARR2D[4] = {
        "Vp vs E/Vp (PCAL sector ",
        "Vp vs E/Vp (ECIN sector ",
        "Vp vs E/Vp (ECOU sector ",
        "Vp vs E/Vp (CALs sector "
};
const char * SFARR1D[4] = {
        "PCAL Sampling Fraction sector ",
        "ECIN Sampling Fraction sector ",
        "ECOU Sampling Fraction sector ",
        "CALs Sampling Fraction sector "
};
const double PLIMITSARR[4][2] = {
        {0.060, 0.250},
        {0.015, 0.120},
        {0.000, 0.400},
        {0.150, 0.300}
};

const char * METADATA_LIST[METADATA_LIST_SIZE] = {
        R_RUNNO, R_EVENTNO, R_BEAME
};
const char * PARTICLE_LIST[PARTICLE_LIST_SIZE] = {
        R_PID, R_CHARGE, R_MASS, R_VX, R_VY, R_VZ, R_PX, R_PY, R_PZ, R_P, R_THETA, R_PHI, R_BETA
};
const char * CALORIMETER_LIST[CALORIMETER_LIST_SIZE] = {
        R_PCAL_E, R_ECIN_E, R_ECOU_E, R_TOT_E
};
const char * SCINTILLATOR_LIST[SCINTILLATOR_LIST_SIZE] = {
        R_DTOF
};
const char * SIDIS_LIST[SIDIS_LIST_SIZE] = {
        R_Q2, R_NU, R_XB, R_W2
};
