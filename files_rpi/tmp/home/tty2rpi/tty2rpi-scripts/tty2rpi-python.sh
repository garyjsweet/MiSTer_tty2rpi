#!/bin/bash

source ~/tty2rpi.ini
source ~/tty2rpi-user.ini

while true; do
  cd ~/tty2rpi-scripts
  python3 tty2rpi.py
done