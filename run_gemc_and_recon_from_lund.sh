#!/bin/bash
#!/bin/bash
#
#SBATCH --job-name=gemc_rge
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
####  SBATCH --partition=short
#SBATCH --time=01:00:00


#SBATCH --array=0-999
#SBATCH --mem-per-cpu=4000
##SBATCH --mail-type=FAIL
##SBATCH --mail-user=sebouh.paul@gmail.com
i=$SLURM_ARRAY_TASK_ID
A=$1

if [[ -z $i ]]; then
    i=$2
fi

if [[ -n $3 ]]; then
    N=$3
else
    N=1000
fi
#source /group/clas12/packages/setup.sh
#module load clas12
module use /scigroup/cvmfs/hallb/clas12/sw/modulefiles
module load clas12
mkdir -p /work/clas12/spaul/rge-sim

#GCARD=/home/spaul/clas12-config/gemc/5.4/clas12-default.gcard 
#RECON_YAML=/group/clas12/packages/coatjava/10.0.4/config/mc-cv.yaml

#GCARD=/home/spaul/clas12-config/gemc/5.9/clas12-default.gcard
GCARD=/home/spaul/clas12-rge-analysis/clas12_fmt_cryoresize.gcard
RECON_YAML=/home/spaul/clas12-rge-analysis/clas12.yaml
RECON_YAML=$COATJAVA/etc/services/mc-cv.yaml

if [[ $A -eq "D" ]]; then
    VTX_ARGS=-RANDOMIZE_LUND_VZ="-6.03*cm,1*cm"
    #VTX_ARGS=-BEAM_V="(0, 0, -3)cm" -SPREAD_V="(0, 0, 1)cm"
else 
    VTX_ARGS=-RANDOMIZE_LUND_VZ="-1*cm,0*cm"
    #VTX_ARGS=-BEAM_V="(0, 0, 1)cm" -SPREAD_V="(0, 0, 0)cm"
fi


lund_file=/work/clas12/spaul/DIS_events/DIS_events_${i}.lund
sim_file=/work/clas12/spaul/rge-sim/rge-sim_${A}_${i}.hipo
reco_file=/work/clas12/spaul/rge-sim/rge-sim_${A}_recon_${i}.hipo
log_file=/work/clas12/spaul/rge-sim/${A}_${i}.log
err_file=/work/clas12/spaul/rge-sim/${A}_${i}.err

rm $sim_file $reco_file $log_file $err_files

module unload ccdb
module load gemc
gemc $GCARD -USE_GUI=0 -INPUT_GEN_FILE="LUND, "${lund_file}  -USE_GUI=0 -OUTPUT="hipo, "${sim_file} -N=$N $VTX_ARGS > $log_file 2> $err_file
module unload gemc
module load clas12
recon-util -i $sim_file -o $reco_file -y $RECON_YAML >> $log_file 2>> $err_file



tmpdir=/work/clas12/spaul/rge-sim/tmp_${i}_${A}
mkdir -p $tmpdir
cd /home/spaul/clas12-rge-analysis/
bin/hipo2root -w $tmpdir $reco_file >> $log_file 2>> $err_file
bin/make_ntuples -w $tmpdir $tmpdir/banks_000000.root >> $log_file 2>> $err_file
#cp $tmpdir/banks_000000.root /work/clas12/spaul/rge-sim/rge-sim_${A}_tuples_${i}.root

