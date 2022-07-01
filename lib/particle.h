#ifndef PARTICLE
#define PARTICLE

#include <math.h>
#include <stdbool.h>

#include "bank_containers.h"
#include "constants.h"
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
    // EB vars.
    int pid;
    int q;
    int sector;
    double beta;
    // Tracking vars.
    double vx, vy, vz;
    double px, py, pz;
    // Derived vars.
    double mass;
} particle;

// particle functions.
particle particle_init();
particle particle_init(REC_Particle * rp, REC_Track * rt, int pos);
particle particle_init(REC_Particle * rp, REC_Track * rt, FMT_Tracks * ft, int pos);
particle particle_init(int charge, double beta, int sector,
                       double vx, double vy, double vz, double px, double py, double pz);
int set_pid(particle * p, int recon_pid, int status, double tot_E, double pcal_E, int htcc_nphe,
            int ltcc_nphe, double sf_params[SF_NPARAMS][2]);
bool is_electron(double tot_E, double pcal_E, double htcc_nphe, double p,
                 double pars[SF_NPARAMS][2]);
int assign_neutral_pid(double tot_E, double beta);
int best_pid_from_momentum(double p, double beta, int pid_list[], int pid_list_size);
int match_pid(int hyp, bool r_match, int q, bool e, bool htcc_s, bool htcc_p);
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
double W2(particle p, double bE);

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
double Xf(particle p, particle e, double bE);
double Mx2(particle p, particle e, double bE);
double t_mandelstam(particle p, particle e, double bE);

#endif
