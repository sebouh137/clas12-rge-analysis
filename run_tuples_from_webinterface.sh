#!/bin/bash
#!/bin/bash
#
#SBATCH --job-name=gemc_rge
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
####  SBATCH --partition=short
#SBATCH --time=00:20:00


#SBATCH --array=0-9999
#SBATCH --mem-per-cpu=4000
##SBATCH --mail-type=FAIL
##SBATCH --mail-user=sebouh.paul@gmail.com
i=$SLURM_ARRAY_TASK_ID
tag=$1
jobid=$2

#for debugging
if [[ -z $i ]]; then
    i=$3
fi


#source /group/clas12/packages/setup.sh
#module load clas12
module use /scigroup/cvmfs/hallb/clas12/sw/modulefiles
module load clas12
workdir=/work/clas12/spaul/rge-sim-output/${tag}_${jobid}
mkdir -p $workdir
mkdir -p $workdir/log
mkdir -p $workdir/rec
mkdir -p $workdir/tuples

#RECON_YAML=$COATJAVA/etc/services/mc-cv.yaml

reco_file=/volatile/clas12/osg/spaul/job_${jobid}/output/*-${i}.hipo
log_file=$workdir/log/${i}.log
err_file=$workdir/log/${i}.err

rm  $log_file $err_files

module load clas12

tmpdir=$workdir/tmp/${i}
mkdir -p $tmpdir
cd /home/spaul/clas12-rge-analysis/
echo hipo2root
ls $reco_file
bin/hipo2root -w $tmpdir $reco_file >> $log_file 2>> $err_file
echo make_tuples
bin/make_ntuples -w $tmpdir $tmpdir/banks_*.root >> $log_file 2>> $err_file
echo move output
cp $tmpdir/ntuples_dc_*.root $workdir/tuples/rge-sim_tuples_${i}.root

