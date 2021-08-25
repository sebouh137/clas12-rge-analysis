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

/* Handler of all input-output of the program. */
public final class IO_handler {
    // No global static classes are allowed in java so this is the closest second...
    private IO_handler() {}

    private static char[] argnames = ['n', 'c'];
    static Map<String, Character> argmap;

    /* Associate char-indexed args with String-indexed args. */
    private static int initialize_argmap() {
        argmap = new HashMap<>();
        for (argname in argnames) {
            if      (argname == 'n') argmap.put("--nevents", 'n');
            else if (argname == 'c') argmap.put("--cut",     'c');
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
        System.out.printf("Usage: acceptance [-n --nevents] [-c --cut] <inputfile>\n");
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
        // NOTE: There is no check on repeated keys, but I hope planning for that case is not
        //       necessary.
        if (params.get('f') == null) return usage();
        for (Character key in params.keySet()) {
            if (key == 'f') continue;

            boolean accept = false;
            for (argname in argnames) {
                if (key == argname) {
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
            // TODO: Check on 'c'.
            // TODO: Check that 'f' exists.
        }

        return 0;
    }
}

Map<Character, String> params = new HashMap<>();
if (IO_handler.parse_args(args, params)) System.exit(1);

// for (Map.Entry<Character, String> entry : params.entrySet()) {
//     System.out.printf(entry.getKey() + " : " + entry.getValue() + "\n");
// }
// System.out.printf("\n");

String infile = params.get('f');
int n_events = (params.get('n') == null) ? 10000000 : Integer.parseInt(params.get('n'));
// TODO: Setup cut.

// Groot setup.
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

// Initial setup.
Constants C = new Constants();
DataGroup dg_vz       = gen_dg_vz(C);
DataGroup dg_vzsector = gen_dg_vzsector();
DataGroup dg_vp       = gen_dg_vp();

// Loop through events.
int i_event      = -1;
int n_DC_tracks  = 0;
int n_FMT_tracks = 0;
HipoDataSource reader = new HipoDataSource();
reader.open(infile);

while (reader.hasEvent() && i_event < n_events) {
    DataEvent event = reader.getNextEvent();
    i_event++;
    // TODO: Update this to get  a e s t h e t i c   p r i n t i n g .
    if (i_event%10000 == 0) System.out.println("Analyzed " + i_event + " events");

    // Get relevant data banks.
    DataBank rec_part   = null;
    DataBank rec_track  = null;
    DataBank rec_traj   = null;
    DataBank fmt_tracks = null;
    if (event.hasBank("REC::Particle")) rec_part   = event.getBank("REC::Particle");
    if (event.hasBank("REC::Track"))    rec_track  = event.getBank("REC::Track");
    if (event.hasBank("REC::Traj"))     rec_traj   = event.getBank("REC::Traj");
    if (event.hasBank("FMT::Tracks"))   fmt_tracks = event.getBank("FMT::Tracks");

    // Ignore events that don't have the minimum required banks.
    if (rec_part==null || rec_track==null || rec_traj==null /*|| */) continue;

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
        if (status != 2) continue; // TODO: Remember what this is.
        if (Math.abs(chi2pid) >= 3) continue; // Ignore spurious particles.
        // TODO: Add a vertex cut for tracks far away from the target.
        // if (vz > (C.FMT_Z[0]+C.FMT_DZ[0])/10) continue; // Ignore tracks further downstream than FMT.
        // if (pid != 11) continue; // Use only electrons.
        if (pid != 211) continue; // Use only pions.
        if (chi2/ndf >= 15) continue; // Ignore tracks with high chi2.
        // if (rec_track.rows() < 1) continue; // Ignore tracks with no tracks... wait a minute...

        // === PROCESS DC TRACKS ===================================================================
        Particle dc_part = new Particle(pid,
                rec_part.getFloat("px", pindex),
                rec_part.getFloat("py", pindex),
                rec_part.getFloat("pz", pindex),
                rec_part.getFloat("vx", pindex),
                rec_part.getFloat("vy", pindex),
                rec_part.getFloat("vz", pindex));
        // NOTE: Why is this cut here?
        if (Math.sqrt(dc_part.vx()*dc_part.vx() + dc_part.vy()*dc_part.vy()) > 2) continue;
        double beta = (double) rec_part.getFloat("beta", pindex);

        n_DC_tracks++;
        if (sector==1) { // NOTE: No beam alignment yet, so we only use one sector for now.
            // Vertex z datagroup.
            dg_vz.getH1F("hi_vz_dc").fill(dc_part.vz());
            dg_vz.getH2F("hi_vz_theta_dc").fill(dc_part.vz(), Math.toDegrees(dc_part.theta()));
            dg_vz.getH2F("hi_vz_phi_dc").fill(dc_part.vz(), Math.toDegrees(dc_part.phi()));

            // Vertex z per sector datagroup.
            dg_vzsector.getH1F("hi_vz_dc_sec"+sector).fill(dc_part.vz());

            // Vertex momentum datagroup.
            dc_beta = beta;
            dg_vp.getH1F("hi_vp_dc").fill(dc_part.p());
            dg_vp.getH1F("hi_vbeta_dc").fill(dc_beta);
            dg_vp.getH2F("hi_vp_vbeta_dc").fill(dc_part.p(), dc_beta);
        }

        // === PROCESS FMT TRACKS ==================================================================
        if (fmt_tracks==null) continue;
        Particle fmt_part = new Particle(pid, fmt_tracks.getFloat("p0_x",   index),
                                              fmt_tracks.getFloat("p0_y",   index),
                                              fmt_tracks.getFloat("p0_z",   index),
                                              fmt_tracks.getFloat("Vtx0_x", index),
                                              fmt_tracks.getFloat("Vtx0_y", index),
                                              fmt_tracks.getFloat("Vtx0_z", index));
        double fchi2 = fmt_tracks.getFloat("chi2", index);
        int nmeas    = fmt_tracks.getInt("NDF", index);

        // Apply FMT cuts.
        if (nmeas != 3) continue; // only use track which pass through the three FMT layers.
        if (fmt_tracks.rows() < 1) continue; // abandon all hope if there are no FMT tracks.

        n_FMT_tracks++;
        if (sector==1) { // NOTE: No beam alignment yet, so we only use one sector for now.
            // Vertex z datagroup.
            dg_vz.getH1F("hi_vz_fmt")      .fill(fmt_part.vz());
            dg_vz.getH2F("hi_vz_theta_fmt").fill(fmt_part.vz(), Math.toDegrees(fmt_part.theta()));
            dg_vz.getH2F("hi_vz_phi_fmt")  .fill(fmt_part.vz(), Math.toDegrees(fmt_part.phi()));

            // Vertex z sector datagroup.
            dg_vzsector.getH1F("hi_vz_fmt_sec"+sector).fill(fmt_part.vz());

            // Vertex momentum datagroup.
            double fmt_beta = beta; // TODO: Figure out how to calculate this from FMT data.
            dg_vp.getH1F("hi_vp_fmt").fill(fmt_part.p());
            dg_vp.getH1F("hi_vbeta_fmt").fill(fmt_beta);
            dg_vp.getH2F("hi_vp_vbeta_fmt").fill(fmt_part.p(), fmt_beta);
        }
	}
}
reader.close();

System.out.printf("# of DC tracks:  %7d\n# of FMT tracks: %7d\n", n_DC_tracks, n_FMT_tracks);
// System.out.printf("% of dropped tracks: %2.5f",
//         ((double) (n_DC_tracks - n_FMT_tracks)) / ((double) n_DC_tracks));

// Setup plots and draw.
EmbeddedCanvasTabbed canvas = new EmbeddedCanvasTabbed(
        "z", "sector z", "p"
);

setup_canvas(canvas, "z",        dg_vz);
setup_canvas(canvas, "sector z", dg_vzsector);
setup_canvas(canvas, "p",        dg_vp);
// canvas.getCanvas("p").cd(0).getPad(1).getAxisY().setLog(true);
// canvas.getCanvas("p").cd(0).getPad(4).getAxisY().setLog(true);

JFrame frame = new JFrame("Acceptance Study Results");
frame.setSize(1500, 1000);
frame.add(canvas);
frame.setLocationRelativeTo(null);
frame.setVisible(true);

class Constants {
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
DataGroup gen_dg_vz(Constants C) {
    DataGroup dg = new DataGroup(3,2);

    // 1D vz.
    H1F hi_vz_dc  = new H1F("hi_vz_dc",  "DC vz (cm)" , "Counts", 500, -50, 50);
    H1F hi_vz_fmt = new H1F("hi_vz_fmt", "FMT vz (cm)", "Counts", 500, -50, 50);
    hi_vz_dc.setFillColor(43);
    hi_vz_fmt.setFillColor(44);
    dg.addDataSet(hi_vz_dc,  0);
    dg.addDataSet(hi_vz_fmt, 3);
    // TODO: Add upstream fit for DC and FMT tracks.

    // 2D vz vs theta.
    H2F hi_vz_theta_dc  = new H2F("hi_vz_theta_dc", 200, -50, 50, 100, 0, 40);
    H2F hi_vz_theta_fmt = new H2F("hi_vz_theta_fmt", 200, -50, 50, 100, 0, 40);
    hi_vz_theta_dc .setTitleX("DC vz (cm)");
    hi_vz_theta_fmt.setTitleX("FMT vz (cm)");
    hi_vz_theta_dc .setTitleY("#theta (deg)");
    hi_vz_theta_fmt.setTitleY("#theta (deg)");
    dg.addDataSet(hi_vz_theta_dc,  1);
    dg.addDataSet(hi_vz_theta_fmt, 4);

    // Draw lines showing FMT acceptance.
    F1D ftheta1 = new F1D("ftheta","57.29*atan([r]/([z0]-x))",  -50, 20.5);
    F1D ftheta2 = new F1D("ftheta2","57.29*atan([r]/([z0]-x))", -50, 3);
    ftheta1.setParameter(0, C.FMT_RMIN/10);
    ftheta2.setParameter(0, C.FMT_RMAX/10);
    ftheta1.setParameter(1, (C.FMT_Z[0]+C.FMT_DZ[0])/10);
    ftheta2.setParameter(1, (C.FMT_Z[0]+C.FMT_DZ[0])/10);
    ftheta1.setLineColor(2);
    ftheta2.setLineColor(2);
    ftheta1.setLineWidth(2);
    ftheta2.setLineWidth(2);
    dg.addDataSet(ftheta1, 1);
    dg.addDataSet(ftheta2, 1);
    dg.addDataSet(ftheta1, 4);
    dg.addDataSet(ftheta2, 4);

    // 2D vz vs phi angle.
    H2F hi_vz_phi_dc  = new H2F("hi_vz_phi_dc",  200, -50, 50, 100, -180, 180);
    H2F hi_vz_phi_fmt = new H2F("hi_vz_phi_fmt", 200, -50, 50, 100, -180, 180);
    hi_vz_phi_dc.setTitleX ("DC vz (cm)");
    hi_vz_phi_fmt.setTitleX("FMT vz (cm)");
    hi_vz_phi_dc.setTitleY ("#phi (deg)");
    hi_vz_phi_fmt.setTitleY("#phi (deg)");
    dg.addDataSet(hi_vz_phi_dc,  2);
    dg.addDataSet(hi_vz_phi_fmt, 5);

    // NOTE: fit
    // F1D f1 = new F1D("upstream fit",
    //       "[amp1]*gaus(x,[mean],[sigma])+[amp2]*gaus(x,[mean]-2.4,[sigma])+[p0]+[p1]*x+[p2]*x*x",
    //         -r, r);
    //
    // f1.setLineWidth(2);
    // f1.setLineColor(2);
    // f1.setOptStat("1111");
    // dg_vzsector[0].addDataSet(f1, 0);
    //
    // Data.fitUpstream(dg_vzsector[0].getH1F("FMT tracks"), dg_vzsector[0].getF1D("upstream fit"),
    //     -36, -30);
    // double amp = hires.getBinContent(hires.getMaximumBin());
    //
    // f1res.setParameter(0, 2*amp);  // amp1
    // f1res.setParameter(1, -32.085); // mean
    // f1res.setParameter(2, 0.2);  // sigma
    //
    // f1res.setRange(min, max);
    // DataFitter.fit(f1res, hires, "Q"); // No options use error for sigma.
    // hires.setFunction(null);

    return dg;
}
DataGroup gen_dg_vzsector() {
    DataGroup dg = new DataGroup(3,2);
    String xax = "z (cm) - sector ";
    String yax = "Counts";
    for (int sec=1; sec<=6; sec++) {
        H1F hi_vz_dc  = new H1F("hi_vz_dc_sec"  + sec, xax + sec, yax, 500, -50, 50);
        H1F hi_vz_fmt = new H1F("hi_vz_fmt_sec" + sec, xax + sec, yax, 500, -50, 50);
        hi_vz_dc .setFillColor(43);
        hi_vz_fmt.setFillColor(44);
        dg.addDataSet(hi_vz_dc,  sec-1);
        dg.addDataSet(hi_vz_fmt, sec-1);
        // Add a fit for DC and for FMT for each sector.
    }
    return dg;
}
DataGroup gen_dg_vp() {
    DataGroup dg = new DataGroup(3,2);

    // Momentum distribution.
    String xax = "p (GeV)";
    String yax = "Counts";
    H1F hi_vp_dc  = new H1F("hi_vp_dc",  "DC  " + xax, yax, 100, 0, 12);
    H1F hi_vp_fmt = new H1F("hi_vp_fmt", "FMT " + xax, yax, 100, 0, 12);
    hi_vp_dc .setFillColor(43);
    hi_vp_fmt.setFillColor(44);
    dg.addDataSet(hi_vp_dc,  0);
    dg.addDataSet(hi_vp_fmt, 3);

    // Beta distribution:
    xax = "#beta";
    yax = "Counts";
    H1F hi_vbeta_dc  = new H1F("hi_vbeta_dc",  "DC  " + xax, yax, 500, 0, 12);
    H1F hi_vbeta_fmt = new H1F("hi_vbeta_fmt", "FMT " + xax, yax, 500, 0, 12);
    hi_vbeta_dc .setFillColor(43);
    hi_vbeta_fmt.setFillColor(44);
    dg.addDataSet(hi_vbeta_dc,  1);
    dg.addDataSet(hi_vbeta_fmt, 4);

    // Momentum vs Beta:
    xax = "#beta";
    yax = "p (GeV)";
    H2F hi_vp_vbeta_dc  = new H2F("hi_vp_vbeta_dc",  200, 0, 12, 200, 0, 12);
    H2F hi_vp_vbeta_fmt = new H2F("hi_vp_vbeta_fmt", 200, 0, 12, 200, 0, 12);
    hi_vp_vbeta_dc.setTitleX ("DC  " + xax);
    hi_vp_vbeta_fmt.setTitleX("FMT " + xax);
    hi_vp_vbeta_dc.setTitleY (yax);
    hi_vp_vbeta_fmt.setTitleY(yax);
    dg.addDataSet(hi_vp_vbeta_dc,  2);
    dg.addDataSet(hi_vp_vbeta_fmt, 5);

    return dg;
}
int setup_canvas(EmbeddedCanvasTabbed canvas, String name, DataGroup tab) {
    canvas.getCanvas(name).draw(tab);
    canvas.getCanvas(name).setGridX(false);
    canvas.getCanvas(name).setGridY(false);
    canvas.getCanvas(name).setAxisFontSize(18);
    canvas.getCanvas(name).setAxisTitleSize(24);
    return 0;
}
