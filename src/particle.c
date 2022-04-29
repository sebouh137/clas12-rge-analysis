#include "../lib/particle.h"

// TODO. Essentially all methods in this file require testing. Get to that.

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

    // Derived vars.
    switch (abs(pid)) {
        case 2212: p.mass = PRTMASS; break;
        case  321: p.mass = KMASS;   break;
        case  211: p.mass = PIMASS;  break;
        case 2112: p.mass = NTRMASS; break;
        case   11: p.mass = EMASS;   break;
        default:   p.mass = -1;
    }

    // NOTE. If programs gets slow, I should cache values of Q2, nu, etc here.

    return p;
}

// === PARTICLE FUNCTIONS ==========================================================================
// Get distance from beamline.
double d_from_beamline(particle p) {
    return (p.vx*p.vx + p.vy+p.vy);
}

// Calculate theta angle in the lab frame from momentum components of particle.
double theta_lab(particle p) {
    return p.px == 0.0 && p.py == 0.0 && p.pz == 0.0 ? 0.0 : atan2(sqrt(p.px*p.px + p.py*p.py), p.pz);
}

// Calculate phi angle in the lab frame from momentum components of particle.
double phi_lab(particle p) {
    return atan2(p.py, p.px);
}

// Calculate momentum magnitude from its components.
double P(particle p) {
    return calc_magnitude(p.px, p.py, p.pz);
}

// Calculate squared mass from momentum and beta.
double mass2(particle p) {
    return (P(p)*P(p)) / (p.beta*p.beta);
}

// === SIDIS e- FUNCTIONS ==========================================================================
// Calculate nu from beam energy and total momentum.
double nu(particle p, double bE) {
    if (!p.is_trigger_electron) return 0; // TODO. I need an invalid return value, not zero!
    return bE - P(p);
}

// Calculate Q^2 from beam energy, particle momentum, and theta angle.
double Q2(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return 4 * bE * P(p) * pow(sin(theta_lab(p)/2), 2);
}

// Calculate x_bjorken from beam energy, particle momentum, and theta angle.
double Xb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return Q2(p, bE) / (2*PRTMASS*nu(p, bE));
}

// Calculate y_bjorken from beam energy and nu.
double Yb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return nu(p, bE) / bE;
}

// Calculate the invariant mass of the electron-nucleon interaction.
double W(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return sqrt(W2(p, bE));
}
// TODO. Plot W without any Q2 cuts.
// TODO. Read a bit about resonances.

// Calculate the squared invariant mass of the electron-nucleon interaction.
double W2(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return PRTMASS*PRTMASS + 2*PRTMASS*nu(p, bE) - Q2(p, bE);
}

// NOTE. double s(particle p) ?

// Calculate the virtual photon's theta angle in the lab frame.
double theta_photon_lab(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return acos((bE - P(p)*cos(theta_lab(p))) / (sqrt(Q2(p, bE)+nu(p, bE)*nu(p, bE))));
}

// Calculate the virtual photon's phi angle in the lab frame.
double phi_photon_lab(particle p) {
    if (!p.is_trigger_electron) return 0;
    return M_PI + phi_lab(p);
}

// === SIDIS PRODUCED PARTICLE FUNCTIONS ===========================================================
// Compute the polar angle of a produced particle p with respect to the virtual photon direction.
// `p` is the produced particle while `e` is the trigger electron.
double theta_pq(particle p, particle e, double bE) {
    return calc_angle(-e.px, -e.py, bE-e.pz, p.px, p.py, p.pz);
}

// Compute the azimuthal angle of a produced particle p with respect to the virtual photon direction.
double phi_pq(particle p, particle e, double bE) {
    double gpx = -e.px, gpy = -e.py, gpz = bE-e.pz;
    double ppx = p.px,  ppy = p.py,  ppz = p.pz;

    double phi_z = M_PI - phi_lab(p);
    rotate_z(&gpx, &gpy, phi_z);
    rotate_z(&ppx, &ppy, phi_z);

    double phi_y = calc_angle(gpx, gpy, gpz, 0, 0, 1);
    rotate_y(&ppx, &ppz, phi_y);

    return atan2(ppy, ppx);
}

// Compute the cosine of the polar angle with respect to the virtual photon direction.
double cos_theta_pq(particle p, particle e, double bE) {
    return (p.pz*(bE-e.pz) - p.px*e.px - p.py*e.py) / (sqrt(nu(e,bE)*nu(e,bE) + Q2(e,bE)) * P(p));
}

// Return the squared momentum transverse to the virtual photon.
double Pt2(particle p, particle e, double bE) {
    return P(p) * P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the squared momentum longitudinal to the virtual photon.
double Pl2(particle p, particle e, double bE) {
    return P(p) * P(p) * cos_theta_pq(p,e,bE) * cos_theta_pq(p,e,bE);
}

// TODO. Pending description.
double zh(particle p, particle e, double bE) {
    return sqrt(p.mass*p.mass + P(p)*P(p)) / nu(e,bE);
}

// TODO. Pending description.
double PlCM(particle p, particle e, double bE) {
    return (nu(e,bE) + PRTMASS) * (sqrt(Pl2(p,e,bE)) - sqrt(Q2(e,bE) + nu(e,bE)*nu(e,bE))
            * zh(p,e,bE)*nu(e,bE) / (nu(e,bE) + PRTMASS)) / W(e,bE);
}

// TODO. Pending description.
double PmaxCM(particle p, particle e, double bE) {
    return sqrt(pow(W(e,bE)*W(e,bE) - NTRMASS*NTRMASS + PIMASS*PIMASS, 2)
            - 4*PIMASS*PIMASS*W(e,bE)*W(e,bE)) / (2*W(e,bE));
}

// Return the momentum transverse component squared of the produced particle wrt the virtual photon
//     direction.
double PTrans2PQ(particle p, particle e, double bE) {
    return P(p)*P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the momentum longitudinal component squared of the produced particle wrt the virtual
//     photon direction.
double PLong2PQ(particle p, particle e, double bE) {
    return P(p)*P(p) * cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE);
}
