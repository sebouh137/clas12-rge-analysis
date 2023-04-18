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
const char *R_VAR_LIST[VAR_LIST_SIZE] = {
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
const char *S_VAR_LIST[VAR_LIST_SIZE] = {
        S_RUNNO, S_EVENTNO, S_BEAME,
        S_PID, S_STATUS, S_CHARGE, S_MASS, S_VX, S_VY, S_VZ, S_PX, S_PY, S_PZ,
                S_P, S_THETA, S_PHI, S_BETA,
        S_CHI2, S_NDF,
        S_PCAL_E, S_ECIN_E, S_ECOU_E, S_TOT_E,
        S_DTOF,
        S_NPHELTCC, S_NPHEHTCC,
        S_Q2, S_NU, S_XB, S_W2,
        S_ZH, S_PT2, S_PL2, S_PHIPQ, S_THETAPQ
};
const char *DIS_LIST[DIS_LIST_SIZE] = {
        R_Q2, R_NU, R_XB, R_W2
};
const char *SIDIS_LIST[SIDIS_LIST_SIZE] = {
        R_ZH, R_PT2, R_PL2, R_PHIPQ, R_THETAPQ
};
