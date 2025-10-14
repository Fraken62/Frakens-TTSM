
/*
    This frickin file should have been called "main.c".  Jeepers... -Fraken

    EVERYTHING INTERESTING THAT HAPPENS IN THIS PROGRAM WILL PROBABLY STEM FROM HERE...

    DESCRIPTION/DOCUMENTATION
    The "graphics()" function will effectively be the main function in this program.  This is because
    virtually everything that happens at a fast rate will be done here.  This is the function that
    creates the main window.  The other threads are all responsible for their own work.  They each
    enumerate/aggregate/average the data themselves.  Each thread will have the following defaults:

    graphics:   30 frames/second
    network:    10 frames/second
    disk:       10 frames/second
    cpu:        10 frames/second
    swap:       1 frame per 5 seconds.

    These values are set in each file if you want to change them

    Swap may seem like an outlier, but there's a reason I decided this default.  Swap seemed like it was 
    going to be fairly trivial when I first started digging into the pstat.c code, but it turns out it's 
    far less so with my poorly set up programming environment, so I decided that since pstat itself is an
    exceptionally light binary, I'm simply going to pop the binary once every five seconds, parse the output
    and be done with it for now.
*/

   /* STANDARD C HEADER FILES */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

   /* HEADER FILES FOR OUR OWN PROGRAM */
#include "graphics.h"
#include "globals.h"
#include "cpu.h"
#include "network.h"
#include "disk.h"
#include "swap.h"
#include "graphics.h"

   /* X11 HEADER FILES */
   /*  Remember that these need the Makefile cflags of 
    -ldevstat -I/usr/local/include -L/usr/local/lib -lX11 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

	/*   For grabbing keyboard keys.	*/
#include <X11/keysym.h>


/****************************************************************************

 ▄▄▄▄   ▄▄▄▄▄▄ ▄▄▄▄▄▄ ▄▄▄▄▄  ▄▄   ▄ ▄▄▄▄▄ ▄▄▄▄▄▄▄ ▄▄▄▄▄   ▄▄▄▄  ▄▄   ▄  ▄▄▄▄ 
 █   ▀▄ █      █        █    █▀▄  █   █      █      █    ▄▀  ▀▄ █▀▄  █ █▀   ▀
 █    █ █▄▄▄▄▄ █▄▄▄▄▄   █    █ █▄ █   █      █      █    █    █ █ █▄ █ ▀█▄▄▄ 
 █    █ █      █        █    █  █ █   █      █      █    █    █ █  █ █     ▀█
 █▄▄▄▀  █▄▄▄▄▄ █      ▄▄█▄▄  █   ██ ▄▄█▄▄    █    ▄▄█▄▄   █▄▄█  █   ██ ▀▄▄▄█▀

***************************  AND SUCH... -FRAKEN   *************************/

/* Type definition for uncompressed 24bpp bitmapped data */
typedef struct {
   int width, height;
   uint8_t *pixels; // RGBRGB...
} BMPImage;

typedef struct {
   unsigned long flags;
   unsigned long functions;
   unsigned long decorations;
   long input_mode;
   unsigned long status;
} MotifWmHints;

#define MWM_HINTS_DECORATIONS (1L << 1)
XEvent event;

	// Size of the app/window.
int width  = 39;
int height = 39;

/**************************************************************

 ▄▄▄▄▄▄ ▄    ▄ ▄▄   ▄   ▄▄▄ ▄▄▄▄▄▄▄ ▄▄▄▄▄   ▄▄▄▄  ▄▄   ▄  ▄▄▄▄ 
 █      █    █ █▀▄  █ ▄▀   ▀   █      █    ▄▀  ▀▄ █▀▄  █ █▀   ▀
 █▄▄▄▄▄ █    █ █ █▄ █ █        █      █    █    █ █ █▄ █ ▀█▄▄▄ 
 █      █    █ █  █ █ █        █      █    █    █ █  █ █     ▀█
 █      ▀▄▄▄▄▀ █   ██  ▀▄▄▄▀   █    ▄▄█▄▄   █▄▄█  █   ██ ▀▄▄▄█▀

***************************************************************/

/*
	ARTWORK DESCRIPTION

	All of these "drawBLAH" functions are for drawing their little tidbits on the bargraph in question.
		You'll notice a near exact duplication, but with the X values offset to draw the bar in the 
	correct place.  -Fraken.  PS: It may seem confusing when first looking at this code and the artwork.
	Instead of graphing by starting at the bottom and then making the bar grow upwards, we fake this so that
	the artwork is easier.  We do an "inverse" kind of black bar that starts at the top and grows down, but
	grows inversely with the percentage.  At a percent of 100, there's 100-100% growth or zero growth.  At
	a percentage of 50%, there's 100-50%, or 50% size and so we "grow" the bar downwards 50% of the total size.
	At 0%, we have 100%-0 which is a bar at 100% size.  If full size is 20px tall, then 100% is a black bar that
	covers the entire area from top to bottom for 20px tall.  At 50%, it's 10px tall, starting at the top and 
	"growing" downwards.  The reason for this is that we can then later make the actual "bars" themselves ANY
	color simply by updating the artwork.  Want a color gradient bar?  Make a color gradient bitmap.  How about
	a bar that is a tall inverted V?  Update the artwork.  By using a black bar to just cover the areas we don't
	want to see, it makes it much more flexible.
*/

void drawSWAP(Display* display, Drawable frontbuffer, GC gc){

	float scaledpercentage;
	int x, y, width, height = 0;
	x		=30;
	y		=3;
	width =6;

		/*****	CALCULATE AND DISPLAY THE MOVING AVERAGE BAR GRAPH 	*****/
	swaplast = swapcurrent; //NO SMOOTHING!  This doesn't bounce around, thus...
	scaledpercentage = swaplast/100; //Scale value to 0 to 1
	height=scaledpercentage*20;
	height=(int)height;
	if (height>20){ height=20; }
	height=20-height;//Remember, 0 is full bar.,the plus one is a buffer.
		/* Blast out the graphics */
	XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);

		/*****	CALCULATE AND DISPLAY THE FAST/CURRENT BAR 	*****/
	//In this case, the y value of the graph will change
	scaledpercentage = swapcurrent/100;  //scale to 0 to 1
	y=scaledpercentage*20;					//Convert range to 0 to 20
	y=(int)y;									//truncate y to only the int for x/y use
	y=23-y;										//The smaller the number, the higher on the graph,20(+a fudge factor)-20=full scale
	height=1;									//Height of the bar graph is 1 pixel.
	y=y-1; //Offset it by 1 vertically so you can see the red under it for non moving things.
		/* Blast out the graphics again */
	XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);
}

//network
void drawNETWORK(Display* display, Drawable frontbuffer, GC gc){

	double currentpercentage = 0;
	//double averagedpercentage = 0;
	double scaledpercentage = 0; // This is temporary.
	int x, y, width, height = 0;
	x		=21;
	y		=3;
	width =6;

	if (networkmax<networkcurrent){ networkmax=networkcurrent; }

	//This is the line to gradually reduce the network peak.
   //Give ME a compile time warning!  I'll cast you, ya sucker! -Fraken.
	if (networknormalize > 0 && networknormalize<(int)networkmax) { 
		networkmax = networkmax - networknormalize;
	}

	if (networkmax != 0){
		currentpercentage = ((double)networkcurrent / (double)networkmax) * 100;
	}

	//Let's just see if this does the "gradual keeping things in bounds" thing...  <-- DO NOT DELETE...
	//lastpercentage = lastpercentage + 10; I AM KEEPING THIS AS A "PIN OF SHAME".  Jesus I'm retarded.

		/*****	CALCULATE AND DISPLAY THE MOVING AVERAGE BAR GRAPH 	*****/
	networklastpercentage = ((networklastpercentage * smoothingvalue) + currentpercentage) / (smoothingvalue + 1);
	scaledpercentage = networklastpercentage/100; //Scale value to 0 to 1
	height=scaledpercentage*20; // Scale it to the number of pixels our bar is tall.
	height=(int)height; //Now invert it because a full bar is zero since we're covering the artwork.
	height=21-height;//Remember, 0 is full bar.,the plus one is a buffer.
		/* Blast out the graphics */
	XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);

		/*****	CALCULATE AND DISPLAY THE FAST/CURRENT BAR 	*****/
	//In this case, the y value of the graph will change
	scaledpercentage = currentpercentage/100;  //scale to 0 to 1
	y=scaledpercentage*20;					//Convert range to 0 to 20
	y=(int)y;									//truncate y to only the int for x/y use
	y=23-y;										//The smaller the number, the higher on the graph, so 20-20=full scale
	height=1;									//Height of the bar graph is 1 pixel.
		/* Blast out the graphics again */
	XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);
}

void drawDISK(Display* display, Drawable frontbuffer, GC gc){

		//these are just double for shitzngigz. (kind of)
	float currentpercentage = 0;
	float averagedpercentage = 0;
	float scaledpercentage = 0;
	int x, y, width, height = 0;
	x		=12;
	y		=3;
	width =6;

	diskmax = diskmax - (disknormalize/1000);// Gradual window size reduction.

	if (diskmax < diskcurrent){ diskmax = diskcurrent; } //Tested and okay.
	if (diskcurrent != 0){ currentpercentage = (diskcurrent / diskmax) * 100; } //Tested/fine -Fraken
	
		/*****	CALCULATE AND DISPLAY THE MOVING AVERAGE BAR GRAPH 	*****/
	diskaverage = ((diskaverage * smoothingvalue) + currentpercentage) / (smoothingvalue + 1); //seems to work.
	averagedpercentage = diskaverage/100; //Scale value to 0 to 1  And works.
	scaledpercentage = currentpercentage/100; //scale it to 0 and 1. And works.

	height=averagedpercentage*20;
	height=(int)height;
	height=21-height;//Remember, 0 is full bar.,the plus one is a buffer.
		/* Blast out the graphics */
	XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);

		/*****	CALCULATE AND DISPLAY THE FAST/CURRENT BAR 	*****/
	//In this case, the y value of the graph will change
	y=scaledpercentage*20;					//Convert range to 0 to 20
	y=(int)y;									//truncate y to only the int for x/y use
	y=23-y;										//The smaller the number, the higher on the graph, so 20-20=full scale
	height=1;									//Height of the bar graph is 1 pixel.
		/* Blast out the graphics again */
	XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);
}

void drawCPU(Display* display, Drawable frontbuffer, GC gc){

	float scaledpercentage;
	int x, y, width, height = 0;
	x		=3;
	y		=3;
	width =6;

      //AHHHH! Leave this?  Make it "normal" and just clamp it to 100?!?!?! -Fraken
	if (cpumax<cpucurrent){ cpumax=cpucurrent; }

   //printf ("\ncpucurrent value=%f", cpucurrent);

		/*****	CALCULATE AND DISPLAY THE MOVING AVERAGE BAR GRAPH 	*****/
	cpulast = ((cpulast * smoothingvalue) + cpucurrent) / (smoothingvalue + 1);
	scaledpercentage = cpulast/100; //Scale value to 0 to 1
	height=scaledpercentage*20;
	height=(int)height;
	height=21-height;//Remember, 0 is full bar.,the plus one is a buffer.
		/* Blast out the graphics */
	XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);

   //float cpufast=0.0f, int fastsmoothing(3); float(local) scaledpercentage; 

		/*****	CALCULATE AND DISPLAY THE FAST/CURRENT BAR 	*****/
	//In this case, the y value of the graph will change
	scaledpercentage = cpucurrent/100;  //scale to 0 to 1
   cpufast = ((cpufast * fastsmoothing) + scaledpercentage) / (fastsmoothing + 1); //slight smoothing

   //start of test code
   //goes up twice as fast as down.
   if(test<cpufast){ test=test + 0.05; } //0.5 scales to 20 values for number of pixels.  One pixel per incr.
   if(test>cpufast){ test=test - 0.05; }
	y=test*20;					//Convert range to 0 to 20
	y=(int)y;									//truncate y to only the int for x/y use
   //end of test code.


   // end of new test code
	//y=cpufast*20;					//Convert range to 0 to 20
	//y=(int)y;									//truncate y to only the int for x/y use
	y=23-y;										//The smaller the number, the higher on the graph, so 20-20=full scale
	height=1;									//Height of the bar graph is 1 pixel.
		/* Blast out the graphics again */
	XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
	XFillRectangle(display, frontbuffer, gc, x, y, width, height);

}

   //  Call this to rip the border off the window (My own preference)
void set_window_borderless(Display *display, Window window) {
   Atom prop = XInternAtom(display, "_MOTIF_WM_HINTS", False);
   MotifWmHints hints;
   hints.flags = MWM_HINTS_DECORATIONS;
   hints.decorations = 0; // 0 means no decorations
   hints.functions = 0;
   hints.input_mode = 0;
   hints.status = 0;

   XChangeProperty(display, window, prop, prop, 32,
                   PropModeReplace, (unsigned char *)&hints, 5);
}

void show_bmp(Display *dpy, Window win, GC gc, BMPImage *img) {

   //This is all make from borrowed/stolen snippets. -Fraken.
   //Notice that it looks like we're coverting the values below
	//paint the image onto the window.
   int scr = DefaultScreen(dpy);
   Visual *vis = DefaultVisual(dpy, scr);
   int depth = DefaultDepth(dpy, scr);

   int w = img->width, h = img->height;
   char *data = malloc(w * h * 4);

   // Convert RGB to XImage format (ARGB32)
   // Note the flipping of RGB to BGR.  AKA, 0->2, 1->1, 2->0
   for (int y = 0; y < h; ++y){
      for (int x = 0; x < w; ++x) {
         int pi = (y * w + x) * 3;
         int di = (y * w + x) * 4;
         data[di + 0] = img->pixels[pi + 2]; // Blue
         data[di + 1] = img->pixels[pi + 1]; // Green
         data[di + 2] = img->pixels[pi + 0]; // Red
         data[di + 3] = 0;
      }
   }
   XImage *xi = XCreateImage(dpy, vis, depth, ZPixmap, 0, data, w, h, 32, 0);
   XPutImage(dpy, win, gc, xi, 0, 0, 0, 0, w, h);
   XDestroyImage(xi);
}

//Returns a pointer called BMPImage that is a BMPImage datatype containing 
// our data that is defined above.
BMPImage *load_bmp(const char *filename) {

      // Very little of this image loading code is mine -Fraken
      // I've done very little raw graphics work in C

   FILE *f = fopen(filename, "rb");
   if (!f) return NULL;

   uint8_t header[54];
   fread(header, 1, 54, f);

   int w = *(int *)&header[18];
   int h = *(int *)&header[22];
   int bits = header[28];

   // Only support 24bpp BMP
   if (bits != 24) {
   	// THIS IS FATAL.  JUST DIE. 
      fclose(f);
      exitflag = 1; //Can't continue, set the exit flag and die.  
		printf("\n\nTHE BACKGROUND ARTWORK HAS THE WRONG FORMAT, LIKELY THE WRONG BIT DEPTH.  SHOULD BE 24 BPP.\n\n");
      return NULL; 
   }

   int row_padded = (w * 3 + 3) & (~3);
   uint8_t *pixels = malloc(w * h * 3);
   uint8_t *row = malloc(row_padded);

   for (int y = h - 1; y >= 0; y--) {
   	fread(row, 1, row_padded, f);
      for (int x = 0; x < w; x++) {
         // BMP stores as BGR
         int pi = (y * w + x) * 3;
         pixels[pi + 2] = row[x * 3 + 0]; // Blue
         pixels[pi + 1] = row[x * 3 + 1]; // Green
         pixels[pi + 0] = row[x * 3 + 2]; // Red
      }
   }
   fclose(f);
   free(row);

   BMPImage *img = malloc(sizeof(BMPImage));
   img->width = w;
   img->height = h;
   img->pixels = pixels;
   return img;
}

/**************************************************************************************

 ▄▄   ▄  ▄▄▄▄ ▄▄▄▄▄▄▄   ▄▄▄▄▄  ▄▄   ▄▄▄▄▄▄▄▄   ▄    ▄   ▄▄   ▄▄▄▄▄  ▄▄   ▄      █   █   
 █▀▄  █ ▄▀  ▀▄   █        █    █▀▄  █   █      ██  ██   ██     █    █▀▄  █     █     █  
 █ █▄ █ █    █   █        █    █ █▄ █   █      █ ██ █  █  █    █    █ █▄ █    █       █  
 █  █ █ █    █   █        █    █  █ █   █      █ ▀▀ █  █▄▄█    █    █  █ █     █     █  
 █   ██  █▄▄█    █      ▄▄█▄▄  █   ██   █      █    █ █    █ ▄▄█▄▄  █   ██      █   █   

**************************    FOR REALSIES!  IT'S NOT!  ******************************/
//                          (BUT IT SURE ACTS LIKE IT!)

void* graphics() {

	printf("Graphics thread started.\n");

		/*
				░█▀▄░█▀▀░█▀▀░█░░░█▀█░█▀▄░█▀█░▀█▀░▀█▀░█▀█░█▀█░█▀▀
				░█░█░█▀▀░█░░░█░░░█▀█░█▀▄░█▀█░░█░░░█░░█░█░█░█░▀▀█
				░▀▀░░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀░▀░▀░▀░░▀░░▀▀▀░▀▀▀░▀░▀░▀▀▀
		***************************************************************/

   Display *display;                				//Connection to X server
   Window window;                   				//Identify specific window
   int screen;                      				//Screen number (X can have multiple)
   int localsleepvalue = (1000000/framerate); 	//Calculate the uSleep value for the message pump loop.

		/*  FOR MANUALLY MOVING THE WINDOW ON MOUSE DOWN ON THE WINDOW ITSELF  */
   int dragging = 0;
   int drag_start_x = 0, drag_start_y = 0;
   int win_start_x = 0, win_start_y = 0;

		/*
							░█▀▀░█▀▀░▀█▀░█░█░█▀█
							░▀▀█░█▀▀░░█░░█░█░█▀▀
							░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░░
		******************************************************/
      
   //This loads our background image into a struct in BMPImage.
   //Defined backgroundfilename in globals.c
   BMPImage *img = load_bmp(backgroundfilename);
   if (!img) {
   	fprintf(stderr, "Load BMP failed\n");
      exitflag = 1; //it's a fatal issue, just exit
   }

	display = XOpenDisplay(NULL);    //Connect to the X server.
   if (display == NULL) {
      exitflag = 1; // We've shit the bed, just turf it.
   	return NULL;
   }
   screen = DefaultScreen(display); //int value describing which screen to use (if multiple)    
   window = XCreateSimpleWindow(
      display, RootWindow(display, screen),
      100, 100,       // x, y position on screen
      width, height,         // width, height in pixels
      1,              // border width
      BlackPixel(display, screen),
      WhitePixel(display, screen)
   );

      // Set the name on the window titlebar and internally (sort of?) -Fraken
   XStoreName(display, window, "TTSM 0.9");

		// Ignore window close (the X button) and make it headerless.
	Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(display, window, &wmDeleteMessage, 1);
	set_window_borderless(display, window);//Borderless

	//set_window_borderless(display, window);//Borderless
   // Keep our window the right size
   XSizeHints *size_hints = XAllocSizeHints();
   size_hints->flags = PMinSize | PMaxSize | PPosition;
   size_hints->min_width = size_hints->max_width = width;
   size_hints->min_height = size_hints->max_height = height;
   XSetWMNormalHints(display, window, size_hints);
   XFree(size_hints);

		//XSelectInput is what determines what "events" we want to snag
		// in our message pump.  No need for ExposureMask | since we're
		// Updating it 30x/second anyway.
	XSelectInput(display, window, KeyPressMask | 
									 PointerMotionMask |
								  StructureNotifyMask |
										ButtonPressMask |
									 ButtonReleaseMask );

	XMapWindow(display, window);	//make the window visible
	//XMoveWindow(display, window, 100, 100);

	GC gc = XCreateGC(display, window, 0, NULL);
	XFlush(display);  				//flush the output buffer

	//Wait for the window to be visible before drawing bitmap
	XEvent event;
	int sanitycounter = 0;
	while (event.type != MapNotify && sanitycounter < 100){
		XNextEvent(display, &event);
		usleep (10000);
		sanitycounter ++;
	}
	show_bmp(display, window, gc, img);
	XFlush(display);

		// Create two pixmaps with the same depth as the window
		// These "pixmaps" are generally kept directly in video memory so they're fast fast fast.
	Pixmap frontbuffer = XCreatePixmap(display, window, width, height, DefaultDepth(display, screen));
	Pixmap backbuffer = 	XCreatePixmap(display, window, width, height, DefaultDepth(display, screen));
		// Copy the current contents of the window into frontbuffer and backbuffer
	XCopyArea(display, window, frontbuffer, gc, 0, 0, width, height, 0, 0);
	XCopyArea(display, window,  backbuffer, gc, 0, 0, width, height, 0, 0);

			/*
					░░░░░░░█▀▀░█▀█░█▀▄░░░█▀█░█▀▀░░░█▀▀░█▀▀░▀█▀░█░█░█▀█░░░░░░
					░░░░░░░█▀▀░█░█░█░█░░░█░█░█▀▀░░░▀▀█░█▀▀░░█░░█░█░█▀▀░░░░░░
					░░░░░░░▀▀▀░▀░▀░▀▀░░░░▀▀▀░▀░░░░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░░░░░░░░
			**********************************************************************/

		// Flush the output buffer to make sure requests are sent to the X server
	XFlush(display);


    /*******************************************************************************
   ░░░░▄▀░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░▀▀█░░█░░█▀█░█▀▄░░█░░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    ************************* >THIS ONE IS DIFFERENT!< ****************************/
	while (exitflag==0){

		//#PERFORMANCE -Maybe check later and test this to see if it causes any kind of weird performance
		//thing due to hammering this kind of event 30 times/second.
		XRaiseWindow(display, window); //I can't seem to get ANY other tools to do this, so.. -Fraken

		/*****************	EVENT HANDLERS	 ****************/

         // NOTE!!!! X events can come in WAY WAY WAY faster than you damn well think they can.
         // You should pull them until exhaustion like we do here, every single pass.  They can
         // come in faster than your framerate update.
		while (XPending(display) > 0){

			XNextEvent(display, &event);
			/*****   KEYBOARD HANDLERS   *******/
			   /*	I take your case switch and raise you an if, 
               if and if again! -Fraken (I'm a bad programmer) */
			if (event.type == KeyPress) {
      		KeySym keysym = XLookupKeysym(&event.xkey, 0);
         	if (keysym == XK_x) 			{ exitflag = 1; }
				if (keysym == XK_q) 			{ exitflag = 1; }
				if (keysym == XK_Escape) 	{ exitflag = 1; }
      	}
			/******	MOUSE HANDLERS	 ******/
			if (event.type == ButtonPress) {
					//DRAG WINDOW AROUND CODE. -Fraken
        		if (event.xbutton.button == Button1) {  //left mouse down
            	dragging = 1;
               drag_start_x = event.xbutton.x_root;
               drag_start_y = event.xbutton.y_root;
               Window returned_root, returned_child;
               int root_x, root_y;
               unsigned int mask_return;
               // Get current window position
               XTranslateCoordinates(display, window, RootWindow(display, screen),
                                     0, 0, &win_start_x, &win_start_y, &returned_child);
				}
			}
			if (event.type == ButtonRelease) {	//left mouse up -Fraken
        		if (event.xbutton.button == Button1) {
                  //eventually we should do a location_get/location_set
                  // here since the user will probably appreciate the window
                  // showing up in the same place as they put it last time.
					dragging = 0;
				}
			}
			if (event.type == MotionNotify) {	//mouse move -Fraken
            if (dragging && (event.xmotion.state & Button1Mask)) {
               int new_x = win_start_x + (event.xmotion.x_root - drag_start_x);
               int new_y = win_start_y + (event.xmotion.y_root - drag_start_y);
               XMoveWindow(display, window, new_x, new_y);
            }
			}
		}

		/********************	START OF GRAPHICAL STUFF	*********************/

			//Copy Backbuffer to the front buffer, aka, our original to our working copy
		XCopyArea(display, backbuffer, frontbuffer, gc, 0, 0, width, height, 0, 0);   
			//********************************* WRITING STUFF TO THE BACKBUFFER **********

			//drawCPU (Display *display, Drawable frontbuffer, GC gc);
		drawCPU(display, frontbuffer, gc);
		drawDISK(display, frontbuffer, gc);
		drawNETWORK(display, frontbuffer, gc);
		drawSWAP(display, frontbuffer, gc);

			// Blit the front buffer to the window.  Or whatever the hell you call it in Xlib programming. -Fraken
		XCopyArea(display, frontbuffer, window, gc, 0, 0, width, height, 0, 0);
		usleep(localsleepvalue); //This really *does* keep cpu use down.
	}
    /***********************************************************************
   ░░░░▄▀░░░█▀▀░█▀█░█▀▄░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░█▀▀░█░█░█░█░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░▀░▀░▀▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    **** SOMEONE DECIDED TO TERMINATE THE MAGIC.  *SAD FACE*!! -FRAKEN ****/

		/* Do the nice clean up things we're supposed to do */

		// WE SHOULD DELETE OUR PIXMAP DATA AS WELL.

		//img is the name of our BMP object  Bye bitmap! *sad face* -Fraken.
	free(img);
		//Free The pixmap data in video memory.  You were a good block(s) of ram 39x39px... -Fraken
	XFreePixmap(display, frontbuffer);
	XFreePixmap(display, backbuffer);
	//XDestroyImage(image); // I think this is orphaned from before I decided to use pixmaps in vram.
	XFreeGC(display, gc);
	XCloseDisplay(display);

   printf("Graphics thread exiting.\n");
	return NULL;
}