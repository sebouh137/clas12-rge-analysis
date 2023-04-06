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

#include "../lib/pid_constants.h"

/** Initialize one pid_constants struct with the given data. */
static pid_constants pid_constants_init(int c, double m, const char *n) {
    pid_constants p;
    p.charge = c; // electron charge.
    p.mass   = m; // GeV.
    p.name   = n;

    return p;
}

/** Map linking PIDs to pid_constants. */
const std::map<int, pid_constants> PID_MAP = {
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

// Functions for getting stuff from a particular PID.
// TODO. Add an rge_errno if PID is not found in PID_MAP.
int get_charge(int pid) {return PID_MAP.at(pid).charge;}
double get_mass(int pid) {return PID_MAP.at(pid).mass;}
const char *get_name(int pid) {return PID_MAP.at(pid).name;}
