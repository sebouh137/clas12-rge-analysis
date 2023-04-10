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

#include "../lib/pid_utils.h"

// --+ static +-----------------------------------------------------------------
/** Map linking PIDs to pid_constants. */
static const std::map<int, pid_constants> PID_MAP = {
    {     -2212, pid_constants_init( 1, 0.938272, "antiproton"           )},
    {      -321, pid_constants_init(-1, 0.493677, "negative kaon"        )},
    {      -211, pid_constants_init(-1, 0.139570, "negative pion"        )},
    {       -11, pid_constants_init( 1, 0.000051, "positron"             )},
    {         0, pid_constants_init( 0, DBL_MAX,  "unidentified particle")},
    {        11, pid_constants_init(-1, 0.000051, "electron"             )},
    {        22, pid_constants_init( 0, 0.,       "photon"               )},
    {        45, pid_constants_init( 0, DBL_MAX,  "unidentified particle")},
    {       211, pid_constants_init( 1, 0.139570, "positive pion"        )},
    {       321, pid_constants_init( 1, 0.493677, "positive kaon"        )},
    {      2112, pid_constants_init( 0, 0.939565, "neutron"              )},
    {      2212, pid_constants_init( 1, 0.938272, "proton"               )}

    // NOTE. Not yet implemented.
    // {        13, pid_constants_init( 1, 0.105658, "positive muon"     )},
    // {       -13, pid_constants_init(-1, 0.105658, "negative muon"     )},
    // {       111, pid_constants_init( 0, 0.134977, "neutral pion"      )},
    // {       311, pid_constants_init( 0, 0.497614, "neutral kaon"      )},
    // {       221, pid_constants_init( 0, 0.548953, "eta"               )},
    // {       223, pid_constants_init( 0, 0.782650, "omega"             )},
    // {1000010020, pid_constants_init( 0, 1.875,    "deuterium"         )}
};

static pid_constants pid_constants_init(int c, double m, const char *n) {
    pid_constants p;
    p.charge = c; // electron charge.
    p.mass   = m; // GeV.
    p.name   = n;

    return p;
}

static int pid_invalid(int pid) {
    if (PID_MAP.contains(pid)) return 0;
    else                       return 1;
}

// --+ library +----------------------------------------------------------------
int rge_get_charge(int pid, int *charge) {
    if (pid_invalid(pid)) {
        rge_errno = ERR_PIDNOTFOUND;
        return 1;
    }

    *charge = PID_MAP.at(pid).charge;
    return 0;
}

int rge_get_mass(int pid, double *mass) {
    if (pid_invalid(pid)) {
        rge_errno = ERR_PIDNOTFOUND;
        return 1;
    }

    *mass = PID_MAP.at(pid).mass;
    return 0;
}

int rge_get_pidlist_size_by_charge(int charge, unsigned int *size) {
    for (
            std::map<int, pid_constants>::const_iterator it = PID_MAP.begin();
            it != PID_MAP.end();
            ++it
    ) {
        if (
                (charge == 0 && it->second.charge == 0) || // both are neutral.
                (charge * it->second.charge > 0)           // equal signs.
        ) {
            ++(*size);
        }
    }

    return 0;
}

int rge_get_pidlist_by_charge(int charge, int pidlist[]) {
    unsigned int counter = 0;
    for (
            std::map<int, pid_constants>::const_iterator it = PID_MAP.begin();
            it != PID_MAP.end();
            ++it
    ) {
        if (
                (charge == 0 && it->second.charge == 0) || // both are neutral.
                (charge * it->second.charge > 0)           // equal signs.
        ) {
            pidlist[counter] = it->first;
            ++counter;
        }
    }

    return 0;
}

int rge_print_pid_names() {
    for (
            std::map<int, pid_constants>::const_iterator it = PID_MAP.begin();
            it != PID_MAP.end();
            ++it
    ) {
        printf("  * %5d (%s).\n", it->first, it->second.name);
    }

    return 0;
}
