#include "err_handler.h"

int handle_args_err(int errcode, char **in_filename, int run_no) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Usage: main [-fn] [file]\n");
            return 1;
        case 2:
            fprintf(stderr, "Error. nevents should be a number greater than 0.\n");
            return 1;
        case 3:
            fprintf(stderr, "Error. input file (%s) should be a root file.\n", *in_filename);
            free(*in_filename);
            return 1;
        case 4:
            fprintf(stderr, "Error. %s does not exist!\n", *in_filename);
            free(*in_filename);
            return 1;
        case 5:
            fprintf(stderr, "Error. Run number could not be extracted from %s.\n", *in_filename);
            free(*in_filename);
            return 1;
        case 6:
            fprintf(stderr, "Error. Run number %d not in database. Add from clas12mon.\n", run_no);
            free(in_filename);
            return 1;
        default:
            fprintf(stderr, "Programmer Error. Uninplemented error code in handle_args()!\n");
            free(in_filename);
            return 1;
    }
}
