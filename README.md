# Acceptance Study
Usage instructions.
* Set the environment variable `HIPO` to the location where hipo is installed.
* Compile `hipo2root` by running `make` inside the `hipo2root` directory.

**NOTE**.
To run with valgrind, ROOT requires some flags:

    valgrind --num-callers=30 --suppressions=$ROOTSYS/etc/valgrind-root.supp

Not sure if I'm using ROOT and hipo wrong or if they have a ton of memory leaks.
Gotta check this issue at some point.

**Use for Simulations**
This software follows a run-number convention for the name of input files. Therefore, every HIPO file used as input has to have its run-number specified just before the extension.
For simulations, use the following type of run-number:
>999X
Where *X* is the number obtained after the energy used in the simulation is multiplied by ten. At the moment, there are three run-numbers available for simulations:
- 999106
- 999110
- 999120

## License
This program is licensed under the GNU LGPLv3 license. Please check the LICENSE file for details.
