# Fraken's TTSM V1.0A  

Intro: Small.  It's just small.  39x39 pixels, but just big enough you can easily and instantly see CPU, Disk, Network and Swap stats.  Sits on top, small enough to fit nine copies on a floppy disk from 1979 (for real) and fast enough that even updating 30x a second, it uses 0.02% CPU on my particular workstation.  I run it all day, every day!  --Fraken

DESCRIPTION: AN ICON SIZED, REAL TIME FREEBSD XORG SYSTEM MONITOR

TTSM: "Teeny Tiny System Monitor"

TTSM is a tiny, 39x39px real time system monitor for FreeBSD.  It is designed to be small enough to not get in the way and to be JUST large enough to see easily, be easy to move (just click and drag it).

It includes CPU use, Disk throughput, Network interfaces throughput and Swap statistics.  TTSM requires no configuration for devices.  It will automatically detect/iterate through your devices and automatically configure to read their stats.  In the case of Disk and Swap, the values you see are aggregates of every device in the system, both input and output.  As an example, if you have three disks in the system, TTSM will show you data flowing through any of those disks.  You don't need to configure it, it does it automatically.

TTSM is small, compiling to around 39 kilobytes in FreeBSD 13.2 and fast at only 0.02% CPU use.

REQUREMENTS AND DEPENDENCIES:

Requires the basic build system that comes with FreeBSD.  If you can buildworld, you can build TTSM.  TTSM is a C program that uses the XLib library and *NO* other dependencies.

BUILDING:

Type: make

RUNNING:

Run the binary like any other.  TL;DR:  Type ./ttsm

NOTE:  

If you want to move the binary somewhere, it expects its background art file (background.bmp) to be in the same directory.

KNOWN BUGS:

The 1.0A codebase has a bug in the network scaling code when using the scale reduction over time feature.  Every so often, it will extend the bar past the bottom of the graph.  It doesn't crash, but it does look crappy.  Inserting "if (height<0) {height=0;}" just before the XSetForeground in the drawNETWORK function fixes it.  This will be fixed in later versions.

Fraken was here: EOF FROM CLIENT/BROKEN PIPE JOKE.
