#!/bin/bash

source ~/tty2rpi.ini
source ~/tty2rpi-user.ini

while true; do
  python3 ~/tty2rpi-scripts/test.py
done