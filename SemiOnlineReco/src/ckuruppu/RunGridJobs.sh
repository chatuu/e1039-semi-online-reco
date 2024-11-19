#!/bin/bash

python run_gridjob_final.py > $(date +%y-%m-%d_%H:%M:%S)_log.txt
