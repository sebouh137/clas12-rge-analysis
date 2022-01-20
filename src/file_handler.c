#include "file_handler.h"

int get_run_no(char* input_file, int *run_no_int) {
    char *run_no_str = (char*) malloc(7);
    strncpy(run_no_str, input_file + ((strchr(input_file, '.')) - input_file) - 6, 6);
    *run_no_int = atoi(run_no_str);
    free(run_no_str);

    return *run_no_int;
}

int get_beam_energy(int run_no, double *beam_energy) {
    switch (run_no) { // NOTE. This should be in constants file or taken directly from clas12mon.
        case 11983: *beam_energy = 10.3894; break;
        case 12016: *beam_energy = 10.3894; break;
        case 12439: *beam_energy =  2.1864; break;
        default:    return -1;
    }

    return 0;
}
