FRAKEN'S TTSM VERSION 1.0 Alpha 
-------------------------------

A teeny tiny Xorg FreeBSD system monitor application.

QUICK START:

WILL NOT RUN ON LINUX OR WINDOWS!  FREEBSD ONLY!
Type "make" to build. Type "./ttsm" to start the program.
Type "./ttsm -h" for help.
To exit, hit x, e or esc.  To move, click and drag.
There is no configuration for devices, etc.
Monitors CPU, Disk, Network and Swap.
Should run on any modern version.

WARNING: 

This code is FreeBSD/Xorg specific.  IT WILL NOT RUN ON LINUX OR WINDOWS.
It MAY run on Wayland with the Xorg compat stuff, but it's untested.
I wrote this for myself originally, but I'm sharing it with you. -Fraken.

DESCRIPTION:

A tiny Xorg 'large icon' sized FreeBSD monitor for CPU/Disk/Network/Swap.

TTSM is a tiny, nearly icon sized (39x39 pixels) system monitor for
FreeBSD that just sits on top of your desktop.  It's meant to monitor and just
give you a general idea of what's going on.  There's no configuration, there's
only two files: The ttsm binary and the background art file background.bmp

When you run the program, the first thing it does is iterate and sort out every
network interface on the system and every accessible disk.  Then, each "tick" of
the worker threads, it sorts out CPU stats, sums up input AND output for each
network interface and each disk as a total sum, then graphically displays this.

There are two kinds of information provided: Real time and a moving average.
The real time stats are denoted by a 1 pixel tall white bar, the moving average
stats are denoted by the bar graph itself.  The real time stats have no 
configuration, but if you use the -h flag, you can get help on how to configure 
the moving average if you want to change these values.  Note that hard coded 
values for sanity are in there, but if you really wanted too, it's trivial to 
change those in the code.

I wrote and tested it on 13.2, but it should work pretty much anywhere.
It's written in C/Xlib and written to be as fast as I could muster.  It
doesn't have any dependencies outside of the background art (background.bmp)
and the basic C libraries that come with FreeBSD.  If you run FreeBSD, it
should compile and run just fine.

BUILDING AND RUNNING:

Building: type "make" in the main directory with the source and makefile.
Running: It's just an Xorg program binary. At a console, type ./ttsm or ./ttsm &
If you want, you can copy the binary anywhere so long as the background.bmp
file is in the same directory.  I know FreeBSD doesn't like this, but
too bad.  I'm not changing it yet (or ever? I dunno...).  It doesn't actually
"install" itself yet so any autoprogram start menu stuff you have for your
window manager, it won't be added.  You can add it yourself if you want.

I included two background art files.  One has bars that start green and work up
to red, the other is just plain red bars.  If you want the pure red version, you
can just rename the original to background.bmp.old and rename the other one to
background.bmp.

Personally, I just start it by opening a console, going into the build directory
and typing "./ttsm &".  It starts the program, drops it into the background
and you can just close the console.  You could also start it from a graphical
file manager by "running" it in whatever way your manager works.  -Fraken

BASIC USE:

While there are flags you can pass in (./ttsm -h), you don't need to configure TTSM.
It's written to autoconfigure everything on its own.  In the case of disk and
network, it will automatically iterate and sum up the input and output of every 
network interface and every disk in the system (other than swap).  There are two
stats shown for every item: Realtime use is a white line, aggregated or average use
over time is a moving average stat that's show by the colored bar.  If you don't
like the look of the background artwork, I include two options or you can just
make one yourself so long as it's a 24 bit file *exactly* the same size as the one
that's included.  If you make one and things are upside down, sideways, etc, then
it wasn't saved as the same kind of file formatting.

If you want to exit, just put your mouse over the app or click on it and hit one of
the usual "exit" buttons like Esc, X or E.  Or, just kill the binary.  Your choice.
If you want to move the window around, click and drag it.  It's straight forward.

The app is written so that it tells the window manager NOT to decorate the window aka, 
put a header and buttons on top of it, but this is only a request so if you see a window 
header and you don't like it, check your window manager to see if there's a way to 
get rid of it.

ADVANCED: FLAGS

By default, TTSM updates the main UI at 30 frames/second for smooth viewing and polls
every stat at 10 ticks/second other than swap (once every 5 seconds).  These are based
on the framerate so doubling framerate will double how fast the polling is.  Since large
peaks of network and disk traffic will often peak at 100% of the link, I use a sort of
"100% scale window == fastest burst we've seen so far" approach to scaling.  Sometimes 
though, you might want to see small bursts and blips.  For this, I included a flag 
that allows you to gradually (or rapidly depending on value) reduce this value.  This
is VERY helpful if you just want to be able to catch programs sending blips of data
or writing to disk.  The flag for reducing the network window is -n value and the
flag for reducing the disk window is -d value.

-f  Framerate of main UI: Sane values: 1 to ~300
-s  Smoothing: Bar graph moving average value: Sane Values: 1 to 10000
-n  Network gradual scale reduction: Raw values, 10 to 10000 (play with values, there's no sanity check)
-d  Disk gradual scale reduction:value is 1/1000th of a megabit so 100 is .1 megabit/tick
Disk and network scale reduction is a per tick value so if you're reducing the window by a
given value, it will happen at the default framerate value or the -f <value> you specify.

Framerate=30, Smoothing=60, NetNormalize=0, DiskNormalize=0

Invocation with all flags: ./ttsm -f 120 -s 500 -n 100 -d 100

HOW TTSM CONFIGURES ITSELF:

Internally, TTSM does all configuration for devices automatically.  It sets up every CPU,
every disk and network device and pulls swap from the system's tools (pstat and a few others). 
It automatically makes a sum for every CPU, every disk and NIC.  The "scale" is set automatically
as well, though as stated above, you can change this behavior.  TTSM configures the scale by a
very simple logic:  Every update of the display, we check the size of the current value like so:

If current_value is greater than max_value then max_value is equal to current_value

Since almost all systems create very short bursts of 100% saturation fairly often, this approach
eliminates the entire notion of having to configure for scale and/or value.

Also, remember that for disk and network, the stats are a sum of both input and output.

LICENSE:

Provided: "As is".  I (The author) accept no responsibility of any kind for use.  
This includes, but is not limited to, security and/or stability.  You are free to 
use, share, extend and modify the code.  If you publish it elsewhere, a mention
of the original author would be nice though.

===========================================================================================

PERSONAL NOTES FROM THE AUTHOR

Why I wrote this:  One of the little fun things I like to run on my desktop (E16) is 
instances of windowmaker dock programs, but just as non docked tools.  One of my favorites
since the 2000's has been "wmcube".  It's a tiny 3D cube that spins and the speed tells
you how much CPU you're using'.  It eventually donned on me that I often want more information
than this provides and it also donned on me that I often find myself moving wmcube around
so it doesn't get in the way.  Thus, I though "I should make my own".  So I did.  The very
first thing I did, was to rough out what it would look like and what it would contain.
I used photoshop and gimp and just made various examples.  I gradually shrunk them in 
size until it was illegible, then "backed off 1/4 turn" as they say.  The result is a
tool just big enough to see easily, small enough you can just park it somewhere and leave
it and never touch it again.

When I wrote this code, I also commented liberally in the code and structured it such
that it's easier for anyone who wants to learn C or understand how the code works.
Some parts of it are fairly esoteric and not exactly trivial, but you should be able
to at least follow along.  I should also add that this WAS NOT a "vibe coding"
project.  While I used A.I. liberally as a sort of "smart search/reference"
tool, only a few bits were directly made by A.I. such as the general boilerplate
that created the blank files with names the same as functions.  Beyond that, most
of the code was *mostly* stuff I wrote, but there are a handful of places where I
used A.I. to write snippets, then modify/extend them.

I should also note that I have been retired due to health and memory/cognitive issues 
for around 23 years now and haven't written more than a hello world in C in about 
that length of time.  As such, the code itself will look like it was written as
a first attempt by a child.  I mean, I used plain old global variables for passing 
data between threads for frig sakes! Lol!  There's two reasons why I chose this 
approach though: First, and most important, while I have experience writing C, 
it's not a *lot* and I haven't done it in 23 years which means it was a virtual
biblical type miracle that I even got this done.  Second, it's a bloody icon sized curiosity
to just keep an eye on things.  If there's some race condition/contention that blows up
a value for a sample because I didn't use atomics, whooopity do.  You have 29 more of 
them to go in the next 1 second.  Having said that, in the months that I've been running
TTSM personally, it's never crashed and never had an issue.  YMMV.

WHY IS THERE GIANT COMMENTS IN THE CODE BASE THAT LOOK ALL BROKEN?!?!

Because I'm old and I bad now and I needed very very large comments that I could see in 
the little preview window on the right side of VSCode.  Just delete them if you don't
like them.  Or, make sure that your font is small enough they don't wrap.  Wrapping
is why they'd look broken.

Overall structure:

(Partly copied from my original specifications document I wrote -Fraken)

main.c is the main thread and creates and dispatches the others to
do what they each need to do.  A handful of globals are used to allow
anything from anywhere to talk to them.  This is shitty design, but it's
simple, trivial and easy to wrap your head around.  I'm bad and I feel bad.

The biggest reason I decided to take this approach is PERFORMANCE:  If I have
a bunch of functions, every time they fire, they will be building up
the interfaces they need, reading the data we want, tearing down all
of that interface stuff and only THEN giving it back to us.  Imagine
the overhead of doing this 10 times a second.  Now imagine doing it 30
times a second!  By building that interface once and using it until we
are done, we avoid that overhead entirely.

The various parts/threads will update at various rates based on a local
value that's calculated from a global value.  Each blah.c file has
these values hardcoded if you want to change them.

Updates the main interface at 30 x per second
Updates the data for CPU/disk/network 10 x per second.
Updates the data for swap at once per 10 seconds

We're using the usleep value which 1 sec = 1,000,000 which means all
math for timers is based upon this math other than swap which is sleep.

30x/second = 33,333 usecond sleep for main loop.
10x/second = 100,000 useconds sleep for each loop.

I changed usleep to sleep in swap.c because apparently usleep was never
intended to be used for more than 1 second and swap.c only updates once
every 10 seconds at 30fps, every 20 seconds at 15fps and so on.