#!/bin/bash
#==============================================================================#
#                                                                              #     
#          FILE:  run_job.sh                                                   #                         
#                                                                              #                 
#         USAGE:  ./run_job.sh                                                 #                 
#                                                                              #                     
#   DESCRIPTION:  This script will initiate stress test to simulate            #                 
#                 reconstruction under detector conditions                     #                 
#                 using E906 data provided                                     #                 
#                                                                              #             
#       OPTIONS:  None                                                         #                     
#  REQUIREMENTS:  It is required to run GetAverageFAC.sh also need             # 
#                 following database: user_e1039_mainreco                      #                                                 
#          BUGS:  Not that I know of                                           #                                     
#         NOTES:  ---                                                          #                             
#        AUTHOR:  Chatura Kuruppu (Author), ckuruppu@fnal.gov                  #                 
#       COMPANY:  Fermi National Accelerator Laboratory                        #                 
#       VERSION:  1.0                                                          #                                     
#       CREATED:  2024-05-31                                                   #                             
#      REVISION:  0.0                                                          #                     
#==============================================================================#

# Command pattern to match
command_pattern="/data2/e1039/share/root/bin/root.exe -splash -l -b -q RecoE906Data.C"

# Max time in seconds to compare against
max_time=60  # Set your maximum time threshold here (e.g., 300 seconds or 5 minutes)

STEP=1.0
min_fac=3.0
# Temporary file to save the FAC value
temp_file="/tmp/fac_value.txt"

if [ -f "$temp_file" ]; then
    source "$temp_file"
else
    # Initial FAC value (set this to the initial value or read from a file if needed)
    FAC=5.0  # Set your initial FAC value here
    echo FAC=$FAC > /tmp/fac_value.txt
fi

# Get the PIDs and elapsed times of the processes matching the command pattern
process_info=$(ps -eo pid,etime,cmd | grep "$command_pattern" | grep -v grep)

# Check if there are any matching processes
if [ -z "$process_info" ]; then
    echo "No processes found matching the command pattern: $command_pattern"
    exit 0
fi

# Initialize variables
total_time=0
process_count=0

# Convert elapsed time to seconds
convert_to_seconds() {
    local etime=$1
    local seconds=0

    # Strip leading zeros from each time component
    etime=$(echo "$etime" | sed 's/^0*//; s/:0*/:/g; s/-0*/-/g')

    if [[ $etime =~ ([0-9]+)-([0-9]+):([0-9]+):([0-9]+) ]]; then
        seconds=$(( (${BASH_REMATCH[1]} * 86400) + (${BASH_REMATCH[2]} * 3600) + (${BASH_REMATCH[3]} * 60) + ${BASH_REMATCH[4]} ))
    elif [[ $etime =~ ([0-9]+):([0-9]+):([0-9]+) ]]; then
        seconds=$(( (${BASH_REMATCH[1]} * 3600) + (${BASH_REMATCH[2]} * 60) + ${BASH_REMATCH[3]} ))
    elif [[ $etime =~ ([0-9]+):([0-9]+) ]]; then
        seconds=$(( (${BASH_REMATCH[1]} * 60) + ${BASH_REMATCH[2]} ))
    elif [[ $etime =~ ([0-9]+) ]]; then
        seconds=${BASH_REMATCH[1]}
    fi

    echo $seconds
}

# Parse process information
while IFS= read -r line; do
    pid=$(echo "$line" | awk '{print $1}')
    etime=$(echo "$line" | awk '{print $2}')
    cmd=$(echo "$line" | awk '{print substr($0, index($0,$3))}')
    
    elapsed_seconds=$(convert_to_seconds "$etime")
    total_time=$((total_time + elapsed_seconds))
    process_count=$((process_count + 1))
done <<< "$process_info"

# Calculate average time
if [ $process_count -gt 0 ]; then
    average_time=$((total_time / process_count))
    echo "Average time: $average_time seconds"
    
    # Check if average_time exceeds max_time
    if [ "$average_time" -gt "$max_time" ]; then
        FAC=5.0
        echo "Average time exceeds $max_time seconds. Setting FAC to $FAC."
    elif [ "$average_time" -lt "$max_time" ]; then
        # Decrease FAC by STEP if it will not go below min_fac
        new_fac=$(echo "$FAC - $STEP" | bc)
        if (( $(echo "$new_fac > $min_fac" | bc -l) )); then
            FAC=$new_fac
            echo "Average time is less than $max_time seconds. Decreasing FAC by $STEP to $FAC."
        else
            FAC=$min_fac
            echo "Average time is less than $max_time seconds. FAC cannot go below $min_fac. Setting FAC to $FAC."
        fi
    fi

    # Save FAC value to temporary file
    echo "FAC=$FAC" > "$temp_file"
    echo "FAC value saved to $temp_file."
else
    echo "No running processes found for the command."
fi

