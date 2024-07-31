use strict;
use warnings;

our %configuration;

sub materials
{
	my $thisVariation = $configuration{"variation"} ;
	my %mat = init_mat();
	# This conditional serves to build materials for all targets
	if($thisVariation ne " ")
	{
		# rohacell
		$mat{"name"}          = "rohacell";
		$mat{"description"}   = "target  rohacell scattering chamber material";
		$mat{"density"}       = "0.1";  # 100 mg/cm3
		$mat{"ncomponents"}   = "4";
		$mat{"components"}    = "G4_C 0.6465 G4_H 0.0784 G4_N 0.0839 G4_O 0.1912";
		print_mat(\%configuration, \%mat);

		# epoxy
		%mat = init_mat();
		$mat{"name"}          = "epoxy";
		$mat{"description"}   = "epoxy glue 1.16 g/cm3";
		$mat{"density"}       = "1.16";
		$mat{"ncomponents"}   = "4";
		$mat{"components"}    = "H 32 N 2 O 4 C 15";
		print_mat(\%configuration, \%mat);

		# carbon fiber
		%mat = init_mat();
		$mat{"name"}          = "carbonFiber";
		$mat{"description"}   = "ft carbon fiber material is epoxy and carbon - 1.75g/cm3";
		$mat{"density"}       = "1.75";
		$mat{"ncomponents"}   = "2";
		$mat{"components"}    = "G4_C 0.745 epoxy 0.255";
		print_mat(\%configuration, \%mat);

		# torlon4435
		%mat = init_mat();
		$mat{"name"}          = "torlon4435";
		$mat{"description"}   = "torlon4435 (my guess) g/cm3";
		$mat{"density"}       = "1.59";
		$mat{"ncomponents"}   = "5";
		$mat{"components"}    = "H 4 N 2 O 3 C 9 Ar 1";
		print_mat(\%configuration, \%mat);

		# torlon4203
		%mat = init_mat();
		$mat{"name"}          = "torlon4203";
		$mat{"description"}   = "torlon4203 (my guess) g/cm3";
		$mat{"density"}       = "1.41";
		$mat{"ncomponents"}   = "5";
		$mat{"components"}    = "H 4 N 2 O 3 C 9 Ar 1";
		print_mat(\%configuration, \%mat);
	}
}
