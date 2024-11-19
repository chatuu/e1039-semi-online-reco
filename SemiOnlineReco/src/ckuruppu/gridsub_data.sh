#!/bin/bash
export GROUP=spinquest
export IFDHC_VERSION=v2_6_20
echo 'Chatura.Kuruppu' | kinit -f ckuruppu@FNAL.GOV
#export HTGETTOKENOPTS="--credkey=spinquestpro/managedtokens/fifeutilgpvm01.fnal.gov"
#export X509_USER_PROXY=/opt/spinquestpro/spinquestpro.Production.proxy
export HTGETTOKENOPTS="-a htvaultprod.fnal.gov -i spinquest -r default --credkey=ckuruppu"
htgettoken $HTGETTOKENOPTS
source /exp/seaquest/app/software/script/jobsub_submit_spinquest.sh

dir_macros=$(dirname $(readlink -f $BASH_SOURCE))
LIFE_TIME=long # short (3h), medium (8h) or long (23h)

jobname=$1
do_sub=$2
run_name=$3
nevents=$4
dst_mode=${5:-'splitting'} # 'splitting' or 'single'
resub_file=${6:-'null'} #file for resubmitting run
source /exp/seaquest/app/software/script/jobsub_submit_spinquest.sh
if [ $do_sub == 1 ]; then
    echo "Grid mode."
    if ! which jobsub_submit &>/dev/null ; then
	echo "Command 'jobsub_submit' not found."
	echo "Forget 'source /exp/seaquest/app/software/script/setup-jobsub-spinquest.sh'?"
	exit
    #source /exp/seaquest/app/software/script/setup-jobsub-spinquest.sh
    fi
    #work=/pnfs/e1039/persistent/cosmic_recodata/$jobname
    #work=/pnfs/e1039/persistent/users/spinquestpro/Harsha/NewGeometry/cosmic_recodata/$jobname
    #work=/pnfs/e1039/persistent/users/spinquestpro/cosmic_recodata/$jobname
    #work=/pnfs/e1039/persistent/users/spinquestpro/commisioning_recodata/recofiles/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/KMagOn/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/KMagOff/Occu4/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/KMagOff/Occu3/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/GoodRuns/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/CoarseFalse/Occu3/KMagOff/KalmanFalse/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/CoarseFalse/Occu3/KMagOff/KalmanTrue/$jobname
    work=/pnfs/e1039/scratch/users/ckuruppu/commisioning_recodata/CoarseFalse/Occu3/KMagOn/HighStatisticsCheck/DC3p/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/KMagOn/Occu1/$jobname
    #work=/pnfs/e1039/persistent/users/ckuruppu/commisioning_recodata/KMagOn/Occu4/$jobname
    #work=/pnfs/e1039/persistent/users/spinquestpro/commisioning_recodata/test/NoOccupancy/$jobname
    #work=/pnfs/e1039/persistent/users/spinquestpro/commisioning_recodata/test/Occupancy-5/afterMapping/$jobname
    #work=/pnfs/e1039/persistent/users/spinquestpro/commisioning_recodata/test/Occupancy-5/RecoFiles/$jobname
    # ln -sf /pnfs/e906/persistent/cosmic_recodata data
else
    echo "Local mode."
    work=$dir_macros/scratch/$jobname
fi

#location of the decoded data
#data_dir="/pnfs/e1039/tape_backed/decoded_data"
data_dir="/pnfs/e1039/scratch/users/ckuruppu/tape_backed/decoded_data"

if [ "$resub_file" = "null" ]; then

    mkdir -p $work
    chmod -R 01755 $work

    echo $work
    echo $dir_macros

    echo "This is a test......................................!"

    #tar -C $dir_macros -czvf $work/input.tar.gz geom.root RecoE1039DataKMagOn.C
    #tar -C $dir_macros -czvf $work/input.tar.gz geom.root alignment RecoE1039DataKMagOff.C
    tar -C $dir_macros -czvf $work/input.tar.gz geom*.root alignment RecoE1039Data*.C


    #declare -a data_path_list=()
    if [ $dst_mode = 'single' ] ; then
	data_path_list=( $data_dir/$(printf 'run_%06d_spin.root' $run_name) )
    else # 'splitting'     
	data_path_list=( $(find $data_dir -name $(printf 'run_%06d_spill_*_spin.root' $run_name) ) )
	echo $data_path_list
    fi

else
 
    #data_path_list=( $(find $data_dir -name  $resub_file ) )
    data_path_list=$resub_file
   
fi #resub_file condition

for data_path in ${data_path_list[*]} ; do
    
    data_file=$(basename $data_path)
    job_name=${data_file%'.root'}

    if [ "$resub_file" = "null" ]; then
	mkdir -p $work/$job_name/log
	mkdir -p $work/$job_name/out
	chmod -R 01755 $work/$job_name
    fi
    
    rsync -av $dir_macros/gridrun_data.sh $work/$job_name/gridrun_data.sh

    if [ $do_sub == 1 ]; then
	cmd="jobsub_submit --debug -G spinquest --onsite-only "
	cmd="$cmd -l '+SingularityImage=\"/cvmfs/seaquest.opensciencegrid.org/seaquest/software/e1039/sin/e1039-al9\"'"
	cmd="$cmd --append_condor_requirements='(TARGET.HAS_SINGULARITY=?=true)'"
	cmd="$cmd --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC,OFFSITE -e IFDHC_VERSION --disk 8GB --memory 8GB --expected-lifetime=8h"
	cmd="$cmd --mail_never"
	cmd="$cmd -L $work/$job_name/log/log.txt"
	cmd="$cmd -f $work/input.tar.gz"
	cmd="$cmd -d OUTPUT $work/$job_name/out"
	#cmd="$cmd -f $dir_macros/$infile"
	cmd="$cmd -f $data_path"
	cmd="$cmd file://$work/$job_name/gridrun_data.sh $run_name $data_file 0"
	echo "$cmd"
	$cmd


    else
	mkdir -p $work/$job_name/input
	rsync -avP $work/input.tar.gz $data_path  $work/$job_name/input
	cd $work/$job_name/
	bash $work/$job_name/gridrun_data.sh $nevents $run_name $data_file | tee $work/$job_name/log/log.txt
	cd -
    fi

done 2>&1 | tee $dir_macros/log_gridsub.txt
