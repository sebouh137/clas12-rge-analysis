#include "../lib/io_handler.h"

int make_ntuples_handle_args(int argc, char ** argv, bool * use_simul,bool * use_fmt, bool * debug, int * nevents,
                           char ** input_file, int * run_no, double * beam_energy) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "fdn:s")) != -1) {
        switch (opt) {
            case 'f': * use_fmt   = true;         break;
            case 'd': * debug     = true;         break;
            case 'n': * nevents   = atoi(optarg); break;
            case 's': * use_simul = true;         break;
            default:  return 1; // Bad usage of optional arguments.
        }
    }
    if (* nevents == 0) return 2; // Check that nevents is valid and atoi performed correctly.

    // Handle positional argument.
    if (argc < 2) return 7;

    * input_file = (char *) malloc(strlen(argv[argc - 1]) + 1);
    strcpy(* input_file, argv[argc - 1]);

    int make_ntuples_handle_args_result = 0;
    if (!*use_simul){
        make_ntuples_handle_args_result = handle_root_filename_data(* input_file, run_no, beam_energy);
    } else{
        make_ntuples_handle_args_result = handle_root_filename_simul(* input_file, beam_energy);
    }
    return make_ntuples_handle_args_result;
}

int extractsf_handle_args(int argc, char ** argv, bool * use_fmt, int * nevents,
                          char ** input_file) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "fn:")) != -1) {
        switch (opt) {
            case 'f': * use_fmt = true;         break;
            case 'n': * nevents = atoi(optarg); break;
            default:  return 1;
        }
    }
    if (* nevents == 0) return 2;
    if (argc < 2) return 5;

    * input_file = (char *) malloc(strlen(argv[argc - 1]) + 1);
    strcpy(* input_file, argv[argc - 1]);
    return check_root_filename(* input_file);
}

int hipo2root_handle_args(int argc, char ** argv, char ** input_file, int * run_no, bool * use_simul) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "s")) != -1) {
        switch (opt) {
            case 's': * use_simul = true;         break;
            default:  return 1; // Bad usage of optional arguments.
        }
    }

    // Handle positional arguments
    if (argc < 2) return 1;
    if (argc > 3) return 2;

    * input_file = (char *) malloc(strlen(argv[argc - 1]) + 1);
    strcpy(* input_file, argv[argc - 1]);

    int handle_hipo_filename_result = 0;
    if(!*use_simul){
       handle_hipo_filename_result = handle_hipo_filename_data(* input_file, run_no);
    } else{
       handle_hipo_filename_result = handle_hipo_filename_simul(* input_file);
    }
    return handle_hipo_filename_result;
}

int check_root_filename(char * input_file) {
    if (!strstr(input_file, ".root"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_root_filename_data(char * input_file, int * run_no, double * beam_energy) {
    int chk = check_root_filename(input_file);
    if (chk) return chk;

    // Get run number and beam energy from filename.
    if (!get_run_no(input_file, run_no))       return 5;
    if (get_beam_energy(* run_no, beam_energy)) return 6;

    return 0;
}

int handle_root_filename_simul(char * input_file, double * beam_energy) {
    int chk = check_root_filename(input_file);
    if (chk) return chk;

    // Beam energy of simulation is fixed here!
    *beam_energy = 11.;
    return 0;
}

int check_hipo_filename(char * input_file) {
    if (!strstr(input_file, ".hipo"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_hipo_filename_data(char * input_file, int * run_no) {
    int chk = check_hipo_filename(input_file);
    if (chk) return chk;

    // Get run number from filename.
    if (!get_run_no(input_file, run_no)) return 5;

    return 0;
}
int handle_hipo_filename_simul(char * input_file) {
    int chk = check_hipo_filename(input_file);
    if (chk) return chk;

    return 0;
}
