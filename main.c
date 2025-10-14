/*****************************************************

 	▄▄▄▄▄▄ ▄▄▄▄▄    ▄▄   ▄    ▄ ▄▄▄▄▄▄ ▄▄   ▄
 	█      █   ▀█   ██   █  ▄▀  █      █▀▄  █
 	█▄▄▄▄▄ █▄▄▄▄▀  █  █  █▄█    █▄▄▄▄▄ █ █▄ █
 	█      █   ▀▄  █▄▄█  █  █▄  █      █  █ █
 	█      █    ▀ █    █ █   ▀▄ █▄▄▄▄▄ █   ██ 's


			▄▄▄▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄  ▄    ▄
			   █      █    █▀   ▀ ██  ██
			   █      █    ▀█▄▄▄  █ ██ █
			   █      █        ▀█ █ ▀▀ █
			   █      █    ▀▄▄▄█▀ █    █

           FRAKEN'S TTSM
	A TEENY-TINY-SYSTEM-MONITOR for FreeBSD.
         VERSION 1.0 ALPHA.

While I consider myself a terrible programmer and I haven't written C code since
I retired, I have done my best to design/write this project in a way that doesn't
reflect this.  This is written at a low level for FreeBSD using Xlib.  As a 
result, the program itself is fairly tiny and kind of fast.

THINGS YOU SHOULD KNOW... -Fraken

#0: No warranties explicit or implied or whatever the heck that language is.  For real.
#1: I used to be a coder.  I mean, I'm pretending I'm one now too, but I used to be one too...
#2: I am bad at this now.  You're going to find typing errors, inclusions that do nothing
     and all kinds of funky stuff.  BUT, I *am* putting effort towards making it 
	  reliable, as bug free as I can get it, do as many "expected" things as I can
	  like being able to drag it around, keep it on top, pass arguments to change
	  a few things, use keys like q and x to quit and so on.  I should also mention that 
	  most of the work I have done has been either at very low levels (ASM for small micro's)
	  or higher level (database, UI design, web, A.I., *some* early distributed computing stuff)
	  so while C isn't new to me, a highly threaded xlib Xorg program certainly is.
#3: It's entirely likely you will be able to pass things into variables that can
     cause weird behavior.  If you need help, scroll down and read.
#4: I didn't want to stray much further into "real" code than I needed too, so there's no locking
	  like subatomic thermonuclear mutexing or anything fancy.  A declaration or two and a few lines
     of code are too much for this 'ol feller.  If there's some weird temporary issue
	  due to using globals for data passing between threads, whoopity doo.  It's eye candy.  Having it
     lose its mind for a single frame out of 30 frames/second is no big deal.
	  I actually paid a bit of attention to this and while I've only done a tiny handful of this
	  kind of lower level work even in my career, it *seems* like the worst outcome is stale or 
     corrupted data for one frame.  This is entirely tolerable.  I may fix this later, I may not.
#5: In case it wasn't clear, the above was stdatomic and pthread_mutex_t jokes.
#6: I poked my head a bit deeper into the FBSD source while doing this project.  I have no business being here.
#7: There are no facilities for saving or storing configuration data so if you want to start TTSM with some
      kind of default values like config or location, you'll need to make a shell script or something.

***************************************************/
	//system library header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
	//my own library header files
#include "cpu.h"
#include "network.h"
#include "disk.h"
#include "swap.h"
#include "graphics.h"

	// This allows them to be accessed from any of the source files.
	//  This is how you know I'm a terrible programmer. :)
   // We should be using atomic/mutex stuff here, but 
   // what's the worst that could happen if we don't? *core dumped*. :P -Fraken.
extern float cpucurrent, cpulast, networkcurrent, networklast, 
             diskcurrent, disklast, swapcurrent, swaplast;

	// These will be the "peak" values we've ever seen.
extern float cpumax, networkmax, diskmax, swapmax;

   // These are to smooth the 3:1 values for framerate vs pollingrate.
extern float cpufast, diskfast, networkfast;

   //for fiddling around if you need to do so..
   // this isn't used, it's just a value you can use for testing for convenience.
extern float test;

extern int exitflag;            //Set to 0.  Set to 1 to exit anywhere in the program.
extern int framerate;           //Original is 30/second. We calculate rough usleep values from this
extern int fastsmoothing;       //To account for a 3:1 framerate/polling.
extern int smoothingvalue;      //Original is 5.   The weighting value for our moving average.  
extern int networknormalize;	  //For gradual reducing of the network window scale.
extern int disknormalize;		  //For gradually reducing the disk window scale.
extern char backgroundfilename; //Filename of our background artwork.

void printhelp(){

	//Ugh, I should have written this AFTER I was done, not before I started.  Live and learn... -Fraken.

   printf("\n");
   printf("\n This is the binary for 'Fraken's TTSM': a tiny X system monitor for FreeBSD.");
   printf("\n ----------------------------------------------------------------------------");
   printf("\n");
   printf("\n To start the program with default values, just type/run/execute: ttsm");
   printf("\n Invoking without arguments will run the program with the following defaults:");
   printf("\n");
   printf("\n  Framerate=%i, Smoothing=%i, NetNormalize=%i, DiskNormalize=%i\n", 
					framerate, smoothingvalue, networknormalize, disknormalize);
   printf("\n");
	printf("\n   NOTE: You don't need to 'set' the scale for disk or network througputs or what devices");
	printf("\n         to monitor.  They're iterated to every available device, in and out stats and the");
	printf("\n         'scale' is automatically set by the peak value detected.");
	printf("\n");
   printf("\n Framerate is how fast the GUI element are updated.  The polling rate for data collection");
   printf("\n  of system metrics are derived from there.  Most are 1/3 the GUI framerate, but some");
   printf("\n  that change slowly like swap are 150x longer (5 sec intervals) than refresh.");
   printf("\n");
   printf("\n If you wish to change the GUI update speed or the rate at which data is averaged, you can");
   printf("\n  use the following flags: -f and -s.  The -f flag sets the GUI frame/second update rate.");
   printf("\n  The -s sets the smoothing rate for data collection.  The smoothing algorithm is just");
   printf("\n  a weighted moving average.  Example: The calculation uses the new and old sample values");
   printf("\n  and makes an average of multiple copies of the old sample with a single value of the new");
   printf("\n  one.  This smooths out the rapid change often seen in values like disk access and network");
   printf("\n  data samples into something easier to see.  Setting the smoothing value very low however");
   printf("\n  will let you see these rapid changes more easily.");
	printf("\n");
   printf("\n The network 'scale' will normalize itself.  Because network connections will tend to send");
	printf("\n   short/large bursts of full speed, it's easy to autodetect and scale for.  But, if you have");
	printf("\n   a lot of small data you want to see, you can set the NetNormalize flag.  This is a 'raw'");
	printf("\n   value that gradually reduces the 'networkmax' value each frame and these are big values so");
	printf("\n   expect to use values from ten to thousands depending on what you want.  Example: ttsm -n 200");
	printf("\n   You can do the same for disk.  Example: ttsm -d 1000");
	printf("\n");
	printf("\n             Invocation with all flags: ttsm -f 120 -s 500 -n 100 -d 100");
	printf("\n	 Runs at 120 frames/second, bar graph averaging over 500 frames, reducing netwindow by 100/update");
	printf("\n   and reducing disk by 0.1 megabit/update. You may need to fiddle with -n and -d");
	printf("\n");
	printf("\n If you have any issues like a blank background, the background artwork might be missing.");
	printf("\n  TTSM relies on a 39x39px 24bpp bitmap as background artwork called 'background.bmp'.");
	printf("\n  If it's missing, TTSM will sh1t the bed, so make sure it's in the same directory as");
	printf("\n  the system binary itself.  Note that flashing while scrolling is normal.  If you hate");
	printf("\n  this, see if your window manager has an 'always on top' option.");
	printf("\n");
   printf("\n       <BROKEN PIPE><EOF FROM CLIENT> (Ah, IRC... AMIRITE!? -Fraken) :P\n");
   return;
}

    /* PRINTS OUT BASIC USAGE INFORMATION ON STARTUP*/
void printoptions(){
    printf("\nFor a short description of options for Fraken's ttsm, use the -h flag");
    printf("\nExample: ttsm -h");
    if (framerate>240){
        framerate=240;
        printf("\nSaving you from yourself.  Clamping framerate to %i", framerate);
    }
    if (smoothingvalue>10000){
        smoothingvalue=10000;
        printf("\nSaving you from yourself.  Clamping smoothing value to %i", smoothingvalue);
    }
    printf("\n\nProceeding with Framerate=%i,Smoothing=%i,NetNormalize=%i,DiskNormalize=%i\n\n", 
	 			framerate, smoothingvalue, networknormalize, disknormalize);
    return;
}

/*************************************************************

▄▄▄▄▄  ▄▄   ▄▄▄▄▄▄▄▄   ▄    ▄   ▄▄   ▄▄▄▄▄  ▄▄   ▄      █   █   
  █    █▀▄  █   █      ██  ██   ██     █    █▀▄  █     █     █  
  █    █ █▄ █   █      █ ██ █  █  █    █    █ █▄ █    █       █  
  █    █  █ █   █      █ ▀▀ █  █▄▄█    █    █  █ █     █     █  
▄▄█▄▄  █   ██   █      █    █ █    █ ▄▄█▄▄  █   ██      █   █   

**************************************************************/

int main(int argc, char *argv[]) {

	/* DEAL WITH COMMAND LINE ARGUMENTS */
	for (int i = 1; i < argc; i++) {
   	if (strlen(argv[i]) == 2 && argv[i][0] == '-') {
      	/*  I WON'T BOTHER DOCUMENTING THIS SINCE IT'S BOG STANDARD C STUFF */
      	/*  SOME HINTS THOUGH: ARGC IS THE ARGUMENT CHARACTERS AND THE */
      	char option = argv[i][1];
      	if (i + 1 >= argc) {
         	// use '' instead of "" to fix the char pointer issue
         	if (option == 'h'){
            	printhelp();
            	return 1;
         	}
            printf("Option -%c requires a value\n", option);
            printf("Use the -h flag for help.  Example: ttsm -h \n");
            return 1;
      	}
      	char *value = argv[i + 1];
      	int intValue = atoi(value);
      	switch (option) {
         	case 'f':
            	framerate = intValue;
               break;
            case 's':
               smoothingvalue = intValue;
               break;  
            case 'n':
               networknormalize = intValue;
               break; 
            case 'd':
               disknormalize = intValue;
               break; 
      	}
      	i++;
   	} 
	}
	printoptions();

		/*  START ALL THE WORKER THREADS AND FUNCTIONS   */
		/*      NICK WOULD BE PROUD!   -FRAKEN           */
		/*   THREADING FUNCTION DEFINITION/STRUCTURE.    */

	pthread_t cpu_thread, network_thread, disk_thread, swap_thread, graphics_thread;

   //  pthread_create(thread, attr, routine, arg);
	if (pthread_create(&cpu_thread, NULL, cpu, NULL) != 0) {
   perror("Failed to create CPU thread");
   exit(EXIT_FAILURE);
   }
   if (pthread_create(&network_thread, NULL, network, NULL) != 0) {
   perror("Failed to create Network thread");
   exit(EXIT_FAILURE);
   }
   if (pthread_create(&disk_thread, NULL, disk, NULL) != 0) {
   perror("Failed to create Disk thread");
   exit(EXIT_FAILURE);
   }
   if (pthread_create(&swap_thread, NULL, swap, NULL) != 0) {
   perror("Failed to create Swap thread");
   exit(EXIT_FAILURE);
   }
   if (pthread_create(&graphics_thread, NULL, graphics, NULL) != 0) {
   perror("Failed to create Graphics thread");
   exit(EXIT_FAILURE);
   }

   	/* THIS LOOPS UNTIL SOMETHING TELLS US TO EXIT*/
      /*  EVERY FUNCTION/THREAD HAS THIS MAIN       */
      /*  WHILE LOOPS AND WILL EXIT CLEANLY ONCE    */
      /*  IT'S SET TO EXIT/QUIT                     */

    // calculate sleep value as 1 second divided by framerate.
    // 1,000,000uS divided by 30 frames/second (default) to get our base 30 frames/second
    // then multiply by 3 to get 10 loops per second. 
   int localsleepvalue = 0;
   localsleepvalue = (1000000/framerate)*3;

 /***********************************************
░░░░▄▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
░░░▀▄░░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
░░░░░▀░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
 ***********************************************/
   while (exitflag==0){
   	usleep(localsleepvalue);
   }
 /***********************************************************************
░░░░▄▀░░░█▀▀░█▀█░█▀▄░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
░░░▀▄░░░░█▀▀░█░█░█░█░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
░░░░░▀░░░▀▀▀░▀░▀░▀▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
 ***********************************************************************/ 

   //  Clean up nicely I guess.  -Fraken
   pthread_join(cpu_thread, NULL);
   pthread_join(network_thread, NULL);
   pthread_join(disk_thread, NULL);
   pthread_join(swap_thread, NULL);
   pthread_join(graphics_thread, NULL);

   printf("All TTSM threads have completed successfully. -Bye! -Fraken\n");

   return EXIT_SUCCESS;
}