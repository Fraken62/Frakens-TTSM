#ifndef GLOBALS_H
#define GLOBALS_H

// The 'extern' keyword tells the compiler that these variables are defined
// in another source file and should not be allocated here.

// Ugh, I'm going to demark stuff into local context size/scale.  Worst description ever.  -Fraken.
//  Am I an angry coder!?!??!! :o -Fraken.

//Working data for basic statistics.
extern float    cpucurrent, cpulast, cpumax;  
extern unsigned long long  networkcurrent, networkmax;
extern double   networklastpercentage;
extern float    diskcurrent, diskaverage, diskmax;
extern float    swapcurrent, swaplast, swapmax;

//For realtime/whitebar smoothing
extern float cpufast, diskfast, networkfast;

// for fiddling around.
extern float test;

//Control and config stuff
extern int      exitflag;
extern int      framerate;
extern int      smoothingvalue;
extern int      fastsmoothing;
extern int      networknormalize;
extern int      disknormalize;

extern char     backgroundfilename[15];

#endif // GLOBALS_H