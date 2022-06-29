#ifndef IO_HANDLER
#define IO_HANDLER

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "file_handler.h"

int make_ntuples_handle_args(int argc, char ** argv, bool * use_simul, bool * use_fmt, bool * debug, int * nevents,
                             char ** input_file, int * run_no, double * beam_energy);
int extractsf_handle_args(int argc, char ** argv, bool * use_fmt, int * nevents,
                          char ** input_file);
int hipo2root_handle_args(int argc, char ** argv, char ** input_file, int * run_no, bool * use_simul);

int check_root_filename(char * input_file);
int handle_root_filename_data(char * input_file, int * run_no, double * beam_energy);
int handle_root_filename_simul(char * input_file, double * beam_energy);

int check_hipo_filename(char * input_file);
int handle_hipo_filename_data(char * input_file, int * run_no);
int handle_hipo_filename_simul(char * input_file);

#endif
