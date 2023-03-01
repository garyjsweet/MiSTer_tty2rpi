#!/bin/bash

# By venice & ojaksch
#
#
#

sysinifile=/media/fat/tty2rpi/tty2rpi-system.ini
userinifile=/media/fat/tty2rpi/tty2rpi-user.ini

. ${sysinifile}
. ${userinifile}

cd /tmp

debug=false

# Debug function
dbug() {
  if [ "${debug}" = "true" ]; then
    echo "${1}"
    if [ ! -e ${debugfile} ]; then                                                # log file not (!) exists (-e) create it
      echo "---------- tty2rpi Debuglog ----------" > ${debugfile}
    fi
    echo "${1}" >> ${debugfile}                                                        # output debug text
  fi
}

# USB Send-Picture-Data function
senddata() {
  echo "${1}" > ${TTYDEV}
  #sleep ${WAITSECS}
}

# Encode spaces as "%20" before sending
encodespaces() {
  echo "${1}" | sed -e 's/ /%20/g' | sed -e 's/,/%2C/g'
}

# ** Main **
# Check for Command Line Parameter
if [ "${#}" -ge 1 ]; then                                                        # Command Line Parameter given, override Parameter
  echo -e "\nUsing Command Line Parameter"
  TTYDEV=${1}                                                                        # Set TTYDEV with Parameter 1
  echo "Using Interface: ${TTYDEV}"                                                # Device Output
fi                                                                                # end if command line Parameter

# Let's go

# Touch the files we want to watch. If they don't exist when we wait, the creation won't trigger a notify
touch ${corenamefile}
touch ${currentpathfile}
touch ${fullpathfile}
touch ${startpathfile}
touch ${selectfile}
touch ${samgamefile}

if [ "${debug}" = "true" ]; then
  quiet="-qq"
fi

dbug "${TTYDEV} detected, setting Parameters."
sleep ${WAITSECS}

while true; do                                                                # main loop
  newcore=$(encodespaces "$(cat ${corenamefile})")                                              # get CORENAME
  dbug "Read CORENAME: -${newcore}-"
  newfullpath=$(encodespaces "$(cat ${fullpathfile})")
  dbug "Read FULLPATH: -${newfullpath}-"
  newstartpath=$(encodespaces "$(cat ${startpathfile})")
  dbug "Read STARTPATH: -${newstartpath}-"
  newsamgame=$(encodespaces "$(cat ${samgamefile})")
  dbug "Read SAM_Game.txt: -${newsamgame}-"
  newfileselect=$(encodespaces "$(cat ${selectfile})")
  dbug "Read FILESELECT: -${newfileselect}-"
  newcurpath=$(encodespaces "$(cat ${currentpathfile})")
  dbug "Read CURPATH: -${newcurpath}-"

  if [ "${newcore}" != "${oldcore}" ] ||
     [ "${newcurpath}" != "${oldcurpath}" ] ||
     [ "${newfullpath}" != "${oldfullpath}" ] ||
     [ "${newstartpath}" != "${oldstartpath}" ] ||
     [ "${newfileselect}" != "${oldfileselect}" ] ||
     [ "${newsamgame}" != "${oldsamegame}" ]; then                                        # proceed only if Core has changed
    dbug "Send -${newcore}- -${newfullpath}- -${newcurpath}- -${newstartpath}- -${newfileselect}- -${newsamgame}- ${TTYDEV}."
    senddata "CMDCOREXTRA,${newcore},${newfullpath},${newcurpath},${newstartpath},${newfileselect},${newsamgame}"                                                # The "Magic"
    oldcore="${newcore}"                                                        # update oldcore variable
    oldcurpath="${newcurpath}"
    oldfullpath="${newfullpath}"
    oldstartpath="${newstartpath}"
    oldfileselect="${newfileselect}"
    oldsamgame="${newsamgame}"
  else                                                                        # end if core check
    . ${userinifile}                                            # ReRead INI for changes
    inotifywait ${quiet} -e modify -e create "${userinifile}" "${currentpathfile}" "${newfileselect}"
  fi
  sleep 0.1

done                                                                                # end while
# ** End Main **
