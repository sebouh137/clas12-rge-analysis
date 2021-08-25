# Hipo Banks Index
Index of the relevant hipo banks to this project.
I find myself checking the hipo banks often enough to justify the existence of this file.

### REC::Particle
Reconstructed particle information.
Entries:

| Name    | Type  | Info                                                        |
|---------|-------|-------------------------------------------------------------|
| pid     | Int   | particle id in LUND conventions.                            |
| px      | Float | x component of the momentum (GeV).                          |
| py      | Float | y component of the momentum (GeV).                          |
| pz      | Float | z component of the momentum (GeV).                          |
| vx      | Float | x component of the vertex (cm).                             |
| vy      | Float | y component of the vertex (cm).                             |
| vz      | Float | z component of the vertex (cm).                             |
| vt      | Float | RF and z corrected vertex time (ns).                        |
| charge  | Byte  | particle charge.                                            |
| beta    | Float | particle beta measured by TOF.                              |
| chi2pid | Float | Chi2 of assigned PID.                                       |
| status  | Short | particle status (represents detector collection it passed). |
