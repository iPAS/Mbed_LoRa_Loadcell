#!/bin/bash 
st-flash --reset write ./BUILD/IM880B/GCC_ARM/test-structure-health-monitoring-project.bin 0x08000000
