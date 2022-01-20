#include "err_handler.h"

int handle_args_err(int errcode, char **argv, char **input_file, int run_no) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Usage: %s [-fn] [file]\n", argv[0]);
            return 1;
        case 2:
            fprintf(stderr, "Error. nevents should be a number greater than 0.\n");
            return 1;
        case 3:
            fprintf(stderr, "Error. input file (%s) should be a hipo file.\n", *input_file);
            free(*input_file);
            return 1;
        case 4:
            fprintf(stderr, "Error. %s does not exist!\n", *input_file);
            free(*input_file);
            return 1;
        case 5:
            fprintf(stderr, "Error. Run number could not be extracted from %s.\n", *input_file);
            free(*input_file);
            return 1;
        case 6:
            fprintf(stderr, "Error. Run number %d not in database. Add from clas12mon.\n", run_no);
            free(input_file);
            return 1;
        default:
            fprintf(stderr, "Programmer Error. Uninplemented error code in handle_args()!\n");
            free(input_file);
            return 1;
    }
}
