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

#ifndef CONSTANTS
#define CONSTANTS

#include <math.h>
#include <map>

// Physics constants.
#define SPEEDOFLIGHT 29.9792458

// PID arrays.
extern const std::map<int, const char*> PID_NAME;
extern const std::map<int, double> PID_MASS;
extern const std::map<int, int> PID_QA;

// Tree name.
#define TREENAME "data"

// Particle cut array.
#define PART_LIST_SIZE 5
extern const char *PART_LIST[PART_LIST_SIZE];
#define R_PALL "all"     // All particles.
#define A_PALL 0
#define R_PPOS "+"       // Positive.
#define A_PPOS 1
#define R_PNEG "-"       // Negative.
#define A_PNEG 2

#define R_PNEU "neutral" // Neutral.
#define A_PNEU 3
#define R_PPID "pid"     // Filter by PID.
#define A_PPID 4

// Tracker types.
#define TRK_LIST_SIZE 2
extern const char *TRK_LIST[TRK_LIST_SIZE];
#define S_DC  "dc"
#define A_DC  0
#define S_FMT "fmt"
#define A_FMT 1

// Plot types.
#define PLOT_LIST_SIZE 2
extern const char *PLOT_LIST[PLOT_LIST_SIZE];
#define R_PLOT1D   "1d"
#define R_PLOT2D   "2d"

#define DIM_LIST_SIZE 2
extern const char *DIM_LIST[DIM_LIST_SIZE];
#define S_DIM1   "x"
#define S_DIM2   "y"

// Miscellaneous.
extern const char *RAN_LIST[2];
#define S_LOWER "lower"
#define S_UPPER "upper"

// "Standard" plots.
#define STDPLT_LIST_SIZE 11
extern const int    STD_PX[STDPLT_LIST_SIZE];
extern const int    STD_VX[STDPLT_LIST_SIZE][2];
extern const double STD_RX[STDPLT_LIST_SIZE][2][2];
extern const long   STD_BX[STDPLT_LIST_SIZE][2];

// Acceptance corrected plots.
#define ACCPLT_LIST_SIZE 5
extern const int    ACC_PX[ACCPLT_LIST_SIZE];
extern const int    ACC_VX[ACCPLT_LIST_SIZE][2];
extern const double ACC_RX[ACCPLT_LIST_SIZE][2][2];
extern const long   ACC_BX[ACCPLT_LIST_SIZE][2];

// All variables.
#define S_PARTICLE "particle"
#define VAR_LIST_SIZE 35
extern const char *R_VAR_LIST[VAR_LIST_SIZE];
extern const char *S_VAR_LIST[VAR_LIST_SIZE];

// Metadata.
#define S_RUNNO   "N_{run}"
#define R_RUNNO   "run_no"
#define A_RUNNO   0
#define S_EVENTNO "N_{event}"
#define R_EVENTNO "event_no"
#define A_EVENTNO 1
#define S_BEAME   "E_{beam}"
#define R_BEAME   "beam_energy"
#define A_BEAME   2

// Particle.
#define S_PID    "pid"
#define R_PID    "pid"
#define A_PID    3
#define S_CHARGE "charge"
#define R_CHARGE "charge"
#define A_CHARGE 4
#define S_STATUS "status"
#define R_STATUS "status"
#define A_STATUS 5
#define S_MASS   "mass"   // GeV.
#define R_MASS   "mass"
#define A_MASS   6
#define S_VX     "vx"     // cm.
#define R_VX     "vx"
#define A_VX     7
#define S_VY     "vy"     // cm.
#define R_VY     "vy"
#define A_VY     8
#define S_VZ     "vz"     // cm.
#define R_VZ     "vz"
#define A_VZ     9
#define S_PX     "p_{x}"  // GeV.
#define R_PX     "px"
#define A_PX     10
#define S_PY     "p_{y}"  // GeV.
#define R_PY     "py"
#define A_PY     11
#define S_PZ     "p_{z}"  // GeV.
#define R_PZ     "pz"
#define A_PZ     12
#define S_P      "p"      // GeV.
#define R_P      "p"
#define A_P      13
#define S_THETA  "#theta" // #degree.
#define R_THETA  "theta"
#define A_THETA  14
#define S_PHI    "#phi"   // #degree.
#define R_PHI    "phi"
#define A_PHI    15
#define S_BETA   "#beta"  // adimensional.
#define R_BETA   "beta"
#define A_BETA   16

// Tracking.
#define S_CHI2   "chi2"
#define R_CHI2   "chi2"
#define A_CHI2   17
#define S_NDF    "NDF"
#define R_NDF    "NDF"
#define A_NDF    18

// Calorimeter.
#define S_PCAL_E "E_{pcal}"  // GeV.
#define R_PCAL_E "e_pcal"
#define A_PCAL_E 19
#define S_ECIN_E "E_{ecin}"  // GeV.
#define R_ECIN_E "e_ecin"
#define A_ECIN_E 20
#define S_ECOU_E "E_{ecou}"  // GeV.
#define R_ECOU_E "e_ecou"
#define A_ECOU_E 21
#define S_TOT_E  "E_{total}" // GeV.
#define R_TOT_E  "e_total"
#define A_TOT_E  22

// Scintillator.
#define S_DTOF "#DeltaTOF" // ns.
#define R_DTOF "dtof"
#define A_DTOF 23

// Cherenkov.
#define S_NPHELTCC "N_{pheLTCC}"
#define R_NPHELTCC "nphe_ltcc"
#define A_NPHELTCC 24

#define S_NPHEHTCC "N_{pheHTCC}"
#define R_NPHEHTCC "nphe_htcc"
#define A_NPHEHTCC 25

// DIS.
#define DIS_LIST_SIZE 4
extern const char *DIS_LIST[DIS_LIST_SIZE];

#define S_Q2 "Q2"          // GeV^2.
#define R_Q2 "q2"
#define A_Q2 26
#define S_NU "#nu"         // GeV.
// #define S_NU "nu"
#define R_NU "nu"
#define A_NU 27
#define S_XB "x_{bjorken}" // adimensional.
#define R_XB "x_bjorken"
#define A_XB 28
#define S_W2 "W2"          // GeV^2.
#define R_W2 "w2"
#define A_W2 29

// SIDIS.
// using usual name convention...
#define SIDIS_LIST_SIZE 5
extern const char *SIDIS_LIST[SIDIS_LIST_SIZE];

#define S_ZH      "z_{h}"       // adimensional.
// #define S_ZH      "zh"
#define R_ZH      "zh"
#define A_ZH       30
#define S_PT2     "Pt2"         // GeV^2.
#define R_PT2     "pt2"
#define A_PT2      31
#define S_PL2     "Pl2"         // GeV^2.
#define R_PL2     "pl2"
#define A_PL2      32
#define S_PHIPQ   "#phi_{PQ}"   // Rad.
// #define S_PHIPQ   "phiPQ"
#define R_PHIPQ   "phipq"
#define A_PHIPQ    33
#define S_THETAPQ "#theta_{PQ}" // Rad.
#define R_THETAPQ "thetapq"
#define A_THETAPQ  34

// #define PHOTONTHETA "virtual photon #theta (lab frame #degree)"
// #define PHOTONPHI   "virtual photon #phi (lab frame #degree)"

// Sampling fraction constants.
#define S_EDIVP    "E/p"
#define PCAL_IDX   0   // PCAL idx in Sampling fraction arrays.
#define ECIN_IDX   1   // ECIN idx in Sampling fraction arrays.
#define ECOU_IDX   2   // ECOU idx in Sampling fraction arrays.
#define CALS_IDX   3   // CALs idx in Sampling fraction arrays.
#define SF_PMIN    1.0 // GeV
#define SF_PMAX    9.0 // GeV
#define SF_PSTEP   0.4 // GeV
#define SF_NPARAMS 4
#define SF_CHI2CONFORMITY 2 // NOTE. This is a source of systematic error!
extern const char *CALNAME[4]; // Calorimeters names.
extern const char *SFARR2D[4]; // Sampling Fraction (SF) 2D arr names.
extern const char *SFARR1D[4]; // SF 1D arr names.
extern const double PLIMITSARR[SF_NPARAMS][2]; // Momentum limits for 1D SF fit.

// Run constants (TODO. these should be in a map or taken from clas12mon.)
#define BE11983 10.3894 //  50 nA.
#define BE12016 10.3894 // 250 nA.
#define BE12439  2.1864 //  15 nA.
#define BE12933 10.4057 // 250 nA.

// Detector constants.
#define NSECTORS    6 // # of CLAS12 sectors.
#define FTOF_ID    12 // FTOF detector id.
#define HTCC_ID    15 // HTCC detector id.
#define LTCC_ID    16 // LTCC detector id.
#define FTOF1A_LYR  1 // FTOF1A layer id.
#define FTOF1B_LYR  2 // FTOF1B layer id.
#define FTOF2_LYR   3 // FTOF2 layer id.
#define PCAL_LYR    1 // PCAL's layer id.
#define ECIN_LYR    4 // EC inner layer id.
#define ECOU_LYR    7 // EC outer layer id.

// General Cuts.
#define FMTNLYRSCUT   3    // # of FMT layers required to accept track.
#define Q2CUT         1.   // Q2 of particle must be over this value.
#define WCUT          2.   // W of particle must be over this value.
#define W2CUT         4.   // W2 of particle must be over this value.
#define ZHCUT         1.   // zh of particle must be below this value.
#define YBCUT         0.85 // Yb of particle must be below this value.
#define CHI2NDFCUT   15    // Chi2/NDF must be below this value.
#define VXVYCUT       4    // sqrt(vx^2 + vy^2) must be below this value.
#define VZLOWCUT    -40    // vz must be above this value.
#define VZHIGHCUT    40    // vz must be below this.

// PID Cuts. Most numbers come from `/calibration/eb/` from the CCDB.
#define NPIDS           6     // # of (signless) PIDs program is capable of
                              //         identifying.
#define HTCC_NPHE_CUT   2     // # of HTCC photoelectrons required to consider
                              //         particle passed HTCC.
#define LTCC_NPHE_CUT   2     // # of LTCC photoelectrons required to consider
                              //         particle passed LTCC.
#define MIN_PCAL_ENERGY 0.060 // Min energy deposited in PCAL to assign pid = 11.
#define E_SF_NSIGMA     5.0   // Max ECAL sampling fraction sigma to assign
                              //         pid = 11.
#define NEUTRON_MAXBETA 0.9   // Max beta allowed to assign pid == 2212.
#define HTCC_PION_THRESHOLD 4.9 // Momentum required to consider particle
                                //         crossing HTCC as pion.
#define LTCC_PION_THRESHOLD 3.0 // Momentum required to consider particle
                                //         crossing LTCC as pion.
#define LTCC_KAON_THRESHOLD 5.0 // Momentum required to consider particle
                                //         crossing LTCC as kaon.

#endif
