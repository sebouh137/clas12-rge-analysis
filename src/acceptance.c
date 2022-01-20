#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "err_handler.h"
#include "io_handler.h"

int main(int argc, char **argv) {
    bool   use_fmt     = false;
    int    nevents     = -1;
    char   *input_file = NULL;
    int    run_no      = -1;
    double beam_energy = -1;

    if (handle_args_err(
            handle_args(argc, argv, &use_fmt, &nevents, &input_file, &run_no, &beam_energy),
            argv, input_file, run_no)
        ) return 1;

    printf("use_fmt:    %d\n", use_fmt);
    printf("nevents:    %d\n", nevents);
    printf("input_file: %s\n", input_file);
    printf("run_no:     %d\n", run_no);

    // NOTE. Program goes here.

    free(input_file);
    return 0;
}
