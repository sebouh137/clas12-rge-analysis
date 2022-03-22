#include "io_handler.h"

int acceptance_handle_args(int argc, char **argv, bool *use_fmt, int *nevents, char **input_file,
                           int *run_no, double *beam_energy) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "fn:")) != -1) {
        switch (opt) {
            case 'f': *use_fmt = true;         break;
            case 'n': *nevents = atoi(optarg); break;
            default:  return 1; // Bad usage of optional arguments.
        }
    }
    if (*nevents == 0) return 2; // Check that nevents is valid and atoi performed correctly.

    // Handle positional argument.
    if (argc < 2) return 7;

    *input_file = (char*) malloc(strlen(argv[argc - 1]) + 1);
    strcpy(*input_file, argv[argc - 1]);
    return handle_root_filename(*input_file, run_no, beam_energy);
}

int hipo2root_handle_args(int argc, char **argv, char **input_file, int *run_no) {
    if (argc < 2) return 1;
    if (argc > 3) return 2;

    *input_file = (char*) malloc(strlen(argv[1]) + 1);
    strcpy(*input_file, argv[1]);
    return handle_hipo_filename(*input_file, run_no);
}

int handle_root_filename(char *input_file, int *run_no, double *beam_energy) {
    if (!strstr(input_file, ".root"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.

    // Get run number and beam energy from filename.
    if (!get_run_no(input_file, run_no))       return 5;
    if (get_beam_energy(*run_no, beam_energy)) return 6;

    return 0;
}

int handle_hipo_filename(char *input_file, int *run_no) {
    if (!strstr(input_file, ".hipo"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.

    // Get run number from filename.
    if (!get_run_no(input_file, run_no)) return 5;

    return 0;
}
