#!/bin/bash

# Define MySQL configuration file path
config_file="/data2/e1039/resource/db_conf/my_db1.cnf"

# Define the database and table names
database="user_e1039_mainreco"
table1="e1039_stresstest"
table2="e1039_recostat"
logdir=./logfiles
temp_file="/tmp/fac_value.txt"

if [ -f "$temp_file" ]; then
    source "$temp_file"
else
# Initial FAC value (set this to the initial value or read from a file if needed)
    FAC=5.0  # Set your initial FAC value here
fi


GetFac()
{
temp_file="/tmp/fac_value.txt"

if [ -f "$temp_file" ]; then
    source "$temp_file"
else
# Initial FAC value (set this to the initial value or read from a file if needed)
    FAC=5.0  # Set your initial FAC value here
fi
}
# Execute the SQL query to select entries
#mysql --defaults-file="$config_file" --batch --execute "
#USE $database;
#SELECT table1.run_id, table1.spill_id, table1.file_name
#FROM $table1 AS table1
#LEFT JOIN $table2 AS table2
#ON table1.run_id = table2.run_id AND table1.spill_id = table2.spill_id
#WHERE table2.run_id IS NULL AND table2.spill_id IS NULL AND table1.status = 2;
#"

mysql --defaults-file="$config_file" --batch --execute "
USE $database;
SELECT table1.run_id, table1.spill_id, table1.file_name
FROM $table1 AS table1
LEFT JOIN $table2 AS table2
ON table1.run_id = table2.run_id AND table1.spill_id = table2.spill_id
WHERE table2.run_id IS NULL AND table2.spill_id IS NULL AND table1.status = 2;
" | tail -n +2 | parallel --delay 30 -j+0 --eta --colsep '\t' 'source /tmp/fac_value.txt && mkdir -p "/data4/e1039_data/semi_online_reco/run_{1}/spill_{2}/" && root -l -b -q "RecoE906Data.C({1},{2},\"{3}\",${FAC})" > /dev/null 2>&1'

# Parse the result and save run_id and spill_id to variables
#while read -r run_id spill_id file; do
    # Use run_id and spill_id as needed
    #echo "run_id: $run_id, spill_id: $spill_id file: $file"
    #./RunParallel.sh $run_id $spill_id $file
    #sleep 60
    # You can perform further operations with the variables here
#done <<< "$result"
