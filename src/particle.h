#ifndef PARTICLE
#define PARTICLE

#include <math.h>
#include <stdbool.h>

#include "constants.h"

typedef struct {
    bool is_trigger_electron;
    int pid, q; double beta;       // EB vars.
    double vx, vy, vz, px, py, pz; // Tracking vars.
    double theta, phi, P;          // Derived vars.
    double Q2, nu, Xb;             // SIDIS vars.
} particle;
particle particle_init(int pid, int charge, double beta, int status, double beam_E,
                       double vx, double vy, double vz, double px, double py, double pz);
double d_from_beamline(particle p);
double calc_theta(particle p);
double calc_phi(particle p);
double calc_P(particle p);
double calc_Q2(particle p, double beam_E);
double calc_nu(particle p, double beam_E);
double calc_Xb(particle p, double beam_E);

#endif
