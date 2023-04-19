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

#include "../lib/rge_constants.h"

// --+ library +----------------------------------------------------------------
const char *RGE_VARS[RGE_VARS_SIZE] = {
        R_RUNNO, R_EVENTNO, R_BEAME,
        R_PID, R_STATUS, R_CHARGE, R_MASS, R_VX, R_VY, R_VZ, R_PX, R_PY, R_PZ,
                R_P, R_THETA, R_PHI, R_BETA,
        R_CHI2, R_NDF,
        R_PCAL_E, R_ECIN_E, R_ECOU_E, R_TOT_E,
        R_DTOF,
        R_NPHELTCC, R_NPHEHTCC,
        R_Q2, R_NU, R_XB, R_W2,
        R_ZH, R_PT2, R_PL2, R_PHIPQ, R_THETAPQ
};
