#include "../lib/constants.h"

// Sampling fraction constant arrays.
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

// Plotting constant arrays.
const char * PLOT_LIST[PLOT_LIST_SIZE] = {
        R_PLOT1D, R_PLOT2D
};
const char * DIM_LIST[DIM_LIST_SIZE] = {
        S_DIM1, S_DIM2
};
const char * RAN_LIST[2] = {
        S_LOWER, S_UPPER
};
const char * R_VAR_LIST[VAR_LIST_SIZE] = {
        R_RUNNO, R_EVENTNO, R_BEAME,
        R_PID, R_CHARGE, R_MASS, R_VX, R_VY, R_VZ, R_PX, R_PY, R_PZ, R_P, R_THETA, R_PHI, R_BETA,
        R_PCAL_E, R_ECIN_E, R_ECOU_E, R_TOT_E,
        R_DTOF,
        R_Q2, R_NU, R_XB, R_W2
};
const char * S_VAR_LIST[VAR_LIST_SIZE] = {
        S_RUNNO, S_EVENTNO, S_BEAME,
        S_PID, S_CHARGE, S_MASS, S_VX, S_VY, S_VZ, S_PX, S_PY, S_PZ, S_P, S_THETA, S_PHI, S_BETA,
        S_PCAL_E, S_ECIN_E, S_ECOU_E, S_TOT_E,
        S_DTOF,
        S_Q2, S_NU, S_XB, S_W2
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
const char * CUTS_LIST[CUTS_LIST_SIZE] = {
        R_RUNNO, R_EVENTNO, R_PID, R_STATUS, R_CHI2, R_NDF, R_VX, R_VY, R_VZ, R_Q2, R_W2
};

// Standard plots constant arrays.
const int STD_PX[STDPLT_LIST_SIZE] = {
        1, /* p vs beta */ 1, /* p vs dTOF */ 1, /* p vs E    */ 0, /* Q^2       */
        0, /* nu        */ 0, /* X_bjorken */ 0, /* W^2       */ 1  /* Q2 vs nu  */
};
const int STD_VX[STDPLT_LIST_SIZE][2] = {
        {12,15}, {12,20}, {12,19}, {21,-1}, {22,-1}, {23,-1}, {24,-1}, {21,22}
};
const double STD_RX[STDPLT_LIST_SIZE][2][2] = {
        {{ 0,10},{ 0, 1}}, {{ 0,10},{ 0,20}}, {{ 0,10},{ 0, 3}}, {{ 0,12},{-1,-1}},
        {{ 0,12},{-1,-1}}, {{ 0, 2},{-1,-1}}, {{ 0,20},{-1,-1}}, {{ 0,12},{ 0,12}}
};
const long STD_BX[STDPLT_LIST_SIZE][2] = {
        {200,200}, {200,100}, {200,200}, { 22, -1}, { 22, -1}, { 20, -1}, {200, -1}, {200,200}
};
