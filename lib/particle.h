#ifndef PARTICLE
#define PARTICLE

#include <math.h>
#include <stdbool.h>

#include "constants.h"
#include "utilities.h"

typedef struct {
    bool is_trigger_electron;
    // EB vars.
    int pid;
    int q;
    int sector;
    double beta;
    // Tracking vars.
    double vx, vy, vz;
    double px, py, pz;
    // Derived vars.
    double P;
    double mass;
} particle;

// particle functions.
particle particle_init(int pid, int charge, double beta, int status, int sector,
                       double vx, double vy, double vz, double px, double py, double pz);
double d_from_beamline(particle p);
double theta_lab(particle p);
double phi_lab(particle p);
double P(particle p);
double mass2(particle p);

// SIDIS e- functions.
double nu(particle p, double beam_E);
double Q2(particle p, double beam_E);
double Xb(particle p, double beam_E);
double Yb(particle p, double beam_E);
double theta_photon_lab(particle p, double beam_E);
double phi_photon_lab(particle p);
double W(particle p, double beam_E);

// SIDIS produced particle functions.
double theta_pq(particle p, particle e, double bE);
double phi_pq(particle p, particle e, double bE);
double cos_theta_pq(particle p, particle e, double bE);
double Pt2(particle p, particle e, double bE);
double Pl2(particle p, particle e, double bE);
double zh(particle p, particle e, double bE);
double PlCM(particle p, particle e, double bE);
double PmaxCM(particle p, particle e, double bE);
double PTrans2PQ(particle p, particle e, double bE);
double PLong2PQ(particle p, particle e, double bE);

#endif
