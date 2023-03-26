# MiSTer_tty2rpi-cpp
WIP : A rewrite of [tty2rpi](https://github.com/ojaksch/MiSTer_tty2rpi) in C++.

Uses a modified version of the socket-based communication from the MiSTer along
with a small modification to [Main_MiSTer](https://github.com/MiSTer-devel/Main_MiSTer) (see [here](https://github.com/MiSTer-devel/Main_MiSTer/pull/763)).

Most of the shell scripts and services have been removed, a single service
remains that just runs the C++ code once.

All sockets comms, display composition and display output are done natively
in the C++ code.

When I printed the display cover I added three small buttons to use for a menu
system. The C++ code also does all of the Pi Zero GPIO handling for those buttons.

Where marquee images exist they will be used, if not, the game database will be
interrogated and the game information will be displayed in text instead.

See a short video of this working [here](https://1drv.ms/v/s!AgZXmGPNWbCKj_Zzbv6p7Q2keYUF-w?e=TY20IA)