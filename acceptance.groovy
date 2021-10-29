import java.util.HashMap;
import java.util.Map;

import org.jlab.clas.physics.Particle;
import org.jlab.detector.calib.utils.DatabaseConstantProvider;
import org.jlab.groot.base.GStyle;
import org.jlab.groot.data.H1F;
import org.jlab.groot.data.H2F;
import org.jlab.groot.data.IDataSet;
import org.jlab.groot.data.TDirectory;
import org.jlab.groot.fitter.DataFitter;
import org.jlab.groot.math.F1D;
import org.jlab.groot.fitter.DataFitter;
import org.jlab.io.base.DataBank;
import org.jlab.io.base.DataEvent;
import org.jlab.io.hipo.*;

Map<Character, String> params = new HashMap<>();
if (IO_handler.parse_args(args, params)) System.exit(1);

String infile = params.get('f');
int n_events  = (params.get('n') == null) ? 10000000 : Integer.parseInt(params.get('n'));
boolean FMT = false;
if (params.get('d') != null && params.get('d').toLowerCase().equals("fmt")) FMT = true;

// Define run data. It would be sweet to ask this to clas12mon, but is not an urgent necesity atm.
// All beam energies are in GeV.
double beam_energy;
switch (IO_handler.get_runno(infile)) { // TODO. Move to Constants I guess.
    case 11983:
        beam_energy = 10.3894;
        break;
    case 12016:
        beam_energy = 10.3894;
        break;
    case 12439:
        beam_energy =  2.1864;
        break;
    default:
        System.err.printf("Run number not in database. Add from clas12mon.\n");
        System.exit(1);
}

// Initial setup.
// NOTE. names and DataGroups should be kept in a struct (or object...).
setup_groot();
Constants C = new Constants();
Map<String, Map<String, Map<String, IDataSet>>> hists =
        new HashMap<String, HashMap<String, HashMap<String, IDataSet>>>();

for (int ki = 0; ki < C.K1ARR.length; ++ki) {
    hists.put(C.K1ARR[ki], new HashMap<String, HashMap<String, IDataSet>>());
    gen_map(C, hists.get(C.K1ARR[ki]), ki);
}

// TODO. TEMPORARY.
double[] p1 = [ 0.25149,  0.25186,  0.24912,  0.24747,  0.24649,  0.25409];
double[] p2 = [ 1.00000,  1.00000,  1.00000,  1.00000,  1.00000,  1.00000];
double[] p3 = [-0.03427, -0.03771, -0.02627, -0.03163, -0.02723, -0.04000];
double[] p4 = [ 0.00070,  0.00070,  0.00070,  0.00070,  0.00070,  0.00070];

// Loop through events.
int i_event      = -1;
int n_DC_tracks  = 0;
int n_FMT_tracks = 0;
HipoDataSource reader = new HipoDataSource();
reader.open(infile);

while (reader.hasEvent() && i_event < n_events) {
    DataEvent event = reader.getNextEvent();
    i_event++;
    if (i_event == 0) System.out.printf("Analyzed %10d events...", i_event);
    else if (i_event%10000 == 0) {
        System.out.printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        System.out.printf("Analyzed %10d events...", i_event);
    }
    // Get relevant data banks.
    DataBank rec_part   = null;
    DataBank rec_track  = null;
    DataBank rec_traj   = null;
    DataBank fmt_tracks = null;
    DataBank rec_ecal   = null;
    DataBank rec_tof    = null;
    if (event.hasBank("REC::Particle"))     rec_part   = event.getBank("REC::Particle");
    if (event.hasBank("REC::Track"))        rec_track  = event.getBank("REC::Track");
    if (event.hasBank("REC::Traj"))         rec_traj   = event.getBank("REC::Traj");
    if (event.hasBank("REC::Calorimeter"))  rec_ecal   = event.getBank("REC::Calorimeter");
    if (event.hasBank("REC::Scintillator")) rec_tof    = event.getBank("REC::Scintillator");
    if (event.hasBank("FMT::Tracks"))       fmt_tracks = event.getBank("FMT::Tracks");

    // Ignore events that don't have the minimum required banks.
    if (rec_part==null || rec_track==null || rec_traj==null) continue;

    // TODO. Make sure that this is actually the trigger electron.
    int e_pindex = rec_track.getShort("pindex", 0);
    double e_tof = Double.POSITIVE_INFINITY;
    if (rec_tof != null) {
        for (int hi = 0; hi < rec_tof.rows(); ++hi) {
            if (rec_tof.getShort("pindex", hi) == e_pindex && rec_tof.getFloat("time", hi) < e_tof)
                e_tof = rec_tof.getFloat("time", hi);
        }
    }

    // Loop through trajectory points.
    for (int loop = 0; loop < rec_track.rows(); loop++) {
        int index      = rec_track.getShort("index",  loop);
        int pindex     = rec_track.getShort("pindex", loop);
        int sector     = rec_track.getByte ("sector", loop);
        int ndf        = rec_track.getInt  ("NDF",    loop);
        double chi2    = rec_track.getFloat("chi2",   loop);

        int charge     = rec_part.getByte  ("charge",  pindex);
        int pid        = rec_part.getInt   ("pid",     pindex);
        int status     = rec_part.getShort ("status",  pindex);
        double vz      = rec_part.getFloat ("vz",      pindex);
        double chi2pid = rec_part.getFloat ("chi2pid", pindex);

        // Apply general cuts.
        if ((int) (Math.abs(status)/1000) != 2) continue; // Filter particles that pass through FMT.
        // if (Math.abs(chi2pid) >= 3) continue; // Ignore spurious particles.
        if (pid == 0) continue; // Ignore bad particles.
        // if (vz < -40 || vz > (C.FMT_Z[0]+C.FMT_DZ[0])/10) continue; // Geometry cut.
        if (chi2/ndf >= 15) continue; // Ignore tracks with high chi2.

        n_DC_tracks++; // Count DC tracks.

        // === PROCESS TRACKS ======================================================================
        // Get reconstructed particle.
        Particle part;
        if (!FMT) {
            part = new Particle(pid, rec_part.getFloat("px", pindex),
                                     rec_part.getFloat("py", pindex),
                                     rec_part.getFloat("pz", pindex),
                                     rec_part.getFloat("vx", pindex),
                                     rec_part.getFloat("vy", pindex),
                                     rec_part.getFloat("vz", pindex));
        }
        else {
            // Apply FMT cuts.
            if (fmt_tracks == null) continue;
            if (fmt_tracks.rows() < 1) continue; // abandon all hope if there are no FMT tracks.
            if (fmt_tracks.getInt("NDF", index) != 3) continue; // track crossed three FMT layers.
            n_FMT_tracks++; // Count FMT tracks.

            part = new Particle(pid, fmt_tracks.getFloat("p0_x",   index),
                                     fmt_tracks.getFloat("p0_y",   index),
                                     fmt_tracks.getFloat("p0_z",   index),
                                     fmt_tracks.getFloat("Vtx0_x", index),
                                     fmt_tracks.getFloat("Vtx0_y", index),
                                     fmt_tracks.getFloat("Vtx0_z", index));
        }
        // Get data from other detectors.
        double pcal_E = 0; // PCAL total deposited energy.
        double ecin_E = 0; // EC inner total deposited energy.
        double ecou_E = 0; // EC outer total deposited energy.
        if (rec_ecal != null) {
            for (int hi = 0; hi < rec_ecal.rows(); ++hi) {
                if (rec_ecal.getShort("pindex", hi) == pindex) {
                    int lyr = (int) rec_ecal.getByte("layer", hi);
                    // TODO. Add correction via sampling fraction.

                    if      (lyr == C.PCAL_LYR) pcal_E += rec_ecal.getFloat("energy", hi);
                    else if (lyr == C.ECIN_LYR) ecin_E += rec_ecal.getFloat("energy", hi);
                    else if (lyr == C.ECOU_LYR) ecou_E += rec_ecal.getFloat("energy", hi);
                    else System.out.printf("This shouldn't happen...?\n");
                }
            }
        }
        double total_E = pcal_E + ecin_E + ecou_E;

        double tof = Double.POSITIVE_INFINITY;
        if (rec_tof != null) {
            for (int hi = 0; hi < rec_tof.rows(); ++hi) {
                if (rec_tof.getShort("pindex", hi) == pindex && rec_tof.getFloat("time", hi) < tof)
                    tof = rec_tof.getFloat("time", hi);
            }
        }

        // Check which histograms to fill. NOTE. These cuts shouldn't be hardcoded.
        Map<String, Boolean> fillMap = new HashMap<String, Boolean>();
        fillMap.put(C.K1_E,       (pid == 11 && status < 0) ? true : false);
        // Add all electrons, including primary and secondary.
        fillMap.put(C.K1_PIPLUS,  pid ==  211 ? true : false);
        fillMap.put(C.K1_PIMINUS, pid == -211 ? true : false);
        fillMap.put(C.K1_NEG,     charge < 0  ? true : false);
        fillMap.put(C.K1_POS,     charge > 0  ? true : false);
        fillMap.put(C.K1_ALL,     true);

        // === PROCESS DC TRACKS ===================================================================
        // Ignore particles too far from the beamline.
        if (part.vx()*part.vx() + part.vy()*part.vy() > 4) continue;

        for (String ki : C.K1ARR) {
            if (!fillMap.get(ki)) continue;

            if (rec_ecal != null) {
                // EC vs PCAL.
                hists.get(ki).get(C.K2_ECAL).get(C.K3_EC_PCAL).fill(pcal_E, ecin_E + ecou_E);

                // Sampling fraction (temporary).
                for (int hi = 0; hi < rec_ecal.rows(); ++hi) {
                    if (rec_ecal.getShort("pindex", hi) == pindex) {
                        double E = rec_ecal.getFloat("energy", hi);
                        double sf = p1[sector-1] * (p2[sector-1]+p3[sector-1]/E+p4[sector-1]/E**2);

                        int lyr = (int) rec_ecal.getByte("layer", hi);
                        String loc;
                        if      (lyr == C.PCAL_LYR) loc = C.K3_SF_PCAL[sector-1];
                        else if (lyr == C.ECIN_LYR) loc = C.K3_SF_ECIN[sector-1];
                        else if (lyr == C.ECOU_LYR) loc = C.K3_SF_ECOU[sector-1];
                        hists.get(ki).get(C.K2_ECAL).get(loc).fill(sf);
                    }
                }
            }

            // Vertex z vs phi angle.
            hists.get(ki).get(C.K2_VZ).get(C.K3_VZ_PHI).fill(part.vz(), Math.toDegrees(part.phi()));

            if (sector != 1) continue; // No beam alignment yet, so we only use one sector.
            // Vertex z.
            hists.get(ki).get(C.K2_VZ).get(C.K3_VZ).fill(part.vz());
            hists.get(ki).get(C.K2_VZ).get(C.K3_VZ_THETA).fill(
                    part.vz(), Math.toDegrees(part.theta()));

            // Vertex momentum.
            double beta = (double) rec_part.getFloat("beta", pindex); // no beta data for FMT.
            hists.get(ki).get(C.K2_VP).get(C.K3_VP).fill(part.p());
            hists.get(ki).get(C.K2_VP).get(C.K3_BETA).fill(beta);
            hists.get(ki).get(C.K2_VP).get(C.K3_P_BETA).fill(part.p(), beta);

            // Energy.
            hists.get(ki).get(C.K2_ECAL).get(C.K3_EP_P).fill(part.p(), total_E/part.p)
            hists.get(ki).get(C.K2_ECAL).get(C.K3_EP_E).fill(total_E,  total_E/part.p)

            hists.get(ki).get(C.K2_ECAL).get(C.K3_P_E_PCAL).fill(part.p(), pcal_E);
            hists.get(ki).get(C.K2_ECAL).get(C.K3_P_E_ECIN).fill(part.p(), ecin_E);
            hists.get(ki).get(C.K2_ECAL).get(C.K3_P_E_ECOU).fill(part.p(), ecou_E);

            // TOF difference. TODO. Check TOF resolution.
            double dtof = tof - e_tof;
            if (!Double.isNaN(dtof)) {
                hists.get(ki).get(C.K2_TOF).get(C.K3_DTOF).fill(dtof);
                hists.get(ki).get(C.K2_TOF).get(C.K3_P_DTOF).fill(part.p(), dtof);
            }

            // Get SIDIS variables.
            if (pid == 11) {
                hists.get(ki).get(C.K2_SIDIS).get(C.K3_Q2).fill(calc_Q2(beam_energy, part));
                hists.get(ki).get(C.K2_SIDIS).get(C.K3_Q2).fill(calc_nu(beam_energy, part));
                hists.get(ki).get(C.K2_SIDIS).get(C.K3_Q2).fill(calc_Xb(C, beam_energy, part));
            }
        }
	}
}
reader.close();
System.out.printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
System.out.printf("Analyzed %10d events. Done!\n", i_event);
if (FMT) {
    int diff = n_DC_tracks - n_FMT_tracks;
    System.out.printf("%% of lost tracks: %5.2f%% (%d/%d)\n",
                      100*(diff / (double) n_DC_tracks), diff, n_DC_tracks);
}

// Fitting.
for (String ki : C.K1ARR) {
    // VZ fits.
    H1F hist = hists.get(ki).get(C.K2_VZ).get(C.K3_VZ);
    F1D f_vz = new F1D(C.K3_VZ_FIT,
            "[amp1]*gaus(x,[mean],[sigma])+[amp2]*gaus(x,[mean]-2.4,[sigma])+[p0]+[p1]*x+[p2]*x*x",
            -36, -30);

    double amp  = hist.getBinContent(hist.getMaximumBin());
    double mean = hist.getDataX(hist.getMaximumBin());
    f_vz.setParameter(0, amp);
    f_vz.setParameter(1, mean);
    f_vz.setParameter(2, 0.5);

    f_vz.setOptStat("1111");
    DataFitter.fit(f_vz, hists.get(ki).get(C.K2_VZ).get(C.K3_VZ), "Q");
}

// Save hists
TDirectory dir = new TDirectory();
for (String ki : C.K1ARR) {
    // Fit.
    // fit_upstream(hists.get(ki).get(C.K2_VZ).get(C.K3_VZ), f_vz, -36, -30);

    // Save hists.
    dir.mkdir("/" + ki);

    mkcd(dir, "/" + ki + "/Vertex z");
    dir.addDataSet(hists.get(ki).get(C.K2_VZ).get(C.K3_VZ));
    dir.addDataSet(hists.get(ki).get(C.K2_VZ).get(C.K3_VZ_PHI));
    dir.addDataSet(hists.get(ki).get(C.K2_VZ).get(C.K3_VZ_THETA));

    mkcd(dir, "/" + ki + "/Vertex p");
    dir.addDataSet(hists.get(ki).get(C.K2_VP).get(C.K3_VP));
    dir.addDataSet(hists.get(ki).get(C.K2_VP).get(C.K3_BETA));
    dir.addDataSet(hists.get(ki).get(C.K2_VP).get(C.K3_P_BETA));

    mkcd(dir, "/" + ki + "/ECAL");
    dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_P_E_PCAL));
    dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_P_E_ECIN));
    dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_P_E_ECOU));
    dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_EC_PCAL));

    dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_EP_P));
    dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_EP_E));

    mkcd(dir, "/" + ki + "/ECAL/Sampling Fraction");
    for (int si = 0; si < C.NSECTORS; ++si) {
        dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_SF_PCAL[si]));
        dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_SF_ECIN[si]));
        dir.addDataSet(hists.get(ki).get(C.K2_ECAL).get(C.K3_SF_ECOU[si]));
    }

    mkcd(dir, "/" + ki + "/TOF");
    dir.addDataSet(hists.get(ki).get(C.K2_TOF).get(C.K3_DTOF));
    dir.addDataSet(hists.get(ki).get(C.K2_TOF).get(C.K3_P_DTOF));

    if (ki.equals(C.K1_E)) {
        mkcd(dir, "/" + ki + "/SIDIS");
        dir.addDataSet(hists.get(ki).get(C.K2_SIDIS).get(C.K3_Q2));
        dir.addDataSet(hists.get(ki).get(C.K2_SIDIS).get(C.K3_NU));
        dir.addDataSet(hists.get(ki).get(C.K2_SIDIS).get(C.K3_XB));
    }

    dir.cd("..");
}
dir.writeFile("out.hipo");

/* Handler of all input-output of the program. */
public final class IO_handler {
    // No global static classes are allowed in java so this is the closest second...
    private IO_handler() {}

    private static char[] argnames = ['n', 'd'];
    static Map<String, Character> argmap;

    /* Associate char-indexed args with String-indexed args. */
    private static int initialize_argmap() {
        argmap = new HashMap<>();
        for (char argname in argnames) {
            if      (argname == 'n') argmap.put("--nevents",  'n');
            else if (argname == 'd') argmap.put("--detector", 'd');
            else {
                System.err.printf("Silly programmer, you forgot to associate all single ");
                System.err.printf("char-indexed args to String-indexed args. Fix this in ");
                System.err.printf(" IO_handler.parse_args().\n");
                return 1;
            }
        }
        return 0;
    }

    /* Print usage to stdout and exit. */
    public static int usage() {
        System.out.printf("Usage: acceptance [-n --nevents] [-d --detector] <inputfile>\n");
        System.out.printf("You dun goofed mate.\n"); // TODO.
        return 1;
    }

    /* Arguments parser. Arguments are detailed in usage(). */
    public static int parse_args(String[] args, Map<Character, String> params) {
        // NOTE: Better error messages here would be cool, but not necessary atm.
        if (initialize_argmap()) return 1;
        if (args.length<1) return usage();

        // Get args. Can't believe that Java doesn't have a standard method for this.
        for (int i=0; i<args.length; ++i) {
            final String a = args[i];
            if (args[i].charAt(0) == '-') {
                if (args[i].length()<2) return usage();
                if (args[i].charAt(1)=='-') { // string-indexed argument.
                    if (args[i].length()<3) return usage();
                    if (argmap.get(args[i]) == null) return usage();
                    params.put(argmap.get(args[i]), args[++i]);
                }
                else { // char-indexed argument.
                    if (args[i].length()>2) return usage();
                    params.put(args[i].substring(args[i].length()-1), args[++i]);
                }
            }
            else { // positional argument.
                params.put('f', args[i]);
            }
        }

        // Check that args are what's expected.
        if (params.get('f') == null) return usage();

        // Check that all args are within accepted keys.
        for (Character key in params.keySet()) {
            if (key == 'f') continue;
            boolean accept = false;
            for (int i = 0; i < argnames.length; ++i) {
                if (key == argnames[i]) {
                    accept = true;
                    break;
                }
            }
            if (!accept) return usage();
        }

        // Check that args are of correct type.
        if (params.get('n') != null) {
            try {Integer.parseInt(params.get('n'));}
            catch (NumberFormatException e) {return usage();}
        }
        if (params.get('d' != null)) {
            String detector = params.get('d').toLowerCase();
            if (!detector.equals("dc") && !detector.equals("fmt")) return usage();
        }

        return 0;
    }

    /* Get run number from file name. */
    public static int get_runno(String path) {
        // This assumes that filename has the format "/path/to/out_clas_<RUNNO>.hipo".
        String[] splitpath = path.split("/");
        return Integer.parseInt(splitpath[splitpath.length-1].split("\\.")[0].split("_")[2]);
    }
}
/* Repository of constants. */
public class Constants {
    // Data structure constants.
    String K1_E        = "trigger electron";
    String K1_PIPLUS   = "pi+";
    String K1_PIMINUS  = "pi-";
    String K1_NEG      = "negative";
    String K1_POS      = "positive";
    String K1_ALL      = "all particles";
    String[] K1ARR = [K1_E, K1_PIPLUS, K1_PIMINUS, K1_NEG, K1_POS, K1_ALL];

    String K2_VZ       = "vertex Z";
    String K2_VP       = "vertex P";
    String K2_ECAL     = "ECAL";
    String K2_TOF      = "TOF"
    String K2_SIDIS    = "SIDIS variables";

    String K3_VZ       = "vertex Z";
    String K3_VZ_FIT   = "vertex Z upstream fit";
    String K3_VZ_THETA = "vertex Z vs Theta angle";
    String K3_VZ_PHI   = "vertex Z vs Phi angle";

    String K3_VP       = "vertex P";
    String K3_BETA     = "beta";
    String K3_P_BETA   = "vertex P vs beta";

    String K3_EP_P     = "P div E vs P";
    String K3_EP_E     = "P div E vs E";
    String K3_P_E_PCAL = "vertex P vs energy (PCAL)";
    String K3_P_E_ECIN = "vertex P vs energy (ECIN)";
    String K3_P_E_ECOU = "vertex P vs energy (ECOU)";
    String K3_EC_PCAL  = "ECAL energy vs PCAL energy";
    String[] K3_SF_PCAL = ["PCAL SF - sector 1", "PCAL SF - sector 2", "PCAL SF - sector 3",
                           "PCAL SF - sector 4", "PCAL SF - sector 5", "PCAL SF - sector 6"];
    String[] K3_SF_ECIN = ["ECIN SF - sector 1", "ECIN SF - sector 2", "ECIN SF - sector 3",
                           "ECIN SF - sector 4", "ECIN SF - sector 5", "ECIN SF - sector 6"];
    String[] K3_SF_ECOU = ["ECOU SF - sector 1", "ECOU SF - sector 2", "ECOU SF - sector 3",
                           "ECOU SF - sector 4", "ECOU SF - sector 5", "ECOU SF - sector 6"];

    String K3_DTOF     = "TOF difference";
    String K3_P_DTOF   = "vertex P vs TOF difference";

    String K3_Q2       = "Q2";
    String K3_NU       = "Nu";
    String K3_XB       = "Xb";

    // Masses.
    double PIMASS  = 0.139570; // Pion mass.
    double PRTMASS = 0.938272; // Proton mass.
    double NTRMASS = 0.939565; // Neutron mass.
    double EMASS   = 0.000051; // Electron mass.
    HashMap<Integer, Double> PIDMASS = new HashMap<Integer, Double>();

    // CLAS12 Constants.
    int NSECTORS   = 6; // Number of CLAS12 sectors.

    // FMT Constants.
    int    FMT_NLAYERS; // Number of FMT layers (3 for RG-F).
    int    FMT_NSTRIPS; // Number of strips per FMT layer (1024).
    double FMT_PITCH;   // Pitch of the entire detector.
    int    FMT_STRIP;   // TODO.
    int    FMT_HALF;    // TODO.
    double FMT_RMIN;    // Radius of the hole at the middle of FMT.
    double FMT_RMAX;    // Radius of the whole FMT circle.
    double[] FMT_Z;     // z position of the FMT layers in mm.
    double[] FMT_ANGLE; // strip angle in deg.
    double[] FMT_DX;    // x shift in mm.
    double[] FMT_DY;    // y shift in mm.
    double[] FMT_DZ;    // z shift in mm.

    // Other detector constants.
    int PCAL_LYR = 1; // PCAL's layer id.
    int ECIN_LYR = 4; // EC inner layer id.
    int ECOU_LYR = 7; // EC outer layer id.

    Constants() {
        // Initialize PIDMASS hashmap.
        PIDMASS.put(  11, EMASS);
        PIDMASS.put(2212, PRTMASS);
        PIDMASS.put(2112, NTRMASS);
        PIDMASS.put( 211, PIMASS);
        PIDMASS.put(-211, PIMASS);
        PIDMASS.put( 111, PIMASS);

        // Get stuff from CCDB.
        DatabaseConstantProvider dbProvider = new DatabaseConstantProvider(10, "rgf_spring2020");
        dbProvider.loadTable("/geometry/fmt/fmt_global");
        dbProvider.loadTable("/geometry/fmt/fmt_layer_noshim");
        dbProvider.loadTable("/geometry/fmt/alignment");

        FMT_NLAYERS = 3;
        FMT_NSTRIPS = 1024;
        FMT_PITCH = dbProvider.getDouble("/geometry/fmt/fmt_global/Pitch",     0);
        FMT_STRIP = dbProvider.getDouble("/geometry/fmt/fmt_global/N_strip",   0);
        FMT_HALF  = dbProvider.getDouble("/geometry/fmt/fmt_global/N_halfstr", 0);
        FMT_RMIN  = dbProvider.getDouble("/geometry/fmt/fmt_global/R_min",     0);
        FMT_RMAX  = FMT_PITCH*(FMT_STRIP-FMT_HALF)/2;
        FMT_Z     = new double[FMT_NLAYERS]; // z position of the FMT layers in mm.
        FMT_ANGLE = new double[FMT_NLAYERS]; // strip angle in deg.
        FMT_DX    = new double[FMT_NLAYERS]; // x shift in mm.
        FMT_DY    = new double[FMT_NLAYERS]; // y shift in mm.
        FMT_DZ    = new double[FMT_NLAYERS]; // z shift in mm.
        for (int i=0; i<FMT_NLAYERS; i++) {
            FMT_Z[i]     = dbProvider.getDouble("/geometry/fmt/fmt_layer_noshim/Z",i);
            FMT_ANGLE[i] = dbProvider.getDouble("/geometry/fmt/fmt_layer_noshim/Angle",i);
            FMT_DX[i]    = dbProvider.getDouble("/geometry/fmt/alignment/deltaX",i);
            FMT_DY[i]    = dbProvider.getDouble("/geometry/fmt/alignment/deltaY",i);
            FMT_DZ[i]    = dbProvider.getDouble("/geometry/fmt/alignment/deltaZ",i);
        }
        // FMT_Z[0]=260.505; FMT_Z[1]=269.891; FMT_Z[2]=282.791;
    }
}
public boolean gen_map(Constants C, HashMap<String, HashMap<String, IDataSet>> hists, int ki) {
    hists.put(C.K2_VZ,    new HashMap<String, IDataSet>());
    hists.put(C.K2_VP,    new HashMap<String, IDataSet>());
    hists.put(C.K2_ECAL,  new HashMap<String, IDataSet>());
    hists.put(C.K2_TOF,   new HashMap<String, IDataSet>());
    hists.put(C.K2_SIDIS, new HashMap<String, IDataSet>());
    gen_map_vz(C, hists.get(C.K2_VZ));
    gen_map_vp(C, hists.get(C.K2_VP));
    gen_map_ecal(C, hists.get(C.K2_ECAL));
    gen_map_tof(C, hists.get(C.K2_TOF));
    gen_map_sidis(C, hists.get(C.K2_SIDIS));

    return false;
}
private boolean gen_map_vz(Constants C, HashMap<String, IDataSet> hists) {
    // 1D vz.
    hists.put(C.K3_VZ, new H1F(C.K3_VZ, "vz (cm)", "Counts", 500, -50, 50));
    hists.get(C.K3_VZ).setFillColor(43);

    // TODO. Fitting is to be done via fit() method at:
    // https://github.com/gavalian/groot/blob/master/src/main/java/org/jlab/groot/data/H1F.java#L456-L458
    // F1D f_vz = new F1D(K3_VZ_FIT,
    //         "[amp1]*gaus(x,[mean],[sigma])+[amp2]*gaus(x,[mean]-2.4,[sigma])+[p0]+[p1]*x+[p2]*x*x",
    //         -50, 50);
    // f_vz.setLineWidth(2);
    // f_vz.setLineColor(2);
    // f_vz.setOptStat("1111");

    // 2D vz vs theta.
    hists.put(C.K3_VZ_THETA, new H2F(C.K3_VZ_THETA, 200, -50, 50, 100, 0, 40));
    hists.get(C.K3_VZ_THETA).setTitleX("vz (cm)");
    hists.get(C.K3_VZ_THETA).setTitleY("#theta (deg)");

    // // Draw lines showing FMT acceptance.
    // F1D ftheta1 = new F1D("ftheta1","57.29*atan([r]/([z0]-x))", -50, 20.5);
    // ftheta1.setParameter(0, C.FMT_RMIN/10);
    // ftheta1.setParameter(1, (C.FMT_Z[0]+C.FMT_DZ[0])/10);
    // ftheta1.setLineColor(2);
    // ftheta1.setLineWidth(2);
    // dg.addDataSet(ftheta1, C.POS_VZ_THETA);
    // F1D ftheta2 = new F1D("ftheta2","57.29*atan([r]/([z0]-x))", -50, 3);
    // ftheta2.setParameter(0, C.FMT_RMAX/10);
    // ftheta2.setParameter(1, (C.FMT_Z[0]+C.FMT_DZ[0])/10);
    // ftheta2.setLineColor(2);
    // ftheta2.setLineWidth(2);
    // dg.addDataSet(ftheta2, C.POS_VZ_THETA);

    // 2D vz vs phi angle.
    hists.put(C.K3_VZ_PHI, new H2F(C.K3_VZ_PHI, 200, -50, 50, 100, -180, 180));
    hists.get(C.K3_VZ_PHI).setTitleX ("vz (cm)");
    hists.get(C.K3_VZ_PHI).setTitleY ("#phi (deg)");

    return false;
}
private boolean gen_map_vp(Constants C, HashMap<String, IDataSet> hists) {
    // Momentum distribution.
    hists.put(C.K3_VP, new H1F(C.K3_VP, "p (GeV)", "Counts", 100, 0, 12));
    hists.get(C.K3_VP).setFillColor(44);

    // Beta distribution.
    hists.put(C.K3_BETA, new H1F(C.K3_BETA, "#beta", "Counts", 500, 0.9, 1));
    hists.get(C.K3_BETA).setFillColor(44);

    // Momentum vs Beta.
    hists.put(C.K3_P_BETA, new H2F(C.K3_P_BETA, 200, 0, 12, 200, 0.9, 1));
    hists.get(C.K3_P_BETA).setTitleX("p (GeV)");
    hists.get(C.K3_P_BETA).setTitleY("#beta");

    // // Draw theoretical curve. // TODO. FIX.
    // Double mass = C.PIDMASS.get(pid);
    // if (mass != null) {
    //     F1D fp = new F1D("fp", "[0]*x/sqrt(1-x)", 0, 1);
    //     fp.setParameter(0, mass);
    //     fp.setLineColor(2);
    //     fp.setLineWidth(2);
    //     dg.addDataSet(fp,C.POS_P_BETA);
    // }

    return false;
}
private boolean gen_map_ecal(Constants C, HashMap<String, IDataSet> hists) {
    // Energy/momentum vs momentum.
    hists.put(C.K3_EP_P, new H2F(C.K3_EP_P, 200, 0, 10, 200, 0.10, 0.40));
    hists.get(C.K3_EP_P).setTitleX("p (GeV)");
    hists.get(C.K3_EP_P).setTitleY("E/p");

    // Energy/momentum vs energy.
    hists.put(C.K3_EP_E, new H2F(C.K3_EP_E, 200, 0, 3, 200, 0.10, 0.40));
    hists.get(C.K3_EP_E).setTitleX("E (GeV)");
    hists.get(C.K3_EP_E).setTitleY("E/p");

    // Energy vs Momentum for PCAL.
    hists.put(C.K3_P_E_PCAL, new H2F(C.K3_P_E_PCAL, 200, 0, 12, 200, 0, 12));
    hists.get(C.K3_P_E_PCAL).setTitleX("p (GeV)");
    hists.get(C.K3_P_E_PCAL).setTitleY("PCAL - E (GeV)");

    // Energy vs Momentum for EC Inner.
    hists.put(C.K3_P_E_ECIN, new H2F(C.K3_P_E_ECIN, 200, 0, 12, 200, 0, 12));
    hists.get(C.K3_P_E_ECIN).setTitleX("p (GeV)");
    hists.get(C.K3_P_E_ECIN).setTitleY("ECIN - E (GeV)");

    // Energy vs Momentum for EC Outer.
    hists.put(C.K3_P_E_ECOU, new H2F(C.K3_P_E_ECOU, 200, 0, 12, 200, 0, 12));
    hists.get(C.K3_P_E_ECOU).setTitleX("p (GeV)");
    hists.get(C.K3_P_E_ECOU).setTitleY("ECOU - E (GeV)");

    // Sampling Fraction.
    for (int si = 0; si < C.NSECTORS; ++si) {
        hists.put(C.K3_SF_PCAL[si], new H1F(C.K3_SF_PCAL[si], "SF", "Counts", 200, 0.1, 0.5));
        hists.get(C.K3_SF_PCAL[si]).setFillColor(43);

        hists.put(C.K3_SF_ECIN[si], new H1F(C.K3_SF_ECIN[si], "SF", "Counts", 200, 0.1, 0.5));
        hists.get(C.K3_SF_ECIN[si]).setFillColor(43);

        hists.put(C.K3_SF_ECOU[si], new H1F(C.K3_SF_ECOU[si], "SF", "Counts", 200, 0.1, 0.5));
        hists.get(C.K3_SF_ECOU[si]).setFillColor(43);
    }

    // EC vs PCAL.
    hists.put(C.K3_EC_PCAL, new H2F(C.K3_EC_PCAL, 200, 0, 2, 200, 0, 2));
    hists.get(C.K3_EC_PCAL).setTitleX("PCAL E (GeV)");
    hists.get(C.K3_EC_PCAL).setTitleY("EC E (GeV)");

    return false;
}
private boolean gen_map_tof(Constants C, HashMap<String, IDataSet> hists) {
    hists.put(C.K3_DTOF, new H1F(C.K3_DTOF, "TOF difference (ns)", "Counts", 100, 0, 50));
    hists.get(C.K3_DTOF).setFillColor(43);

    hists.put(C.K3_P_DTOF, new H2F(C.K3_P_DTOF, 200, 0, 12, 200, 0, 12));
    hists.get(C.K3_P_DTOF).setTitleX("p (GeV)");
    hists.get(C.K3_P_DTOF).setTitleX("TOF difference (ns)");

    return false;
}
private boolean gen_map_sidis(Constants C, HashMap<String, IDataSet> hists) {

    // Q2.
    hists.put(C.K3_Q2, new H1F(C.K3_Q2, "Q^2 (GeV^2)", "Counts", 22, 0, 12));
    hists.get(C.K3_Q2).setFillColor(43);

    // nu
    hists.put(C.K3_NU, new H1F(C.K3_NU, "#nu (GeV)", "Counts", 22, 0, 12));
    hists.get(C.K3_NU).setFillColor(43);

    // X_bjorken
    hists.put(C.K3_XB, new H1F(C.K3_XB, "Xb (GeV^2)", "Counts", 20, 0, 2));
    hists.get(C.K3_NU).setFillColor(43);

    return false;
}
public static double calc_Q2(double beam_e, Particle p) {
    return 4 * beam_e * p.p() * Math.sin(p.theta()/2)**2;
}
public static double calc_nu(double beam_e, Particle p) {
    return beam_e - p.p();
}
public static double calc_Xb(Constants C, double beam_e, Particle p) {
    return calc_Q2(beam_e, p)/2 / calc_nu(beam_e, p)/C.PRTMASS;
}
public static void mkcd(TDirectory dir, String name) {
    dir.mkdir(name);
    dir.cd(name);
}
public static int setup_groot() {
    GStyle.getAxisAttributesX().setTitleFontSize(24);
    GStyle.getAxisAttributesX().setLabelFontSize(18);
    GStyle.getAxisAttributesY().setTitleFontSize(24);
    GStyle.getAxisAttributesY().setLabelFontSize(18);
    GStyle.getAxisAttributesZ().setLabelFontSize(14);
    GStyle.getAxisAttributesX().setLabelFontName("Arial");
    GStyle.getAxisAttributesY().setLabelFontName("Arial");
    GStyle.getAxisAttributesZ().setLabelFontName("Arial");
    GStyle.getAxisAttributesX().setTitleFontName("Arial");
    GStyle.getAxisAttributesY().setTitleFontName("Arial");
    GStyle.getAxisAttributesZ().setTitleFontName("Arial");
    GStyle.setGraphicsFrameLineWidth(1);
    GStyle.getH1FAttributes().setLineWidth(2);
    GStyle.getH1FAttributes().setOptStat("1111");

    return 0;
}
public static int fit_upstream(H1F hist, F1D fit, double min, double max) {
    double amp = hist.getBinContent(hist.getMaximumBin());

    // Initial guesses for fit.
    fit.setParameter(0, 2*amp);   // amp1
    fit.setParameter(1, -32.085); // mean
    fit.setParameter(2, 1);       // sigma

    fit.setRange(min, max);
    DataFitter.fit(fit, hist, "Q"); // No options use error for sigma.
    hist.setFunction(null);

    return 0;
}
