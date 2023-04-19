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

// --+ structs +----------------------------------------------------------------
/**
 * Struct containing the address in RGE_VARS and name of a variable.
 * NOTE. This approach is error-prone and hard to work with -- I would prefer to
 *     use an std::map<const char *, double, cmp_str>, but ROOT's dumb TNTuple
 *     object constructor rules make this approach uglier in comparison.
 *         -Bruno.
 */
typedef struct {
    int addr;
    const char *name;
} RGE_VAR;

// --+ library +----------------------------------------------------------------
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

/** Variable array data. */
#define RGE_VARS_SIZE 35
extern const char *RGE_VARS[RGE_VARS_SIZE];

// Metadata.
__extension__ const RGE_VAR RGE_RUNNO   = {.addr = 0, .name = "run_no"};
__extension__ const RGE_VAR RGE_EVENTNO = {.addr = 1, .name = "event_no"};
__extension__ const RGE_VAR RGE_BEAME   = {.addr = 2, .name = "beam_energy"};

// Particle.
__extension__ const RGE_VAR RGE_PID    = {.addr =  3, .name = "pid"};
__extension__ const RGE_VAR RGE_CHARGE = {.addr =  4, .name = "charge"};
__extension__ const RGE_VAR RGE_STATUS = {.addr =  5, .name = "status"};
__extension__ const RGE_VAR RGE_MASS   = {.addr =  6, .name = "mass"};
__extension__ const RGE_VAR RGE_VX     = {.addr =  7, .name = "vx"};
__extension__ const RGE_VAR RGE_VY     = {.addr =  8, .name = "vy"};
__extension__ const RGE_VAR RGE_VZ     = {.addr =  9, .name = "vz"};
__extension__ const RGE_VAR RGE_PX     = {.addr = 10, .name = "px"};
__extension__ const RGE_VAR RGE_PY     = {.addr = 11, .name = "py"};
__extension__ const RGE_VAR RGE_PZ     = {.addr = 12, .name = "px"};
__extension__ const RGE_VAR RGE_P      = {.addr = 13, .name = "p"};
__extension__ const RGE_VAR RGE_THETA  = {.addr = 14, .name = "theta"};
__extension__ const RGE_VAR RGE_PHI    = {.addr = 15, .name = "phi"};
__extension__ const RGE_VAR RGE_BETA   = {.addr = 16, .name = "beta"};

// Tracking.
__extension__ const RGE_VAR RGE_CHI2 = {.addr = 17, .name = "chi2"};
__extension__ const RGE_VAR RGE_NDF  = {.addr = 18, .name = "NDF"};

// Calorimeter.
__extension__ const RGE_VAR RGE_PCALE = {.addr = 19, .name = "e_pcal"};
__extension__ const RGE_VAR RGE_ECINE = {.addr = 20, .name = "e_ecin"};
__extension__ const RGE_VAR RGE_ECOUE = {.addr = 21, .name = "e_ecou"};
__extension__ const RGE_VAR RGE_TOTE  = {.addr = 22, .name = "e_total"};

// Scintillator.
__extension__ const RGE_VAR RGE_DTOF = {.addr = 23, .name = "dtof"};

// Cherenkov.
__extension__ const RGE_VAR RGE_NPHELTCC = {.addr = 24, .name = "nphe_ltcc"};
__extension__ const RGE_VAR RGE_NPHEHTCC = {.addr = 25, .name = "nphe_htcc"};

// DIS.
__extension__ const RGE_VAR RGE_Q2 = {.addr = 26, .name = "q2"};
__extension__ const RGE_VAR RGE_NU = {.addr = 27, .name = "nu"};
__extension__ const RGE_VAR RGE_XB = {.addr = 28, .name = "x_bjorken"};
__extension__ const RGE_VAR RGE_W2 = {.addr = 29, .name = "w2"};

// SIDIS.
__extension__ const RGE_VAR RGE_ZH      = {.addr = 30, .name = "zh"};
__extension__ const RGE_VAR RGE_PT2     = {.addr = 31, .name = "pt2"};
__extension__ const RGE_VAR RGE_PL2     = {.addr = 32, .name = "pl2"};
__extension__ const RGE_VAR RGE_PHIPQ   = {.addr = 33, .name = "phi_pq"};
__extension__ const RGE_VAR RGE_THETAPQ = {.addr = 34, .name = "theta_pq"};

#endif
