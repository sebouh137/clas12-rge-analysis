#include "../lib/particle.h"

// TODO. Essentially all methods in this file require testing. Get to that.

// Initialize an empty particle.
particle particle_init() {
    particle p;
    p.is_valid = false;
    return p;
}

// Initialize a new particle.
particle particle_init(int pid, int charge, double beta, int status, int sector,
                       double vx, double vy, double vz, double px, double py, double pz) {
    particle p;

    // Inherent vars.
    p.is_valid = true;
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

    // Derived vars. TODO. We might need more PIDs.
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

// Initialize a new particle from the particle and track banks.
particle particle_init(REC_Particle * rp, REC_Track * rt, int pos) {
    int pindex = rt->pindex->at(pos); // pindex is always equal to pos!

    return particle_init(rp->pid->at(pindex), rp->charge->at(pindex), rp->beta->at(pindex),
                         rp->status->at(pindex), rt->sector->at(pos),
                         rp->vx->at(pindex), rp->vy->at(pindex), rp->vz->at(pindex),
                         rp->px->at(pindex), rp->py->at(pindex), rp->pz->at(pindex));
}

// Initialize a new particle from the particle, tracks, and FMT banks.
particle particle_init(REC_Particle * rp, REC_Track * rt, FMT_Tracks * ft, int pos) {
    int index  = rt->index->at(pos);
    int pindex = rt->pindex->at(pos); // pindex is always equal to pos!

    // Apply FMT cuts.
    if (ft->vz->size() < 1)               return particle_init(); // Track reconstructed by FMT.
    if (ft->ndf->at(index) < FMTNLYRSCUT) return particle_init(); // Track crossed 3 FMT layers.

    return particle_init(rp->pid->at(pindex), rp->charge->at(pindex), rp->beta->at(pindex),
                         rp->status->at(pindex), rt->sector->at(pos),
                         ft->vx->at(index), ft->vy->at(index), ft->vz->at(index),
                         ft->px->at(index), ft->py->at(index), ft->pz->at(index));
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
    return sqrt(abs(W2(p, bE)));
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
// TODO. Many of the methods hereafter assume the particle is a pion. This probably needs to be
//       fixed for CLAS12.

// Compute the polar angle of a produced particle p with respect to the virtual photon direction.
// `p` is the produced particle while `e` is the trigger electron.
double theta_pq(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return calc_angle(-e.px, -e.py, bE-e.pz, p.px, p.py, p.pz);
}

// Compute the azimuthal angle of a produced particle p with respect to the virtual photon direction.
double phi_pq(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;

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
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return (p.pz*(bE-e.pz) - p.px*e.px - p.py*e.py) / (sqrt(nu(e,bE)*nu(e,bE) + Q2(e,bE)) * P(p));
}

// Return the squared momentum transverse to the virtual photon.
double Pt2(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p) * P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the squared momentum longitudinal to the virtual photon.
double Pl2(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p) * P(p) * cos_theta_pq(p,e,bE) * cos_theta_pq(p,e,bE);
}

// Obtain the fraction of the virtual photon energy taken by the produced particle in the lab frame.
double zh(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return sqrt(p.mass*p.mass + P(p)*P(p)) / nu(e,bE);
}

// Return the longitudinal momentum in the center of mass frame.
double PlCM(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return (nu(e,bE) + PRTMASS) * (sqrt(Pl2(p,e,bE)) - sqrt(Q2(e,bE) + nu(e,bE)*nu(e,bE))
            * zh(p,e,bE)*nu(e,bE) / (nu(e,bE) + PRTMASS)) / W(e,bE);
}

// Obtain the maximum possible value that the momentum could've had in the center of mass frame.
double PmaxCM(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return sqrt(pow(W(e,bE)*W(e,bE) - NTRMASS*NTRMASS + PIMASS*PIMASS, 2)
            - 4*PIMASS*PIMASS*W(e,bE)*W(e,bE)) / (2*W(e,bE));
}

// Return the momentum transverse component squared of the produced particle wrt the virtual photon
//     direction.
double PTrans2PQ(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p)*P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the momentum longitudinal component squared of the produced particle wrt the virtual
//     photon direction.
double PLong2PQ(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p)*P(p) * cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE);
}

// Calculate X_f (X Feynmann).
double Xf(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return PlCM(p,e,bE) / PmaxCM(p,e,bE);
}

// Compute the missing mass
double Mx2(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return W(e,bE)*W(e,bE) - 2*nu(e,bE)*zh(p,e,bE) * (nu(e,bE) + PRTMASS) + PIMASS*PIMASS
            + 2*sqrt((Q2(e,bE) + nu(e,bE)*nu(e,bE)) * Pl2(p,e,bE));
}

// Compute Mandelstam t. TODO. Make sure that that is what this is!
double t_mandelstam(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return 2*sqrt((nu(e,bE)*nu(e,bE) + Q2(e,bE)) * Pl2(p,e,bE)) + PIMASS*PIMASS - Q2(e,bE)
            - 2*nu(e,bE)*nu(e,bE)*zh(p,e,bE);
}
