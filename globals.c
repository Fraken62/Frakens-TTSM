#include "globals.h"

   //GLOBALS!  What a good idea! -Fraken (C.G. Fraser)

   //These variables are for parsing/passing/evaluating
   //the actual data and passing it around because
   //I am a bad programmer and this seemed like a
   //good idea. -Fraken

   //If this gets released before I delete this, there's orphaned stuff all over the place here.  Sadly, this is
   // purely just down to my working memory deficits.  The truth is, I can't even recall what half of these are for
   // and *I* wrote the damn things. (Update) God this code is aweful.  Just aweful.  How does it even compile? -Fraken.

   //let's start demarking these post mid write so we actually know wtf we're doing here.  This stupid memory
   // and visual blindness thing is bullshit!.  -Fraken.
float cpucurrent                    =  0.0f;
float cpulast                       =  0.0f;
float cpumax                        =  0.0f;
float cpufast                       =  0.0f;

float diskcurrent                   =  0.0f;
float diskaverage                      =  0.0f;
float diskmax                       =  0.0f;

   // Swap doesn't bounce around, I COULD have just had one variable.
float swapcurrent                   =  0.0f;
float swaplast                      =  0.0f;
float swapmax                       =  0.0f; // Swap is unique and will only ever be a percentage pulled from pstat/swapinfo

float test                          =  0.0f; //for fiddling around. 

   // These ended up having some long longs due to internal network.c precision/inherent counter datatypes, etc.
unsigned long long networkcurrent   =  0;
double networklastpercentage        =  0;
unsigned long long networkmax       =  0;

    //These are just control variables of various kinds.
int exitflag                        =   0;  // Once this is set to 1, EVERY thread cleans up and exits.  *ALL* of them.
int framerate                       =  30;  // obvious.  Frames to render per second (roughly)
int fastsmoothing                   =  2;   // This is "realtime" smoothing, only high enough to account for 30x framerate.
int smoothingvalue                  =  60;  // smooth over this many frames (1 second by default.)
int networknormalize                =   0;  // The amount to reduce the network "max" value by for each pass.
int disknormalize                   =   0;

// This is the name of the background artwork
//    image we use for the UI.
char backgroundfilename[15] = "background.bmp";