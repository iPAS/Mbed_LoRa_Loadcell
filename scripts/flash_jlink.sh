#!/bin/bash 

PROJECT_PATH="./scripts/im980a.jflash"
BINARY_PATH="./BUILD/IM880B/GCC_ARM/test-structure-health-monitoring-project.bin"
LOG_FILE="jflash_log.log"

JFlash -openprj"${PROJECT_PATH}" -open"${BINARY_PATH}",0x8000000 -jflashlog${LOG_FILE}  -min -auto -startapp -exit
# -hide