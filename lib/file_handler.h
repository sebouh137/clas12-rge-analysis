#ifndef FILE_HANDLER
#define FILE_HANDLER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"

int get_run_no(char *input_file, int *run_no_int);
int get_beam_energy(int run_no, double *beam_energy);

#endif
