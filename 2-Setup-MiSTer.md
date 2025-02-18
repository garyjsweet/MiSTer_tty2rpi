
Table of Contents
[Setting up MiSTer](#setting-up-mister)
[The INI files](#the-ini-files)
[Bugs and things to do](#bugs-and-things-still-to-do)
[License](#license)

---

# Setting up MiSTer

- Open a ssh-shell to your MiSTer or open a console at MiSTer and logon.
- Type in
```
wget -q https://raw.githubusercontent.com/ojaksch/MiSTer_tty2rpi/main/files_mister/update_tty2rpi.sh -O - | bash
```
This will download and setup the needed files for MiSTer. You can use *update_tty2rpi.sh* to keep this up to date.
Read The next chapter for a description of the used INI files and the last needed step to set and activate tty2rpi.

---

# The INI files

There are two INI files: tty2rpi-system.ini and tty2rpi-user.ini which are read and evaluated by the daemon in that order. tty2rpi-user.ini is overwriting values done by tty2rpi-system.ini
tty2rpi-system.ini contains the system wide variables and definitions. Do not edit this file as it will be overwritten when doing an update!
For your own favorite variables please use tty2rpi-user.ini and take over and edit the needed line from tty2rpi-system.ini. All useful variables are commented - see "# Userdata" in tty2rpi
-system.ini

# Current game / menu selection

When enabled in *MiSTer.ini* via the ```log_file_entry``` value, extra information about the currently selected game in the menusystem will be sent to the RPi for processing in addition to the standard core name. This provides details of the game being played on console cores for example rather than just the console name.

**IMPORTANT**
When setting up tty2rpi for the first time by running *update_tty2rpi.sh*, you'll see a notice to edit tty2rpi-user.ini and set the IP of your RPi there.
Change the value **IP-ADDRESS-OF-RPI** to the IP address or FQDN of your RPi and re-run update_tty2rpi.sh

---

# Bugs and things still to do

- No bugs for now (Yikes!)

---

# License

![CC BY-NC-SA 4.0](by-nc-sa.eu.png)
[Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
