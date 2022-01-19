#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <map>
// #include <string>

int get_run_no(char* input_file) {
    char* run_no_str = (char*) malloc(7);
    strncpy(run_no_str, input_file + ((strchr(input_file, '.')) - input_file) - 6, 6);
    int run_no_int = atoi(run_no_str);
    free(run_no_str);

    return run_no_int;
}

double get_beam_energy(int run_no) {
    double beam_energy = -1;
    if (run_no == 0) { // Check that run number was extracted correctly.
        fprintf(stderr, "ERROR. Run number could not be extracted from input file %s.", input_file);
        return 1;
    }
    switch (run_no) {
        case 11983: beam_energy = 10.3894; break;
        case 12016: beam_energy = 10.3894; break;
        case 12439: beam_energy =  2.1864; break;
        default:
            fprintf(stderr, "ERROR. Run number %d not in database. Add from clas12mon.\n", run_no);
            return -1;
    }

    return beam_energy;
}

int main(int argc, char** argv) {
    // Handle optional arguments.
    bool use_fmt = false;
    int  nevents = -1;

    int opt;
    while ((opt = getopt(argc, argv, "fn:")) != -1) {
        switch (opt) {
            case 'f': use_fmt = true;         break;
            case 'n': nevents = atoi(optarg); break;
            default:
                fprintf(stderr, "Usage: %s [-fn] [file]\n", argv[0]);
                return 1;
        }
    }
    if (nevents == 0) {
        fprintf(stderr, "ERROR. nevents should be a number greater than 0.\n");
        return 1;
    }

    // Handle positional argument.
    char* input_file = (char*) malloc(strlen(argv[argc - 1]) + 1);
    strcpy(input_file, argv[argc - 1]);

    if (!strstr(input_file, ".hipo")) { // Check that file is valid.
        fprintf(stderr, "ERROR. input_file should be a hipo file. Currently is: %s.\n", input_file);
        return 1;
    }
    if (!access(input_file, F_OK) == 0) { // Check that file exists.
        fprintf(stderr, "ERROR. %s does not exist!\n", input_file);
        return 1;
    }

    // Get beam energy from run number.
    int run_no = get_run_no(input_file);
    if (run_no == 0) { // Check that run number was extracted correctly.
        fprintf(stderr, "ERROR. Run number could not be extracted from input file %s.", input_file);
        return 1;
    }
    double beam_energy = get_beam_energy(run_no);
    if (beam_energy == -1) {
        fprintf(stderr, "ERROR. Run number %d not in database. Add from clas12mon.\n", run_no);
        return 1;
    }

    printf("use_fmt:    %d\n", use_fmt);
    printf("nevents:    %d\n", nevents);
    printf("input_file: %s\n", input_file);
    printf("run_no:     %d\n", run_no);

    free(input_file);
    return 0;
}
