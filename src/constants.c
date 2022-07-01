#include "../lib/constants.h"

// Mass map in GeV.
const std::map<int, double> MASS = {
        {        11,  0.000051}, // electron.
        {      2212,  0.938272}, // proton.
        {      2112,  0.939565}, // neutron.
        {       211,  0.139570}, // charged pion.
        {       321,  0.493677}, // charged kaon.
        {        13,  0.105658}, // charged muon.
        {       111,  0.134977}, // neutral pion.
        {       311,  0.497614}, // neutral kaon.
        {        22,  0.},       // photon.
        {       221,  0.548953}, // eta.
        {       223,  0.782650}, // omega.
        {1000010020,  1.875},    // deuterium.
        {        45, -1.},       // unidentified particle.
        {         0, -1.}        // unidentified particle.
};

// Particle constant array.
const char * PART_LIST[PART_LIST_SIZE] = {
    R_PALL, R_PPOS, R_PNEG, R_PNEU
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
        R_PID, R_STATUS, R_CHARGE, R_MASS, R_VX, R_VY, R_VZ, R_PX, R_PY, R_PZ, R_P, R_THETA, R_PHI,
                R_BETA,
        R_CHI2, R_NDF,
        R_PCAL_E, R_ECIN_E, R_ECOU_E, R_TOT_E,
        R_DTOF,
        R_Q2, R_NU, R_XB, R_W2
};
const char * S_VAR_LIST[VAR_LIST_SIZE] = {
        S_RUNNO, S_EVENTNO, S_BEAME,
        S_PID, S_STATUS, S_CHARGE, S_MASS, S_VX, S_VY, S_VZ, S_PX, S_PY, S_PZ, S_P, S_THETA, S_PHI,
                S_BETA,
        S_CHI2, S_NDF,
        S_PCAL_E, S_ECIN_E, S_ECOU_E, S_TOT_E,
        S_DTOF,
        S_Q2, S_NU, S_XB, S_W2
};
const char * DIS_LIST[DIS_LIST_SIZE] = {
        R_Q2, R_NU, R_XB, R_W2
};

// Standard plots constant arrays.
const int STD_PX[STDPLT_LIST_SIZE] = {
        1, /* p vs beta */ 1, /* p vs dTOF */ 1, /* p vs E    */ 0, /* Q^2       */
        0, /* nu        */ 0, /* x_bjorken */ 0, /* W^2       */ 1  /* Q2 vs nu  */
};
const int STD_VX[STDPLT_LIST_SIZE][2] = {
        {A_P,A_BETA}, {A_P,A_DTOF}, {A_P,A_TOT_E}, {A_Q2,-1}, {A_NU,-1}, {A_XB,-1}, {A_W2,-1},
        {A_Q2,A_NU}
};
const double STD_RX[STDPLT_LIST_SIZE][2][2] = {
        {{ 0,10},{ 0, 1}}, {{ 0,10},{ 0,20}}, {{ 0,10},{ 0, 3}}, {{ 0,12},{-1,-1}},
        {{ 0,12},{-1,-1}}, {{ 0, 2},{-1,-1}}, {{ 0,20},{-1,-1}}, {{ 0,12},{ 0,12}}
};
const long STD_BX[STDPLT_LIST_SIZE][2] = {
        {200,200}, {200,100}, {200,200}, { 22, -1}, { 22, -1}, { 20, -1}, {200, -1}, {200,200}
};

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
