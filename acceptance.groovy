import javax.swing.JFrame;

import org.jlab.clas.physics.Particle;
import org.jlab.detector.calib.utils.DatabaseConstantProvider;
import org.jlab.groot.base.GStyle;
import org.jlab.groot.data.H1F;
import org.jlab.groot.data.H2F;
import org.jlab.groot.math.F1D;
import org.jlab.groot.fitter.DataFitter;
import org.jlab.groot.group.DataGroup;
import org.jlab.groot.graphics.EmbeddedCanvasTabbed;
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
        System.err.printf("Run number not in database. Add from clas12mon.");
        System.exit(1);
}

// Initial setup.
// NOTE. names and DataGroups should be kept in a struct (or object...).
setup_groot();
Constants C = new Constants();
String[] names = new String[C.N_CNVS]; DataGroup[] dg = new DataGroup[C.N_CNVS];
names[0] = "e";                        dg[0] = gen_dg(C,  11);
names[1] = "pi+";                      dg[1] = gen_dg(C, 211);
names[2] = "pi-";                      dg[2] = gen_dg(C,-211);
names[3] = "negative";                 dg[3] = gen_dg(C,   0);
names[4] = "positive";                 dg[4] = gen_dg(C,   0);
names[5] = "all particles";            dg[5] = gen_dg(C,   0);

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

    // We assume that the first particle in the particle bank is the elctron.
    // TODO. Confirm this with Raffaella with urgency.
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
        status = (int) (Math.abs(status)/1000);

        // Apply general cuts.
        if (status != 2) continue; // Only use particles that pass through DC and FMT.
        if (Math.abs(chi2pid) >= 3) continue; // Ignore spurious particles.
        if (vz < -40 || vz > (C.FMT_Z[0]+C.FMT_DZ[0])/10) continue; // Geometry cut.
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
        double pcal_E = 0; // ECAL total deposited energy.
        if (rec_ecal != null) {
            for (int hi = 0; hi < rec_ecal.rows(); ++hi) {
                if (rec_ecal.getShort("pindex", hi) == pindex)
                    pcal_E += rec_ecal.getFloat("energy", hi);
            }
        }
        double tof = Double.POSITIVE_INFINITY;
        if (rec_tof != null) {
            for (int hi = 0; hi < rec_tof.rows(); ++hi) {
                if (rec_tof.getShort("pindex", hi) == pindex && rec_tof.getFloat("time", hi) < tof)
                    tof = rec_tof.getFloat("time", hi);
            }
        }

        // Check which histograms to fill. NOTE. These cuts shouldn't be hardcoded.
        boolean[] hists = new boolean[6];
        hists[0] = pid ==   11 ? true : false;
        hists[1] = pid ==  211 ? true : false;
        hists[2] = pid == -211 ? true : false;
        hists[3] = charge < 0  ? true : false;
        hists[4] = charge > 0  ? true : false;
        hists[5] = true;

        // === PROCESS DC TRACKS ===================================================================
        // Ignore particles too far from the beamline.
        if (part.vx()*part.vx() + part.vy()*part.vy() > 4) continue;

        for (int cnvs_i = 0; cnvs_i < dg.length; ++cnvs_i) {
            if (!hists[cnvs_i]) continue;

            dg[cnvs_i].getH2F("vz_phi").fill(part.vz(), Math.toDegrees(part.phi()));
            if (sector != 1) continue; // No beam alignment yet, so we only use one sector.
            // Vertex z datagroup.
            dg[cnvs_i].getH1F("vz").fill(part.vz());
            dg[cnvs_i].getH2F("vz_theta").fill(part.vz(), Math.toDegrees(part.theta()));

            // Vertex momentum datagroup.
            double beta = (double) rec_part.getFloat("beta", pindex); // no beta data for FMT.
            dg[cnvs_i].getH1F("p").fill(part.p());
            dg[cnvs_i].getH1F("beta").fill(beta);
            dg[cnvs_i].getH2F("p_beta").fill(part.p(), beta);

            // Energy datagroup.
            dg[cnvs_i].getH2F("p_E").fill(part.p(), part.e());
            dg[cnvs_i].getH2F("p_E_pcal").fill(part.p(), pcal_E);

            // TOF difference. TODO. Check TOF resolution.
            double dtof = tof - e_tof;
            if (!Double.isNaN(dtof)) {
                dg[cnvs_i].getH1F("dtof").fill(dtof);
                dg[cnvs_i].getH2F("p_dtof").fill(part.p(), dtof);
            }

            // Get SIDIS variables.
            if (pid != 11) {
                dg[cnvs_i].getH1F("Q2").fill(calc_Q2(beam_energy, part));
                dg[cnvs_i].getH1F("nu").fill(calc_nu(beam_energy, part));
                dg[cnvs_i].getH1F("Xb").fill(calc_Xb(C, beam_energy, part));
            }
        }
	}
}
reader.close();
System.out.printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
System.out.printf("Analyzed %10d events. Done!\n", i_event);
System.out.printf("%% of lost tracks: %5.2f%% (%d/%d)\n",
        100*((n_DC_tracks - n_FMT_tracks) / (double) n_DC_tracks),
        n_DC_tracks - n_FMT_tracks, n_DC_tracks);

// Setup plots and draw.
EmbeddedCanvasTabbed canvases = new EmbeddedCanvasTabbed(false);
for (int cnvs_i = 0; cnvs_i < dg.length; ++cnvs_i) {
    // Apply fits.
    fit_upstream(dg[cnvs_i].getH1F("vz"), dg[cnvs_i].getF1D("fit_vz"), -36, -30);

    canvases.addCanvas(names[cnvs_i]);
    setup_canvas(canvases, names[cnvs_i], dg[cnvs_i]);
    canvases.getCanvas(names[cnvs_i]).cd(0).getPad(C.POS_P).getAxisY().setLog(true);
}

JFrame frame = new JFrame("Acceptance Study Results");
frame.setSize(1500, 1000);
frame.add(canvases);
frame.setLocationRelativeTo(null);
frame.setVisible(true);

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
    // Generic constants.
    int N_CNVS        = 6;

    // Canvas positions --- using names instead of arrays to minimize confusion.
    int POS_VZ        = 0;
    int POS_VZ_THETA  = 1;
    int POS_VZ_PHI    = 2;
    int POS_P         = 6;
    int POS_BETA      = 7;
    int POS_P_BETA    = 8;

    int POS_P_E      = 12;
    int POS_DTOF     = 13;
    int POS_P_E_PCAL = 18;
    int POS_P_DTOF   = 19;

    int POS_Q2 = 15;
    int POS_NU = 16;
    int POS_XB = 17;

    // Masses.
    double PIMASS  = 0.139570; // Pion mass.
    double PRTMASS = 0.938272; // Proton mass.
    double NTRMASS = 0.939565; // Neutron mass.
    double EMASS   = 0.000051; // Electron mass.
    HashMap<Integer, Double> PIDMASS = new HashMap<Integer, Double>();

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
public DataGroup gen_dg(Constants C, int pid) {
    DataGroup dg = new DataGroup(6,4);
    gen_dg_vz   (dg, C);      // Vertex z.
    gen_dg_vp   (dg, C, pid); // Vertex p and beta.
    gen_dg_E    (dg, C);      // All E and TOF stuff.
    gen_dg_sidis(dg, C);      // SIDIS variables.
    return dg;
}
private int gen_dg_vz(DataGroup dg, Constants C) {
    // 1D vz.
    H1F hi_vz  = new H1F("vz", "vz (cm)" , "Counts", 500, -50, 50);
    hi_vz.setFillColor(43);
    dg.addDataSet(hi_vz, C.POS_VZ);

    // Upstream fit for tracks.
    F1D f_vz = new F1D("fit_vz",
            "[amp1]*gaus(x,[mean],[sigma])+[amp2]*gaus(x,[mean]-2.4,[sigma])+[p0]+[p1]*x+[p2]*x*x",
            -50, 50);
    f_vz.setLineWidth(2);
    f_vz.setLineColor(2);
    f_vz.setOptStat("1111");
    dg.addDataSet(f_vz, C.POS_VZ);

    // 2D vz vs theta.
    H2F hi_vz_theta = new H2F("vz_theta", 200, -50, 50, 100, 0, 40);
    hi_vz_theta.setTitleX("vz (cm)");
    hi_vz_theta.setTitleY("#theta (deg)");
    dg.addDataSet(hi_vz_theta, C.POS_VZ_THETA);

    // Draw lines showing FMT acceptance.
    F1D ftheta1 = new F1D("ftheta1","57.29*atan([r]/([z0]-x))", -50, 20.5);
    ftheta1.setParameter(0, C.FMT_RMIN/10);
    ftheta1.setParameter(1, (C.FMT_Z[0]+C.FMT_DZ[0])/10);
    ftheta1.setLineColor(2);
    ftheta1.setLineWidth(2);
    dg.addDataSet(ftheta1, C.POS_VZ_THETA);
    F1D ftheta2 = new F1D("ftheta2","57.29*atan([r]/([z0]-x))", -50, 3);
    ftheta2.setParameter(0, C.FMT_RMAX/10);
    ftheta2.setParameter(1, (C.FMT_Z[0]+C.FMT_DZ[0])/10);
    ftheta2.setLineColor(2);
    ftheta2.setLineWidth(2);
    dg.addDataSet(ftheta2, C.POS_VZ_THETA);

    // 2D vz vs phi angle.
    H2F hi_vz_phi = new H2F("vz_phi", 200, -50, 50, 100, -180, 180);
    hi_vz_phi.setTitleX ("vz (cm)");
    hi_vz_phi.setTitleY ("#phi (deg)");
    dg.addDataSet(hi_vz_phi, C.POS_VZ_PHI);

    return 0;
}
private int gen_dg_vzsector() { // NOTE. Unused, obsolete.
    DataGroup dg = new DataGroup(3,2);
    String xax = "z (cm) - sector ";
    String yax = "Counts";
    for (int sec=1; sec<=6; sec++) {
        H1F hi_vz_dc  = new H1F("hi_vz_dc_sec"  + sec, xax + sec, yax, 500, -50, 50);
        hi_vz_dc .setFillColor(43);
        dg.addDataSet(hi_vz_dc,  sec-1);
        H1F hi_vz_fmt = new H1F("hi_vz_fmt_sec" + sec, xax + sec, yax, 500, -50, 50);
        hi_vz_fmt.setFillColor(44);
        dg.addDataSet(hi_vz_fmt, sec-1);
        // NOTE: Maybe add a fit for DC and for FMT for each sector?
    }
    return 0;
}
private int gen_dg_vp(DataGroup dg, Constants C, int pid) {
    // Momentum distribution.
    H1F hi_p = new H1F("p", "p (GeV)", "Counts", 100, 0, 12);
    hi_p.setFillColor(44);
    dg.addDataSet(hi_p, C.POS_P);

    // Beta distribution:
    H1F hi_beta = new H1F("beta", "#beta", "Counts", 500, 0.9, 1);
    hi_beta.setFillColor(44);
    dg.addDataSet(hi_beta, C.POS_BETA);

    // Momentum vs Beta:
    H2F hi_p_beta = new H2F("p_beta", 200, 0, 1, 200, 0.9, 1);
    hi_p_beta.setTitleX("p (GeV)");
    hi_p_beta.setTitleY("#beta");
    dg.addDataSet(hi_p_beta,C.POS_P_BETA);

    // Draw theoretical curve. // TODO. FIX.
    Double mass = C.PIDMASS.get(pid);
    if (mass != null) {
        F1D fp = new F1D("fp", "[0]*x/sqrt(1-x)", 0, 1);
        fp.setParameter(0, mass);
        fp.setLineColor(2);
        fp.setLineWidth(2);
        dg.addDataSet(fp,C.POS_P_BETA);
    }

    return 0;
}
private int gen_dg_E(DataGroup dg, Constants C) {
    // Energy vs Momentum distribution.
    H2F hi_p_E = new H2F("p_E", 200, 0, 12, 200, 0, 12);
    hi_p_E.setTitleX("p (GeV)");
    hi_p_E.setTitleY("E (GeV)");
    dg.addDataSet(hi_p_E, C.POS_P_E);

    // Energy vs Momentum for PCAL.
    H2F hi_p_E_pcal = new H2F("p_E_pcal", 200, 0, 12, 200, 0, 12);
    hi_p_E_pcal.setTitleX("PCAL - p (GeV)");
    hi_p_E_pcal.setTitleY("PCAL - E (GeV)");
    dg.addDataSet(hi_p_E_pcal,C.POS_P_E_PCAL);

    // TOF distribution.
    H1F hi_dtof = new H1F("dtof", "TOF difference (ns)", "Counts", 100, 0, 50);
    hi_dtof.setFillColor(43);
    dg.addDataSet(hi_dtof,C.POS_DTOF);

    // TOF vs momentum.
    H2F hi_p_dtof = new H2F("p_dtof", 200, 0, 12, 200, 0, 12);
    hi_p_dtof.setTitleX("p (GeV)");
    hi_p_dtof.setTitleY("TOF difference (ns)");
    dg.addDataSet(hi_p_dtof,C.POS_P_DTOF);

    return 0;
}
private int gen_dg_sidis(DataGroup dg, Constants C) {
    // Q2.
    H1F hi_Q2 = new H1F("Q2", "Q^2 (GeV^2)", "Counts", 22, 0, 12);
    hi_Q2.setFillColor(43);
    dg.addDataSet(hi_Q2,C.POS_Q2);

    // nu
    H1F hi_nu = new H1F("nu", "#nu (GeV)", "Counts", 22, 0, 12);
    hi_nu.setFillColor(43);
    dg.addDataSet(hi_nu,C.POS_NU);

    // X_bjorken
    H1F hi_Xb = new H1F("Xb", "Xb (GeV^2)", "Counts", 20, 0, 2);
    hi_Xb.setFillColor(43);
    dg.addDataSet(hi_Xb,C.POS_XB);

    return 0;
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
public static int setup_canvas(EmbeddedCanvasTabbed canvas, String name, DataGroup tab) {
    canvas.getCanvas(name).draw(tab);
    canvas.getCanvas(name).setGridX(false);
    canvas.getCanvas(name).setGridY(false);
    canvas.getCanvas(name).setAxisFontSize(16);
    canvas.getCanvas(name).setAxisTitleSize(16);
    return 0;
}
