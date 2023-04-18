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
extern const char *VAR_LIST[VAR_LIST_SIZE];

// Metadata.
#define R_RUNNO   "run_no"
#define A_RUNNO   0
#define R_EVENTNO "event_no"
#define A_EVENTNO 1
#define R_BEAME   "beam_energy"
#define A_BEAME   2

// Particle.
#define R_PID    "pid"
#define A_PID    3
#define R_CHARGE "charge"
#define A_CHARGE 4
#define R_STATUS "status"
#define A_STATUS 5
#define R_MASS   "mass" // GeV.
#define A_MASS   6
#define R_VX     "vx" // cm.
#define A_VX     7
#define R_VY     "vy" // cm.
#define A_VY     8
#define R_VZ     "vz" // cm.
#define A_VZ     9
#define R_PX     "px" // GeV.
#define A_PX     10
#define R_PY     "py" // GeV.
#define A_PY     11
#define R_PZ     "pz" // GeV.
#define A_PZ     12
#define R_P      "p" // GeV.
#define A_P      13
#define R_THETA  "theta" // #degree.
#define A_THETA  14
#define R_PHI    "phi" // #degree.
#define A_PHI    15
#define R_BETA   "beta" // adimensional.
#define A_BETA   16

// Tracking.
#define R_CHI2   "chi2"
#define A_CHI2   17
#define R_NDF    "NDF"
#define A_NDF    18

// Calorimeter.
#define R_PCAL_E "e_pcal" // GeV.
#define A_PCAL_E 19
#define R_ECIN_E "e_ecin" // GeV.
#define A_ECIN_E 20
#define R_ECOU_E "e_ecou" // GeV.
#define A_ECOU_E 21
#define R_TOT_E  "e_total" // GeV.
#define A_TOT_E  22

// Scintillator.
#define R_DTOF "dtof" // ns.
#define A_DTOF 23

// Cherenkov.
#define R_NPHELTCC "nphe_ltcc"
#define A_NPHELTCC 24
#define R_NPHEHTCC "nphe_htcc"
#define A_NPHEHTCC 25

// DIS.
#define DIS_LIST_SIZE 4
extern const char *DIS_LIST[DIS_LIST_SIZE];

#define R_Q2 "q2" // GeV^2.
#define A_Q2 26
#define R_NU "nu" // GeV.
#define A_NU 27
#define R_XB "x_bjorken" // adimensional.
#define A_XB 28
#define R_W2 "w2" // GeV^2.
#define A_W2 29

// SIDIS.
#define R_ZH      "zh" // adimensional.
#define A_ZH       30
#define R_PT2     "pt2" // GeV^2.
#define A_PT2      31
#define R_PL2     "pl2" // GeV^2.
#define A_PL2      32
#define R_PHIPQ   "phipq" // radians.
#define A_PHIPQ    33
#define R_THETAPQ "thetapq" // radians.
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
