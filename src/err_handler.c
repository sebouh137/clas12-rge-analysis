#include "../lib/err_handler.h"

int draw_plots_usage() {
    fprintf(stderr, "TODO.\n");
    return 1;
}

int draw_plots_err(int errcode, char ** command, char ** cuts, char ** binning) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Programmer Error. draw_plots failed to compile the `plt_rx` "); fprintf(stderr, "regular expression. Check code.\n");
            break;
        case 2:
            fprintf(stderr, "Error. command `%s` matches more than one plt_str ", * command);
            fprintf(stderr, "regular expression.\n");
            break;
        case 3:
            fprintf(stderr, "Error. command `%s` does not match any plt_str ", * command);
            fprintf(stderr, "regular expression.\n");
            break;
        case 4:
            fprintf(stderr, "Programmer Error. draw_plots failed to compile the `var_rx` "); fprintf(stderr, "regular expression. Check code.\n");
            break;
        case 5:
            fprintf(stderr, "Error. command `%s` matches more than two var_str regular ", * command);
            fprintf(stderr, " expression.\n");
            break;
        case 6:
            fprintf(stderr, "Error. Number of variables inconsistent with detected plt_rx.\n");
            break;
        default:
            fprintf(stderr, "Programmer Error. Error code %d not implemented in ", errcode);
            fprintf(stderr, "draw_plots_err()! You're on your own.\n");
            return 1;
    }
    if (*command != NULL) free(* command);
    if (*cuts    != NULL) free(* cuts);
    if (*binning != NULL) free(* binning);
    return draw_plots_usage();
}

int draw_plots_handle_args_err(int errcode, char ** command, char ** cuts, char ** binning) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Error. No command provided.\n");
            if (*command != NULL) free(* command);
            if (*cuts    != NULL) free(* cuts);
            if (*binning != NULL) free(* binning);
            return draw_plots_usage();
        default:
            fprintf(stderr, "Programmer Error. Error code %d not implemented in ", errcode);
            fprintf(stderr, "draw_plots_handle_args()! You're on your own.\n");
            return 1;
    }
}

int make_ntuples_usage() {
    fprintf(stderr, "Usage: make_ntuples [-fd] [-n NEVENTS] file\n");
    fprintf(stderr, " * -f: Use FMT data. If unspecified, program will only use DC data.\n");
    fprintf(stderr, " * -d: Activate debug mode. Only use when programming new features.\n");
    fprintf(stderr, " * -n NEVENTS: Specify number of events to be processed with optarg.\n");
    fprintf(stderr, " * file: ROOT file to be processed. Expected file format is: `run_no.root`.\n");
    return 1;
}

int make_ntuples_handle_args_err(int errcode, char ** in_filename, int run_no) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            return make_ntuples_usage();
        case 2:
            fprintf(stderr, "Error. nevents should be a number greater than 0.\n");
            return make_ntuples_usage();
        case 3:
            fprintf(stderr, "Error. input file (%s) should be a root file.\n", * in_filename);
            free(* in_filename);
            return 1;
        case 4:
            fprintf(stderr, "Error. %s does not exist!\n", * in_filename);
            free(* in_filename);
            return 1;
        case 5:
            fprintf(stderr, "Error. Run number could not be extracted from %s.\n", * in_filename);
            free(* in_filename);
            return 1;
        case 6:
            fprintf(stderr, "Error. Run number %d not in database. Add from clas12mon.\n", run_no);
            free(* in_filename);
            return 1;
        case 7:
            fprintf(stderr, "Error. No file name provided.\n");
            return make_ntuples_usage();
        default:
            fprintf(stderr, "Programmer Error. Error code %d not implemented in ", errcode);
            fprintf(stderr, "make_ntuples_handle_args()! You're on your own.\n");
            return 1;
    }
}

int make_ntuples_err(int errcode, char ** in_filename) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Error. %s is not a valid ROOT file.\n", * in_filename);
            break;
        case 2:
            fprintf(stderr, "Error. Invalid EC layer. Check bank data or add layer to constants.\n");
            break;
        case 3:
            fprintf(stderr, "Error. A particle is in an invalid sector. Check bank integrity.\n");
        default:
            fprintf(stderr, "Programmer Error. Error code %d not implemented in \n", errcode);
            fprintf(stderr, "make_ntuples_err()! You're on your own.\n");
            break;
    }
    free(* in_filename);
    return 1;
}

int extractsf_usage() {
    fprintf(stderr, "Usage: extract_sf [-f] [-n NEVENTS] file\n");
    fprintf(stderr, " * -f: Use FMT data. If unspecified, program will only use DC data.\n");
    fprintf(stderr, " * -n NEVENTS: Specify number of events to be processed with optarg.\n");
    fprintf(stderr, " * file: ROOT file to be processed.\n");
    return 1;
}

int extractsf_err(int errcode, char ** in_filename) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Error. %s is not a valid ROOT file.\n", * in_filename);
            break;
        case 2:
            fprintf(stderr, "Error. Invalid EC layer. Check bank data or add layer to constants.\n");
            break;
        case 3:
            fprintf(stderr, "Error. A particle is in an invalid sector. Check bank integrity.\n");
            break;
        case 4:
            fprintf(stderr, "Error. Could not create sf_results file.\n");
            break;
        default:
            fprintf(stderr, "Programmer Error. Error code %d not implemented in \n", errcode);
            fprintf(stderr, "make_ntuples_err()! You're on your own.\n");
            break;
    }
    free(* in_filename);
    return 1;
}

int extractsf_handle_args_err(int errcode, char ** in_filename) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            return extractsf_usage();
        case 2:
            fprintf(stderr, "Error. nevents should be a number greater than 0.\n");
            return extractsf_usage();
        case 3:
            fprintf(stderr, "Error. input file (%s) should be a root file.\n", * in_filename);
            free(* in_filename);
            return 1;
        case 4:
            fprintf(stderr, "Error. %s does not exist!\n", * in_filename);
            free(* in_filename);
            return 1;
        case 5:
            fprintf(stderr, "Error. No file name provided.\n");
            return extractsf_usage();
        default:
            fprintf(stderr, "Programmer Error. Error code %d not implemented in ", errcode);
            fprintf(stderr, "extractsf_handle_args()! You're on your own.\n");
            return 1;
    }
}

int hipo2root_usage() {
    fprintf(stderr, "Usage: hipo2root filename\n");
    return 1;
}

int hipo2root_handle_args_err(int errcode, char ** in_filename) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Error. No file name provided.\n");
            return hipo2root_usage();
        case 2:
            fprintf(stderr, "Error. Too many arguments, only a file name is needed.\n");
            return hipo2root_usage();
        case 3:
            fprintf(stderr, "Error. input file (%s) should be a hipo file.\n", * in_filename);
            free(* in_filename);
            return 1;
        case 4:
            fprintf(stderr, "Error. %s does not exist!\n", * in_filename);
            free(* in_filename);
            return 1;
        default:
            fprintf(stderr, "Programmer Error. Error code %d not implemented in \n", errcode);
            fprintf(stderr, "hipo2root_handle_args()! You're on your own.\n");
            return 1;
    }
}
