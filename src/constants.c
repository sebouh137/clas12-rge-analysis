// CLAS12 RG-E Analyser.
// Copyright (C) 2022 Bruno Benkel
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
//
// You can see a copy of the GNU Lesser Public License under the LICENSE file.

#include "../lib/constants.h"

// Particle name map.
const std::map<int, const char*> PID_NAME = {
        {        11,  "electron"},
        {       -11,  "positron"},
        {      2212,  "proton"},
        {     -2212,  "antiproton"},
        {      2112,  "neutron"},
        {       211,  "positive pion"},
        {      -211,  "negative pion"},
        {       321,  "positive kaon"},
        {      -321,  "negative kaon"},
        // {        13,  "positive muon"}, // NOTE. not yet implemented.
        // {       -13,  "negative muon"}, // NOTE. not yet implemented.
        // {       111,  "neutral pion"},  // NOTE. not yet implemented.
        // {       311,  "neutral kaon"},  // NOTE. not yet implemented.
        {        22,  "photon"},
        // {       221,  "eta"},           // NOTE. not yet implemented.
        // {       223,  "omega"},         // NOTE. not yet implemented.
        // {1000010020,  "deuterium"},     // NOTE. not yet implemented.
        {        45,  "unidentified particle"},
        {         0,  "unidentified particle"}
};

// Mass map in GeV.
const std::map<int, double> PID_MASS = {
        {        11,  0.000051}, // electron.
        {       -11,  0.000051}, // positron.
        {      2212,  0.938272}, // proton.
        {     -2212,  0.938272}, // antiproton.
        {      2112,  0.939565}, // neutron.
        {       211,  0.139570}, // positive pion.
        {      -211,  0.139570}, // negative pion.
        {       321,  0.493677}, // positive kaon.
        {      -321,  0.493677}, // negative kaon.
        // {        13,  0.105658}, // positive muon. NOTE. not yet implemented.
        // {       -13,  0.105658}, // negative muon. NOTE. not yet implemented.
        // {       111,  0.134977}, // neutral pion. NOTE. not yet implemented.
        // {       311,  0.497614}, // neutral kaon. NOTE. not yet implemented.
        {        22,  0.},       // photon.
        // {       221,  0.548953}, // eta. NOTE. not yet implemented.
        // {       223,  0.782650}, // omega. NOTE. not yet implemented.
        // {1000010020,  1.875},    // deuterium. NOTE. not yet implemented.
        {        45, -1.},       // unidentified particle.
        {         0, -1.}        // unidentified particle.
};

// PID qa map.
const std::map<int, int> PID_QA = {
    {11, 0}, {211, 1}, {321, 2}, {2212, 3}, {2112, 4}, {22, 5}
};

// Trackers array.
const char *TRK_LIST[TRK_LIST_SIZE] = {
    S_DC, S_FMT
};

// Particle constant array.
const char *PART_LIST[PART_LIST_SIZE] = {
    R_PALL, R_PPOS, R_PNEG, R_PNEU, R_PPID
};

// Plotting constant arrays.
const char *PLOT_LIST[PLOT_LIST_SIZE] = {
        R_PLOT1D, R_PLOT2D
};
const char *DIM_LIST[DIM_LIST_SIZE] = {
        S_DIM1, S_DIM2
};
const char *RAN_LIST[2] = {
        S_LOWER, S_UPPER
};
const char *R_VAR_LIST[VAR_LIST_SIZE] = {
        R_RUNNO, R_EVENTNO, R_BEAME,
        R_PID, R_STATUS, R_CHARGE, R_MASS, R_VX, R_VY, R_VZ, R_PX, R_PY, R_PZ,
                R_P, R_THETA, R_PHI, R_BETA,
        R_CHI2, R_NDF,
        R_PCAL_E, R_ECIN_E, R_ECOU_E, R_TOT_E,
        R_DTOF,
        R_Q2, R_NU, R_XB, R_W2,
        R_ZH, R_PT2, R_PL2, R_PHIPQ, R_THETAPQ
};
const char *S_VAR_LIST[VAR_LIST_SIZE] = {
        S_RUNNO, S_EVENTNO, S_BEAME,
        S_PID, S_STATUS, S_CHARGE, S_MASS, S_VX, S_VY, S_VZ, S_PX, S_PY, S_PZ,
                S_P, S_THETA, S_PHI, S_BETA,
        S_CHI2, S_NDF,
        S_PCAL_E, S_ECIN_E, S_ECOU_E, S_TOT_E,
        S_DTOF,
        S_Q2, S_NU, S_XB, S_W2,
        S_ZH, S_PT2, S_PL2, S_PHIPQ, S_THETAPQ
};
const char *DIS_LIST[DIS_LIST_SIZE] = {
        R_Q2, R_NU, R_XB, R_W2
};
const char *SIDIS_LIST[SIDIS_LIST_SIZE] = {
        R_ZH, R_PT2, R_PL2, R_PHIPQ, R_THETAPQ
};

// Standard plots constant arrays.
const int STD_PX[STDPLT_LIST_SIZE] = {
        1, /* p vs beta */ 1, /* p vs dTOF */ 1, /* p vs E    */
        0, /* Q^2       */ 0, /* nu        */ 0, /* zh        */
        0, /* Pt2       */ 0, /* phi_PQ    */
        0, /* x_bjorken */ 0, /* W^2       */ 1  /* Q2 vs nu  */
};
const int STD_VX[STDPLT_LIST_SIZE][2] = {
        {A_P,A_BETA},      {A_P,A_DTOF},      {A_P,A_TOT_E},
        {A_Q2,-1},         {A_NU,-1},         {A_ZH,-1},
        {A_PT2,-1},        {A_PHIPQ,-1},
        {A_XB,-1},         {A_W2,-1},         {A_Q2,A_NU}
};
const double STD_RX[STDPLT_LIST_SIZE][2][2] = {
        {{ 0,10},{ 0, 1}}, {{ 0,10},{ 0,20}}, {{ 0,10},{ 0, 3}},
        {{ 0,12},{-1,-1}}, {{ 0,12},{-1,-1}}, {{ 0, 1},{-1,-1}},
        {{ 0,12},{-1,-1}}, {{ -M_PI,M_PI},{-1,-1}},
        {{ 0, 1},{-1,-1}}, {{ 0,20},{-1,-1}}, {{ 0,12},{ 0,12}}
};
const long STD_BX[STDPLT_LIST_SIZE][2] = {
        {200,200},         {200,100},         {200,200},
        {400, -1},         {400, -1},         {400, -1},
        {400, -1},         {400, -1},
        {400, -1},         {400, -1},         {200,200}
};

// Sampling fraction constant arrays.
const char *CALNAME[4] = {
        "PCAL", "ECIN", "ECOU", "ALL"
};
const char *SFARR2D[4] = {
        "Vp vs E/Vp (PCAL sector ",
        "Vp vs E/Vp (ECIN sector ",
        "Vp vs E/Vp (ECOU sector ",
        "Vp vs E/Vp (CALs sector "
};
const char *SFARR1D[4] = {
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
