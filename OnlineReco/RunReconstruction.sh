#!/bin/bash
#==============================================================================#
#                                                                              #
#          FILE:  RunReconstruction.sh                                         #
#                                                                              #
#         USAGE:  ./RunReconstruction.sh                                       #
#                                                                              #
#   DESCRIPTION:  This script will continuously run run_job.sh script          #
#                 within the given intervals                                   #
#                                                                              #
#                                                                              #
#       OPTIONS:  None                                                         #
#  REQUIREMENTS:  It is required to run RunReconstruction.sh                   #
#                                                                              #
#          BUGS:  Not that I know of                                           #
#         NOTES:  ---                                                          #
#        AUTHOR:  Chatura Kuruppu (Author), ckuruppu@fnal.gov                  #
#       COMPANY:  Fermi National Accelerator Laboratory                        #
#       VERSION:  1.0                                                          #
#       CREATED:  2024-06-15                                                   #
#      REVISION:  0.0                                                          #
#==============================================================================#
loc=`pwd`
log_file=$loc/RecoLogs.txt

# Interval in seconds (900 seconds = 15 minutes)
interval=900

# Infinite loop to run the script every interval and append the output to the log file
while true; do
    echo "Running run_job.sh at $(date +"%Y-%m-%d %H:%M:%S")" >> "$log_file"
    #./run_job_1.sh >> "$log_file" 2>&1
    ./run_job_1.sh >> "$log_file"
    echo "Sleeping for $interval seconds..." >> "$log_file"
    sleep $interval
done
