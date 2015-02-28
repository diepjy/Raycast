#!/bin/bash

# script to run the raycast benchmark for the ACA15 exercise
# Paul Kelly 2015

# Usage example: ./runRaycast 10 8

# To run the benchmark 10 times.  Choose a number of repeats that ensures
# that the total time being measured is large enough to be measured reliably.
# The important performance metric is the time *per repeat*

NREPEATS=$1

# The second parameter (8 above) controls the number of threads to use.
# The program uses OpenMP to execute in parallel across the cores of a 
# multicore CPU.  You can control how many cores it uses by varying this 
# parameter, with 1 being serial.

export OMP_NUM_THREADS="$2"

# The other parameters for the raycast program provide input data 
# (RaycastInVolume RaycastInPos) and 
# files containing the expected output data for validation 
# (RaycastOutVertex  RaycastOutNormal).  Do not disable validation,
# and do not report results if validation fails.

./raycast   RaycastInVolume RaycastInPos RaycastOutVertex  RaycastOutNormal 640 480 0.400000006 4 0.0078125  0.07500000298 256 2 $NREPEATS
