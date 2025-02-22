use strict;
use warnings;

our %configuration;
our %parameters;

sub build_targets
{
	my $thisVariation = $configuration{"variation"} ;

	if($thisVariation eq "eg2-C" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";

		# move targets by -6 cm relative to what it was before
		# so that the solid target is at z=-1*cm
		# --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 1.4890/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "251607";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_GRAPHITE";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-C-lD2" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";

		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
                $detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		# Solid target
		my $thickness  = 1.4890/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "251607";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_GRAPHITE";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);

		# Liquid deuterium
		$nplanes = 5;
		my @oradiusT  =  (   2.5,  0.8 * 10.3,  7.3, 5.0,  2.5);
		my @z_planeT  =  ( -9.2, -6.2, 7.5, 8.5, 9.5);

		%detector = init_det();
		$detector{"name"}        = "lD2";
		$detector{"mother"}      = "target";
		$detector{"description"} = "Target Cell";
		$detector{"color"}       = "aa0000";
		$detector{"type"}        = "Polycone";
		$dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradiusT[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_planeT[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "LD2"; # defined in gemc database
		$detector{"style"}       = 1;
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Al" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";

		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 1.2133/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "D9DAD9";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Al";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Al-lD2" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";
		
		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 1.2133/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "D9DAD9";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Al";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);

		# Liquid deuterium
		$nplanes = 5;
		my @oradiusT  =  (   2.5,  0.8 * 10.3,  7.3, 5.0,  2.5);
		my @z_planeT  =  ( -9.2, -6.2, 7.5, 8.5, 9.5);

		%detector = init_det();
		$detector{"name"}        = "lD2";
		$detector{"mother"}      = "target";
		$detector{"description"} = "Target Cell";
		$detector{"color"}       = "aa0000";
		$detector{"type"}        = "Polycone";
		$dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradiusT[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_planeT[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "LD2"; # defined in gemc database
		$detector{"style"}       = 1;
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Cu" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";

		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 0.3619/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "D1793B";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Cu";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Cu-lD2" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";
		
		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 0.3619/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "D1793B";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Cu";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);

		# Liquid deuterium
		$nplanes = 5;
		my @oradiusT  =  (   2.5,  0.8 * 10.3,  7.3, 5.0,  2.5);
		my @z_planeT  =  ( -9.2, -6.2, 7.5, 8.5, 9.5);

		%detector = init_det();
		$detector{"name"}        = "lD2";
		$detector{"mother"}      = "target";
		$detector{"description"} = "Target Cell";
		$detector{"color"}       = "aa0000";
		$detector{"type"}        = "Polycone";
		$dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradiusT[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_planeT[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "LD2"; # defined in gemc database
		$detector{"style"}       = 1;
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Sn" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";
		
		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 0.3032/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "d3d4d5";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Sn";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Sn-lD2" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";

		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 0.3032/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "solidSn";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Sn";
		$detector{"color"}       = "d3d4d5";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Sn";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);

		# Liquid deuterium
		$nplanes = 5;
		my @oradiusT  =  (   2.5,  0.8 * 10.3,  7.3, 5.0,  2.5);
		my @z_planeT  =  ( -9.2, -6.2, 7.5, 8.5, 9.5);

		%detector = init_det();
		$detector{"name"}        = "lD2";
		$detector{"mother"}      = "target";
		$detector{"description"} = "Target Cell";
		$detector{"color"}       = "aa0000";
		$detector{"type"}        = "Polycone";
		$dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradiusT[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_planeT[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "LD2"; # defined in gemc database
		$detector{"style"}       = 1;
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Pb" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";

		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 0.1409/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "Pb";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Pb";
		$detector{"color"}       = "6c6c6a";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Pb";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);
	}
	if($thisVariation eq "eg2-Pb-lD2" )
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";

		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);

		my $thickness  = 0.1409/2.;
		my $zpos       = 50;
		my $radius     = 2.5;
		$detector{"name"}        = "Pb";
		$detector{"mother"}      = "target";
		$detector{"description"} = "eg2 double-target Pb";
		$detector{"color"}       = "6c6c6a";
		$detector{"type"}        = "Tube";
		$detector{"pos"}         = "0 0 $zpos*mm";
		$detector{"dimensions"}  = "0*mm $radius*mm $thickness*mm 0*deg 360*deg";
		$detector{"material"}    = "G4_Pb";
		$detector{"sensitivity"} = "flux";
		$detector{"hit_type"}    = "flux";
		$detector{"identifiers"} = "solidID";
		$detector{"style"}       = "1";
		print_det(\%configuration, \%detector);

		# Liquid deuterium
		$nplanes = 5;
		my @oradiusT  =  (   2.5,  0.8 * 10.3,  7.3, 5.0,  2.5);
		my @z_planeT  =  ( -9.2, -6.2, 7.5, 8.5, 9.5);

		%detector = init_det();
		$detector{"name"}        = "lD2";
		$detector{"mother"}      = "target";
		$detector{"description"} = "Target Cell";
		$detector{"color"}       = "aa0000";
		$detector{"type"}        = "Polycone";
		$dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradiusT[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_planeT[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "LD2"; # defined in gemc database
		$detector{"style"}       = 1;
		print_det(\%configuration, \%detector);

	}

	# cad variation has two volume:
	# target container
	# and inside cell
	if($thisVariation eq "lD2")
	{
		my $nplanes = 4;

		my @oradius  =  (    50.3,   50.3,  21.1,  21.1 );
		# NOMINAL : (  -140.0,  265.0, 280.0, 280.0 );
		my @z_plane  =  (  -240.0,  265.0, 280.0, 280.0 );

		# vacuum target container
		my %detector = init_det();
		$detector{"name"}        = "target";
		$detector{"mother"}      = "root";
		$detector{"description"} = "Target Container";
		$detector{"color"}       = "22ff22";
		$detector{"type"}        = "Polycone";
		
		# move targets by -6 cm relative to what it was before
                # so that the solid target is at z=-1*cm
                # --Sebouh Paul
		$detector{"pos"} = "0.0*mm 0.0*mm -6.0*cm";
		my $dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradius[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_plane[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "G4_Galactic";
		$detector{"style"}       = 0;
		print_det(\%configuration, \%detector);


		$nplanes = 5;
		my @oradiusT  =  (   2.5,  0.8 * 10.3,  7.3, 5.0,  2.5);
		my @z_planeT  =  ( -9.2, -6.2, 7.5, 8.5, 9.5);

		# actual target
		%detector = init_det();
		$detector{"name"}        = "lD2";
		$detector{"mother"}      = "target";
		$detector{"description"} = "Target Cell";
		$detector{"color"}       = "aa0000";
		$detector{"type"}        = "Polycone";
		$dimen = "0.0*deg 360*deg $nplanes*counts";
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." 0.0*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $oradiusT[$i]*mm";}
		for(my $i = 0; $i <$nplanes; $i++) {$dimen = $dimen ." $z_planeT[$i]*mm";}
		$detector{"dimensions"}  = $dimen;
		$detector{"material"}    = "LD2"; # defined in gemc database
		$detector{"style"}       = 1;
		print_det(\%configuration, \%detector);

	}
}

1;






















