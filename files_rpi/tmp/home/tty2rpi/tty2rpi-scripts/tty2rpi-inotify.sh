#!/bin/bash

source ~/tty2rpi.ini
source ~/tty2rpi-user.ini

while true; do
  inotifywait -qq -e modify ${SOCKET}
  COMMAND=$(tail -n1 ${SOCKET} | tr -d '\0')
  if [ "${COMMAND}" != "NIL" ]; then					# NIL is a "hello?" command
    ! [ "$(<${PID_TTY2RPI})" = "0" ] && KILLPID $(<${PID_TTY2RPI})
    #KILLPID "feh"
    #KILLPID "vlc"
    sync ${SOCKET}
    ~/tty2rpi-scripts/tty2rpi.sh & echo $! > ${PID_TTY2RPI}
  fi
done
