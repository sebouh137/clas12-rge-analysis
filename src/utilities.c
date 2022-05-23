#include "../lib/utilities.h"

// Pass from radians to degrees.
double to_deg(double radians) {
    return radians * (180.0 / M_PI);
}

// Compute a vector's magnitude from its components.
double calc_magnitude(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

// Get angle between two vectors.
double calc_angle(double x1, double y1, double z1, double x2, double y2, double z2) {
    return acos((x1*x2 + y1*y2 + z1*z2)/(calc_magnitude(x1,y1,z1) * calc_magnitude(x2,y2,z2)));
}

// Rotate a vector around the y axis by theta.
void rotate_y(double *x, double *z, double th) {
    double x_prev = *x;
    double z_prev = *z;
    *x =  x_prev*cos(th) + z_prev*sin(th);
    *z = -x_prev*sin(th) + z_prev*cos(th);
}

// Rotate a vector around the z axis by theta.
void rotate_z(double *x, double *y, double th) {
    double x_prev = *x;
    double y_prev = *y;
    *x = x_prev*cos(th) - y_prev*sin(th);
    *y = x_prev*sin(th) + y_prev*cos(th);
}

// Catch a string within a list.
int catch_string(const char * list[], int size) {
    double x;
    while (true) {
        char str[32];
        printf(">>> ");
        scanf("%31s", str);

        for (int i = 0; i < size; ++i) if (!strcmp(str, list[i])) x = i;
        if (x != -1) break;
    }

    return x;
}

// Catch a long value from stdin.
long catch_long() {
    long r;
    while (true) {
        char str[32];
        char * endptr;
        printf(">>> ");
        scanf("%31s", str);
        r = strtol(str, &endptr, 10);

        if (endptr != str) break;
    }

    return r;
}

// Catch a double value from stdin.
double catch_double() {
    double r;
    while (true) {
        char str[32];
        char * endptr;
        printf(">>> ");
        scanf("%31s", str);
        r = strtod(str, &endptr);

        if (endptr != str) break;
    }

    return r;
}

// Insert a 1-dimensional histogram of floating point numbers into a map.
int insert_TH1F(std::map<const char *, TH1 *> *map, const char *k, const char *n, const char *xn,
               int bins, double min, double max) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH1F(Form("%s: %s", k, n), Form("%s;%s", n, xn), bins, min, max)));
    return 0;
}

// Insert a 2-dimensional histogram of floating point numbers into a map.
int insert_TH2F(std::map<const char *, TH1 *> *map, const char *k,
                const char *n, const char *nx, const char *ny,
                int xbins, double xmin, double xmax, int ybins, double ymin, double ymax) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH2F(Form("%s: %s", k, n), Form("%s;%s;%s", n, nx, ny),
                         xbins, xmin, xmax, ybins, ymin, ymax)));
    return 0;
}
