#!/bin/bash

data_directory=(/volatile/clas12/rg-e/production/calib/v1/calib/recon/020046/*)
working_directory=.
num_files=573
run_num=020046
for FILE in "${data_directory[@]::${num_files}}"
do
    echo "Processing file ${FILE}"
    shortened_file_name=$(echo $FILE| rev | cut -d'/' -f -1 | rev) # Removing directory from file name
    shortened_file_name=$(echo $shortened_file_name| rev | cut -d'.' -f -2 | rev) # String is now file_num.hipo
    file_num=$(echo $shortened_file_name | cut -d'.' -f 1) # Extracting file_num
    ${working_directory}/bin/hipo2root ${FILE}
    root_file="${working_directory}/root_io/banks_000000.root"
    ${working_directory}/bin/make_ntuples ${root_file}
    mv ${working_directory}/root_io/banks_000000.root ${working_directory}/root_io/banks_${run_num}_${file_num}.root
    mv ${working_directory}/root_io/ntuples_dc_000000.root ${working_directory}/root_io/ntuples_dc_${run_num}_${file_num}.root
    mv ${working_directory}/root_io/sf_study_000000.root ${working_directory}/root_io/sf_study_${run_num}_${file_num}.root
    mv ${working_directory}/data/sf_params_000000.txt ${working_directory}/data/sf_params_${run_num}_${file_num}.txt
done
