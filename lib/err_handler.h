#ifndef ERR_HANDLER
#define ERR_HANDLER

#include <stdio.h>
#include <stdlib.h>

int acceptance_usage();
int acceptance_handle_args_err(int errcode, char **in_filename, int run_no);
int acceptance_err(int errcode, char **in_filename);
int extractsf_usage();
int extractsf_handle_args_err(int errcode, char **in_filename);
int extractsf_err(int errcode, char **in_filename);
int hipo2root_usage();
int hipo2root_handle_args_err(int errcode, char **in_filename);

#endif
