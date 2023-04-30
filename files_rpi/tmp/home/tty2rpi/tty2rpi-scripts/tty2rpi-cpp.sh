#!/bin/bash

# Set a 24 bit framebuffer to improve copy performance a little
fbset -depth 24

# Configure the three GPIO pins we will use for display menu buttons
raspi-gpio set 16 ip pu
sleep 1
raspi-gpio set 20 ip pu
sleep 1
raspi-gpio set 21 ip pu
sleep 1

while true; do
  cd ~/tty2rpi-cpp/build
  ./tty2rpi-cpp
done