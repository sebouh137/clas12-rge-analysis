#include "utilities.h"

// Pass from radians to degrees.
double to_deg(double radians) {
    return radians * (180.0 / M_PI);
}

// Calculate theta angle from momentum components of particle.
double calc_theta(double px, double py, double pz) {
    return px == 0.0 && py == 0.0 && pz == 0.0 ? 0.0 : atan2(sqrt(px*px + py*py), pz);
}

// Calculate phi angle from momentum components of particle.
double calc_phi(double px, double py) {
    return atan2(py, px);
}

// Calculate momentum magnitude from its components.
double calc_P(double px, double py, double pz) {
    return sqrt(px*px + py*py + pz*pz);
}

// Calculate Q^2 from beam energy, particle momentum, and theta angle.
double calc_Q2(double beam_E, double momentum, double theta) {
    return 4 * beam_E * momentum * pow(sin(theta/2), 2);
}

// Calculate nu from beam energy and total momentum.
double calc_nu(double beam_E, double momentum) {
    return beam_E - momentum;
}

// Calculate x_bjorken from beam energy, particle momentum, and theta angle.
double calc_Xb(double beam_E, double momentum, double theta) {
    return (calc_Q2(beam_E, momentum, theta)/2) / (calc_nu(beam_E, momentum)/PRTMASS);
}

// Insert a 1-dimensional histogram of floating point numbers into a map.
int insert_TH1F(std::map<const char *, TH1 *> * map, const char * k, const char * n, const char * xn,
               int bins, double min, double max) {
    // printf("\nINSERTING TH1F WITH NAME: `%s`\n", n);
    // printf("  TH1F is named:   `%s: %s`\n", k, n);
    // printf("  Title is:        `%s`\n", n);
    // printf("  x axis is named: `%s`\n", xn);
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH1F(Form("%s: %s", k, n), Form("%s;%s", n, xn), bins, min, max)));
    return 0;
}

// Insert a 2-dimensional histogram of floating point numbers into a map.
int insert_TH2F(std::map<const char *, TH1 *> * map, const char * k,
                const char * n, const char * nx, const char * ny,
                int xbins, double xmin, double xmax, int ybins, double ymin, double ymax) {
    // printf("\nINSERTING TH2F WITH NAME: `%s`\n", n);
    // printf("  TH2F is named:   `%s: %s`\n", k, n);
    // printf("  Title is:        `%s`\n", n);
    // printf("  x axis is named: `%s`\n", xn);
    // printf("  y axis is named: `%s`\n", yn);
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH2F(Form("%s: %s", k, n), Form("%s;%s;%s", n, nx, ny),
                         xbins, xmin, xmax, ybins, ymin, ymax)));
    return 0;
}
