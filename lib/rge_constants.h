// CLAS12 RG-E Analyser.
// Copyright (C) 2022-2023 Bruno Benkel
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

#ifndef RGE_CONSTANTS
#define RGE_CONSTANTS

// --+ preamble +---------------------------------------------------------------
// C.
#include <math.h>

// C++.
#include <map>

// typedefs.
typedef unsigned int uint;
typedef long unsigned int luint;
typedef long int lint;

// --+ internal +---------------------------------------------------------------
// --+ library +----------------------------------------------------------------
// All variables.
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
#define SIDIS_LIST_SIZE 5
extern const char *SIDIS_LIST[SIDIS_LIST_SIZE];

#define S_ZH      "z_{h}"       // adimensional.
#define R_ZH      "zh"
#define A_ZH       30
#define S_PT2     "Pt2"         // GeV^2.
#define R_PT2     "pt2"
#define A_PT2      31
#define S_PL2     "Pl2"         // GeV^2.
#define R_PL2     "pl2"
#define A_PL2      32
#define S_PHIPQ   "#phi_{PQ}"   // Rad.
#define R_PHIPQ   "phipq"
#define A_PHIPQ    33
#define S_THETAPQ "#theta_{PQ}" // Rad.
#define R_THETAPQ "thetapq"
#define A_THETAPQ  34

/** Data tree name used by various programs. */
#define RGE_TREENAMEDATA "data"

/** Detector constants. */
#define RGE_NSECTORS     6 /** # of CLAS12 sectors. */
#define RGE_NSFPARAMS    4 /** # of sampling fraction parameters. */

/** Cuts (geometric, fiducial, SIDIS, etc). */
#define RGE_Q2CUT        1.   /** Q2 of particle must be over this value. */
#define RGE_WCUT         2.   /** W of particle must be over this value. */
#define RGE_W2CUT        4.   /** W2 of particle must be over this value. */
#define RGE_ZHCUT        1.   /** zh of particle must be below this value. */
#define RGE_YBCUT        0.85 /** Yb of particle must be below this value. */
#define RGE_CHI2NDFCUT  15    /** Chi2/NDF must be below this value. */
#define RGE_VXVYCUT      4    /** sqrt(vx^2 + vy^2) must be below this value. */
#define RGE_VZLOWCUT   -40    /** vz must be above this value. */
#define RGE_VZHIGHCUT   40    /** vz must be below this. */

#endif
