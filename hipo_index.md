# Hipo Banks Index
Index of the relevant hipo banks to this project.
I find myself checking the hipo banks often enough to justify the existence of this file.

Beam energies and currents (from clas12mon) are:
* 11983: 10389.4 MeV,  50 nA
* 12016: 10389.4 MeV, 250 nA
* 12439:  2186.4 MeV,  15 nA

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

### REC::Event
Event Header Bank.
Entries:

| Name        | Type   | Info                                                             |
|-------------|--------|------------------------------------------------------------------|
| category    | Long   | Undefined.                                                       |
| topology    | Long   | Undefined.                                                       |
| beamCharge  | Float  | Beam charge, gated (nano-Coulomb).                               |
| liveTime    | Double | Lifetime.                                                        |
| startTime   | Float  | Event Start Time (ns).                                           |
| RFTime      | Float  | RF Time (ns).                                                    |
| helicity    | Byte   | Helicity of Event (0 or 1, else undefined), with HWP-correction. |
| helicityRaw | Byte   | Helicity of Event (0 or 1, else undefined).                      |
| procTime    | Float  | Event Processing Time (UNIX Time = seconds).                     |

### FMT::Tracks
Reconstructed tracks using FMT information.
Entries:

| Name   | Type  | Info                                                                           |
|--------|-------|--------------------------------------------------------------------------------|
| index  | Short | index of the track in the DC bank.                                             |
| status | Byte  | status of the track (0: not refit using FMT, 1: refit using FMT).              |
| sector | Byte  | sector of the track in DC.                                                     |
| Vtx0_x | Float | Vertex x-position of the swam track to the DOCA to the beamline (in cm).       |
| Vtx0_y | Float | Vertex y-position of the swam track to the DOCA to the beamline (in cm).       |
| Vtx0_z | Float | Vertex z-position of the swam track to the DOCA to the beamline (in cm).       |
| p0_x   | Float | 3-momentum x-coordinate of the swam track to the DOCA to the beamline (in cm). |
| p0_y   | Float | 3-momentum y-coordinate of the swam track to the DOCA to the beamline (in cm). |
| p0_z   | Float | 3-momentum z-coordinate of the swam track to the DOCA to the beamline (in cm). |
| q      | Byte  | charge of the track.                                                           |
| chi2   | Float | chi^2 of the fit.                                                              |
| NDF    | Byte  | number of degrees of freedom of the fit.                                       |
