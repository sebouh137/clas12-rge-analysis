#ifndef IO_HANDLER
#define IO_HANDLER

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file_handler.h"

int handle_args(int argc, char **argv, bool *use_fmt, int *nevents, char **input_file, int *run_no,
                double *beam_energy);
int handle_filename(char *input_file, int *run_no, double *beam_energy);

#endif
