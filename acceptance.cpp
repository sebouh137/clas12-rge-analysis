#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <map>
// #include <string>

int main(int argc, char** argv) {
    // Handle optional arguments.
    bool use_fmt = false;
    int  nevents = -1;

    int opt;
    while ((opt = getopt(argc, argv, "fn:")) != -1) {
        switch (opt) {
            case 'f': use_fmt = true;         break;
            case 'n': nevents = atoi(optarg); break;
            default:
                fprintf(stderr, "Usage: %s [-fn] [file]\n", argv[0]);
                return 1;
        }
    }
    if (nevents == 0) {
        fprintf(stderr, "ERROR. nevents should be a number greater than 0.\n");
        return 1;
    }

    // Handle positional argument.
    char* input_file = (char*) malloc(strlen(argv[argc - 1]) + 1);
    strcpy(input_file, argv[argc - 1]);

    if (!strstr(input_file, ".hipo")) {
        fprintf(stderr, "ERROR. input_file should be a hipo file. Currently is: %s.\n", input_file);
        return 1;
    }

    printf("use_fmt: %d\n", use_fmt);
    printf("nevents: %d\n", nevents);
    printf("input_file: %s\n", input_file);

    free(input_file);
    return 0;
}
