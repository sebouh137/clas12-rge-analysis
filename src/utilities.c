#include "utilities.h"

double to_deg(double radians) {
    return radians * (180.0 / M_PI);
}
double calc_theta(double px, double py, double pz) {
    return px == 0.0 && py == 0.0 && pz == 0.0 ? 0.0 : atan2(sqrt(px*px + py*py), pz);
}
double calc_phi(double px, double py) {
    return atan2(py, px);
}
double calc_P(double px, double py, double pz) {
    return sqrt(px*px + py*py + pz*pz);
}
double calc_Q2(double beam_E, double momentum, double theta) {
    return 4 * beam_E * momentum * pow(sin(theta/2), 2);
}
double calc_nu(double beam_E, double momentum) {
    return beam_E - momentum;
}
double calc_Xb(double beam_E, double momentum, double theta) {
    return (calc_Q2(beam_E, momentum, theta)/2) / (calc_nu(beam_E, momentum)/PRTMASS);
}
int insert_TH1F(std::map<const char *, TH1 *> * map, const char * k, const char * n, const char * xn,
               int bins, double min, double max) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH1F(Form("%s: %s", k, n), Form("%s;%s", n, xn), bins, min, max)));
    return 0;
}
int insert_TH2F(std::map<const char *, TH1 *> * map, const char * k,
                const char * n, const char * nx, const char * ny,
                int xbins, double xmin, double xmax, int ybins, double ymin, double ymax) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH2F(Form("%s: %s", k, n), Form("%s;%s;%s", n, nx, ny),
                         xbins, xmin, xmax, ybins, ymin, ymax)));
    return 0;
}
