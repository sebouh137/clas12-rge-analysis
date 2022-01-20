#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "io_handler.h"

int main(int argc, char** argv) {
    bool   use_fmt     = false;
    int    nevents     = -1;
    char*  input_file  = NULL;
    int    run_no      = -1;
    double beam_energy = -1;

    switch (handle_args(argc, argv, &use_fmt, &nevents, &input_file, &run_no, &beam_energy)) {
        case 0:
            break;
        case 1:
            fprintf(stderr, "Usage: %s [-fn] [file]\n", argv[0]);
            return 1;
        case 2:
            fprintf(stderr, "Error. nevents should be a number greater than 0.\n");
            return 1;
        case 3:
            fprintf(stderr, "Error. input_file should be a hipo file. Currently is: %s.\n", input_file);
            free(input_file);
            return 1;
        case 4:
            fprintf(stderr, "Error. %s does not exist!\n", input_file);
            free(input_file);
            return 1;
        case 5:
            fprintf(stderr, "Error. Run number could not be extracted from input file %s.\n", input_file);
            free(input_file);
            return 1;
        case 6:
            fprintf(stderr, "Error. Run number %d not in database. Add from clas12mon.\n", run_no);
            free(input_file);
            return 1;
        default:
            fprintf(stderr, "Programmer Error. Uninplemented error code in handle_args!\n");
            return 1;
    }

    printf("use_fmt:    %d\n", use_fmt);
    printf("nevents:    %d\n", nevents);
    printf("input_file: %s\n", input_file);
    printf("run_no:     %d\n", run_no);

    // NOTE. Program goes here.

    free(input_file);
    return 0;
}
