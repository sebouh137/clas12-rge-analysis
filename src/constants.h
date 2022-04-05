#ifndef CONSTANTS
#define CONSTANTS

// Masses.
#define PIMASS  0.139570 // Pion mass.
#define PRTMASS 0.938272 // Proton mass.
#define NTRMASS 0.939565 // Neutron mass.
#define EMASS   0.000051 // Electron mass.

// Particle map keys.
#define PALL "All particles"
#define PPOS "Positive particles"
#define PNEG "Negative particles"
#define PPIP "Pi+"
#define PPIM "Pi-"
#define PELC "e-"
#define PTRE "Trigger e-"

// Histogram map keys.
#define VZ       "Vz"
#define PHI      "#phi"
#define VZPHI    "Vz vs #phi"
#define THETA    "#theta"
#define VZTHETA  "Vz vs #theta"

#define VP       "Vp"
#define BETA     "#beta"
#define VPBETA   "Vp vs #beta"

#define DTOF     "#Delta TOF"
#define VPTOF    "Vp vs #Delta TOF"

#define E        "E"
#define EDIVP    "E/Vp"
#define PEDIVP   "Vp vs E/Vp"
#define EEDIVP   "E vs E/Vp"
#define PPCALE   "Vp vs E (PCAL)"
#define PECINE   "Vp vs E (ECIN)"
#define PECOUE   "Vp vs E (ECOU)"
#define ECALPCAL "E (ECAL) vs E (PCAL)"

// Sampling fraction.
extern const char *SFARR[3];
#define SF_PMIN  1.0 // GeV
#define SF_PMAX  9.0 // GeV
#define SF_PSTEP 0.4 // GeV

#define Q2       "Q^{2}"
#define NU       "#nu"
#define XB       "X_{bjorken}"

// RUN CONSTANTS (TODO. these should be in a map.)
#define BE11983 10.3894 //  50 nA.
#define BE12016 10.3894 // 250 nA.
#define BE12439  2.1864 //  15 nA.

// DETECTOR CONSTANTS
#define PCAL_LYR 1 // PCAL's layer id.
#define ECIN_LYR 4 // EC inner layer id.
#define ECOU_LYR 7 // EC outer layer id.

#endif
