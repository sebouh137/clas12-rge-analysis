# Acceptance Study
Usage instructions.
* Set the environment variable `HIPO` to the location where hipo is installed.
* Compile `hipo2root` by running `make` inside the `hipo2root` directory.

**NOTE**.
To run with valgrind, ROOT requires some flags:

    valgrind --num-callers=30 --suppressions=$ROOTSYS/etc/valgrind-root.supp

Not sure if I'm using ROOT and hipo wrong or if they have a ton of memory leaks.
Gotta check this issue at some point.

## License
This program is licensed under the GNU LGPLv3 license. Please check the LICENSE file for details.
