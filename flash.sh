#!/bin/bash 
st-flash --reset write BUILD/XDOT_L151CC/GCC_ARM/test.bin 0x08000000
