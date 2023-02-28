#!/bin/bash

source ~/tty2rpi.ini
source ~/tty2rpi-user.ini

CORENAME="${COMMANDLINE[1]}"
FULLPATH="${COMMANDLINE[2]}"
CURPATH="${COMMANDLINE[3]}"

logger "Socket got »${CORENAME}«,»${FULLPATH}«,»${CURPATH}«"

# Just echo the data to our local ramdisk corename.
# This will be picked up by the python script.
echo "${CORENAME}>${FULLPATH}>${CURPATH}" > /dev/shm/corename
