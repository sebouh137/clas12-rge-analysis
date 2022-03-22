# Acceptance Study
Usage instructions.
* Set the environment variable `HIPO` to the location where hipo is installed.
* Compile `hipo2root` by running `make` inside the `hipo2root` directory.

NOTE.
To run with valgrind, ROOT requires some flags:

    valgrind --num-callers=30 --suppressions=$ROOTSYS/etc/valgrind-root.supp

Not sure if I'm using hipo wrong or if it has a ton of memory leaks. Gotta check this issue at some point.
