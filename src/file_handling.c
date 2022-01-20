#include "file_handling.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_run_no(char* input_file) {
    char* run_no_str = (char*) malloc(7);
    strncpy(run_no_str, input_file + ((strchr(input_file, '.')) - input_file) - 6, 6);
    int run_no_int = atoi(run_no_str);
    free(run_no_str);

    return run_no_int;
}

double get_beam_energy(int run_no) {
    switch (run_no) { // TODO. This should be in constants file or taken directly from clas12mon.
        case 11983: return 10.3894;
        case 12016: return 10.3894;
        case 12439: return  2.1864;
        default:    return -1;
    }
}
