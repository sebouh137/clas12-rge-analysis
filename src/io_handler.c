#include "../lib/io_handler.h"

int make_ntuples_handle_args(int argc, char ** argv, bool * debug, int * nevents,
                             bool * use_simul, char ** input_file, int * run_no, double * beam_energy) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-dn:s")) != -1) {
        switch (opt) {
            case 'd': * debug     = true;         break;
            case 'n': * nevents   = atoi(optarg); break;
            case 's': * use_simul = true;         break;
            case  1 :{
                * input_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(* input_file, optarg);
                break;
            }
            default:  return 1; // Bad usage of optional arguments.
        }
    }
    if (* nevents == 0) return 2; // Check that nevents is valid and atoi performed correctly.

    // Handle positional argument.
    if (argc < 2) return 7;

    return handle_root_filename(* input_file, run_no, beam_energy, use_simul);
}

int extractsf_handle_args(int argc, char ** argv, bool * use_fmt, int * nevents,
                          char ** input_file, int * run_no, bool * use_simul) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-fn:s")) != -1) {
        switch (opt) {
            case 'f': * use_fmt = true;         break;
            case 'n': * nevents = atoi(optarg); break;
            case 's': * use_simul = true;       break;
            case  1 :{
                * input_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(* input_file, optarg);
                break;
            }
            default:  return 1;
        }
    }
    if (* nevents == 0) return 2;
    if (argc < 2) return 5;

    return handle_root_filename(* input_file, run_no, use_simul);
}

int hipo2root_handle_args(int argc, char ** argv, char ** input_file, int * run_no, bool * use_simul) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-s")) != -1) {
        switch (opt) {
            case 's': * use_simul = true;         break;
            case  1 :{
                * input_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(* input_file, optarg);
                break;
            }
            default:  return 1; // Bad usage of optional arguments.
        }
    }

    // Handle positional arguments
    if (argc < 2) return 1;
    if (argc > 3) return 2;

    return handle_hipo_filename(* input_file, run_no, use_simul);
}

int check_root_filename(char * input_file) {
    if (!strstr(input_file, ".root"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_root_filename(char * input_file, int * run_no, bool * use_simul) {
    double dump = 0.;
    return handle_root_filename(input_file, run_no, &dump, use_simul);
}

int handle_root_filename(char * input_file, int * run_no, double * beam_energy, bool * use_simul) {
    int chk = check_root_filename(input_file);
    if (chk) return chk;
    // Get run number and beam energy from filename.
    if (!get_run_no(input_file, run_no, use_simul))  return 5;
    if (get_beam_energy(* run_no, beam_energy))      return 6;
    
    return 0;
}

int check_hipo_filename(char * input_file) {
    if (!strstr(input_file, ".hipo"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_hipo_filename(char * input_file, int * run_no, bool * use_simul) {
    int chk = check_hipo_filename(input_file);
    if (chk) return chk;

    // Guarding statement.
    if(*use_simul) {*run_no = 0; return 0;}

    // Get run number from filename.
    if (!get_run_no(input_file, run_no)) return 5;

    return 0;
}
