#include "../lib/particle.h"

// Initialize a new particle.
particle particle_init(int pid, int charge, double beta, int status, double beam_E,
                       double vx, double vy, double vz, double px, double py, double pz) {
    particle p;

    // Inherent values.
    p.is_trigger_electron = (pid == 11 && status < 0);
    p.pid  = pid;
    p.q    = charge;
    p.beta = beta;
    p.vx   = vx;
    p.vy   = vy;
    p.vz   = vz;
    p.px   = px;
    p.py   = py;
    p.pz   = pz;

    // Derived values.
    p.theta = calc_theta(p);
    p.phi   = calc_phi(p);
    p.P     = calc_P(p);

    // SIDIS variables.
    if (p.pid == 11) {
        p.Q2 = calc_Q2(p, beam_E);
        p.nu = calc_nu(p, beam_E);
        p.Xb = calc_Xb(p, beam_E);
    }

    return p;
}

// Get distance from beamline.
double d_from_beamline(particle p) {
    return (p.vx*p.vx + p.vy+p.vy);
}

// Calculate theta angle from momentum components of particle.
double calc_theta(particle p) {
    return p.px == 0.0 && p.py == 0.0 && p.pz == 0.0 ? 0.0 : atan2(sqrt(p.px*p.px + p.py*p.py), p.pz);
}

// Calculate phi angle from momentum components of particle.
double calc_phi(particle p) {
    return atan2(p.py, p.px);
}

// Calculate momentum magnitude from its components.
double calc_P(particle p) {
    return sqrt(p.px*p.px + p.py*p.py + p.pz*p.pz);
}

// Calculate Q^2 from beam energy, particle momentum, and theta angle.
double calc_Q2(particle p, double beam_E) {
    return 4 * beam_E * p.P * pow(sin(p.theta/2), 2);
}

// Calculate nu from beam energy and total momentum.
double calc_nu(particle p, double beam_E) {
    return beam_E - p.P;
}

// Calculate x_bjorken from beam energy, particle momentum, and theta angle.
double calc_Xb(particle p, double beam_E) {
    return p.Q2/2 / (p.nu/PRTMASS);
}
