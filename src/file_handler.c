#include "../lib/file_handler.h"

int get_run_no(char *input_file, int *run_no_int) {
    // TODO. This is a very brute way to find the run number and **should** be changed.
    char run_no_str[7];
    char *dot_pos = strrchr(input_file, '.');
    if (!dot_pos) return 0;
    strncpy(run_no_str, dot_pos - 6, 6);
    run_no_str[6] = '\0';
    *run_no_int = atoi(run_no_str);

    return *run_no_int;
}

int get_beam_energy(int run_no, double *beam_energy) {
    switch (run_no) { // NOTE. This should be a map in constants or taken directly from clas12mon.
        case 11983:  *beam_energy = BE11983;  break;
        case 12016:  *beam_energy = BE12016;  break;
        case 12439:  *beam_energy = BE12439;  break;
        case 999106: *beam_energy = BE999106; break;
        case 999110: *beam_energy = BE999110; break;
        case 999120: *beam_energy = BE999120; break;
        default:    return -1;
    }

    return 0;
}

// NOTE. Maybe this function could call extract_sf for the user if they have not done so already?
int get_sf_params(char *fname, double sf[NSECTORS][SF_NPARAMS][2]) {
    if (access(fname, F_OK) != 0) return 1;
    FILE *t_in = fopen(fname, "r");

    for (int si = 0; si < NSECTORS; ++si) {
        for (int ppi = 0; ppi < 2; ++ppi) {
            for (int pi = 0; pi < SF_NPARAMS; ++pi) {
                fscanf(t_in, "%lf ", &sf[si][pi][ppi]);
            }
        }
    }

    fclose(t_in);
    return 0;
}
