#ifndef ERR_HANDLER
#define ERR_HANDLER

#include <stdio.h>
#include <stdlib.h>

int acceptance_handle_args_err(int errcode, char **in_filename, int run_no);
int hipo2root_handle_args_err(int errcode, char **in_filename);
int acceptance_err(int errcode, char **in_filename);

#endif
