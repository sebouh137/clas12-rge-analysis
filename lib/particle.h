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

#ifndef PARTICLE
#define PARTICLE

#include "bank_containers.h"
#include "utilities.h"

#define PID_POSITIVE_SIZE 5
#define PID_NEGATIVE_SIZE 4
#define PID_NEUTRAL_SIZE  2
extern const int PID_POSITIVE[PID_POSITIVE_SIZE];
extern const int PID_NEGATIVE[PID_NEGATIVE_SIZE];
extern const int PID_NEUTRAL[PID_NEUTRAL_SIZE];

typedef struct {
    bool is_valid;
    bool is_trigger_electron;
    bool is_hadron;
    // EB vars.
    int pid;
    int q;
    int sector;
    float beta;
    // Tracking vars.
    float vx, vy, vz;
    float px, py, pz;
    // Derived vars.
    float mass;
} particle;

// particle functions.
particle particle_init();
particle particle_init(Particle *rp, Track *rt, int pos, bool fmt);
particle particle_init(int charge, double beta, int sector, double vx,
        double vy, double vz, double px, double py, double pz);
int set_pid(particle *p, int recon_pid, int status, double tot_E,
        double pcal_E, int htcc_nphe, int ltcc_nphe,
        double sf_params[SF_NPARAMS][2]);
bool is_electron(double tot_E, double pcal_E, double htcc_nphe, double p,
        double pars[SF_NPARAMS][2]);
int assign_neutral_pid(double tot_E, double beta);
int best_pid_from_momentum(double p, double beta, int pid_list[],
        int pid_list_size);
int match_pid(int hyp, bool r_match, int q, bool e, bool htcc_s, bool htcc_p);
float d_from_beamline(particle p);
float theta_lab(particle p);
float phi_lab(particle p);
float P(particle p);
float mass2(particle p);

// SIDIS e- functions.
float nu(particle p, double beam_E);
float Q2(particle p, double beam_E);
float Xb(particle p, double beam_E);
float Yb(particle p, double beam_E);
float theta_photon_lab(particle p, double beam_E);
float phi_photon_lab(particle p);
float W(particle p, double beam_E);
float W2(particle p, double bE);

// SIDIS produced particle functions.
float theta_pq(particle p, particle e, double bE);
float phi_pq(particle p, particle e, double bE);
float cos_theta_pq(particle p, particle e, double bE);
float Pt2(particle p, particle e, double bE);
float Pl2(particle p, particle e, double bE);
float zh(particle p, particle e, double bE);
float PlCM(particle p, particle e, double bE);
float PmaxCM(particle p, particle e, double bE);
float PTrans2PQ(particle p, particle e, double bE);
float PLong2PQ(particle p, particle e, double bE);
float Xf(particle p, particle e, double bE);
float Mx2(particle p, particle e, double bE);
float t_mandelstam(particle p, particle e, double bE);

// Fill array for ntuples file.
int fill_ntuples_arr(Float_t *vec, particle p, particle e, int run_no, int evn,
        int status, double beam_E, float chi2, float ndf, double pcal_E,
        double ecin_E, double ecou_E, double tof, double tre_tof);

#endif
