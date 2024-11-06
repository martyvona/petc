#!/bin/bash

PORT=/dev/cu.usbserial-110 
if [[ $# -gt 0 ]]; then PORT=$1; fi

arduino-cli compile --fqbn arduino:avr:uno -e -u -p $PORT .

