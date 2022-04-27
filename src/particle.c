#include "../lib/particle.h"

// Initialize a new particle.
particle particle_init(int pid, int charge, double beta, int status, int sector,
                       double vx, double vy, double vz, double px, double py, double pz) {
    particle p;

    // Inherent vars.
    p.is_trigger_electron = (pid == 11 && status < 0);
    p.pid    = pid;
    p.q      = charge;
    p.beta   = beta;
    p.sector = sector;

    // Tracking vars.
    p.vx = vx;
    p.vy = vy;
    p.vz = vz;
    p.px = px;
    p.py = py;
    p.pz = pz;

    // NOTE. If programs gets slow, I should cache values of Q2, nu, etc here.

    return p;
}

// Get distance from beamline.
double d_from_beamline(particle p) {
    return (p.vx*p.vx + p.vy+p.vy);
}

// Calculate theta angle in the lab frame from momentum components of particle.
double lab_theta(particle p) {
    return p.px == 0.0 && p.py == 0.0 && p.pz == 0.0 ? 0.0 : atan2(sqrt(p.px*p.px + p.py*p.py), p.pz);
}

// Calculate phi angle in the lab frame from momentum components of particle.
double lab_phi(particle p) {
    return atan2(p.py, p.px);
}

// Calculate momentum magnitude from its components.
double P(particle p) {
    return sqrt(p.px*p.px + p.py*p.py + p.pz*p.pz);
}

// Calculate squared mass from momentum and beta.
double mass2(particle p) {
    return (P(p)*P(p)) / (p.beta*p.beta);
}

// Calculate Q^2 from beam energy, particle momentum, and theta angle.
double Q2(particle p, double bE) {
    if (!p.is_trigger_electron) return 0; // TODO. I need an invalid return value, not zero!
    return 4 * bE * P(p) * pow(sin(lab_theta(p)/2), 2);
}

// Calculate nu from beam energy and total momentum.
double nu(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return bE - P(p);
}

// Calculate x_bjorken from beam energy, particle momentum, and theta angle.
double Xb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return (Q2(p, bE)/2) / (nu(p, bE)/PRTMASS);
}

// Calculate y_bjorken from beam energy and nu.
double Yb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return (nu(p, bE) / bE);
}

// Calculate the virtual photon's theta angle in the lab frame.
double lab_photon_theta(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return acos((bE - P(p)*cos(lab_theta(p))) / (sqrt(Q2(p, bE)+nu(p, bE)*nu(p, bE))));
}

// Calculate the virtual photon's phi angle in the lab frame.
double lab_photon_phi(particle p) {
    if (!p.is_trigger_electron) return 0;
    return M_PI + lab_phi(p);
}

// Calculate the invariant mass of the electron-nucleon interaction.
double W(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return sqrt(PRTMASS*PRTMASS + 2 * PRTMASS * nu(p, bE) - Q2(p, bE));
}
