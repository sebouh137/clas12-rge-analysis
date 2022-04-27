#include "utilities.h"

// Pass from radians to degrees.
double to_deg(double radians) {
    return radians * (180.0 / M_PI);
}

double calc_P(double px, double py, double pz) {
    return sqrt(px*px + py*py + pz*pz);
}

// Insert a 1-dimensional histogram of floating point numbers into a map.
int insert_TH1F(std::map<const char *, TH1 *> * map, const char * k, const char * n, const char * xn,
               int bins, double min, double max) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH1F(Form("%s: %s", k, n), Form("%s;%s", n, xn), bins, min, max)));
    return 0;
}

// Insert a 2-dimensional histogram of floating point numbers into a map.
int insert_TH2F(std::map<const char *, TH1 *> * map, const char * k,
                const char * n, const char * nx, const char * ny,
                int xbins, double xmin, double xmax, int ybins, double ymin, double ymax) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH2F(Form("%s: %s", k, n), Form("%s;%s;%s", n, nx, ny),
                         xbins, xmin, xmax, ybins, ymin, ymax)));
    return 0;
}
