
   /* STANDARD C LIBRARIES AND SUCH - Fraken */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>

   /* THESE ARE MINE  -Fraken */
#include "cpu.h"
#include "globals.h"


/**************************************************************************************

 ▄▄   ▄  ▄▄▄▄ ▄▄▄▄▄▄▄   ▄▄▄▄▄  ▄▄   ▄▄▄▄▄▄▄▄   ▄    ▄   ▄▄   ▄▄▄▄▄  ▄▄   ▄      █   █   
 █▀▄  █ ▄▀  ▀▄   █        █    █▀▄  █   █      ██  ██   ██     █    █▀▄  █     █     █  
 █ █▄ █ █    █   █        █    █ █▄ █   █      █ ██ █  █  █    █    █ █▄ █    █       █  
 █  █ █ █    █   █        █    █  █ █   █      █ ▀▀ █  █▄▄█    █    █  █ █     █     █  
 █   ██  █▄▄█    █      ▄▄█▄▄  █   ██   █      █    █ █    █ ▄▄█▄▄  █   ██      █   █   

***************************************************************************************/

void* cpu(){

   printf("CPU thread started.\n");

      //Thanks to some nice people at stack overflow for most of the heavy lifting 
      // that we're doign here! (Edmond Meinfelder) We needed a bit of A.I.
      // help and a bit of manual human kerjiggering/refactoring to get things working.  -Fraken.

      // Define various things/named constants/macro's that the C preprocessor will use. -Fraken
      // Example: At compiler time "CP_USER" will be replaced with "0". (Thanks Perplexity.)
   #define CP_USER   0
   #define CP_NICE   1
   #define CP_SYS    2
   #define CP_INTR   3
   #define CP_IDLE   4
   #define CPUSTATES 5

      //Declare various variables of various types that we'll need.
   uint64_t cur[CPUSTATES], last[CPUSTATES];
   size_t cur_sz = sizeof(cur);
   int state;
   long double total_diff;
   int first_sample = 1;

      //Initialize our array and set the values to zero.
   memset(last, 0, sizeof(last));
      //Set the rate at which we'll poll the CPU stats here at 10x/second.
   int localsleepvalue = (1000000/framerate)*3;

    /*******************************************************************************
   ░░░░▄▀░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░▀▀█░░█░░█▀█░█▀▄░░█░░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    ************************* >THIS ONE IS DIFFERENT!< ****************************/
   while (exitflag==0){

         // Fetch current CPU times from the kernel
      if (sysctlbyname("kern.cp_time", &cur, &cur_sz, NULL, 0) < 0){
         printf("The CPU thread failed while trying to fetch time data via kern.cp_time.  Call Fraken (No, don't)\n");
         exitflag = 1;
         return NULL;
      }

      //printf("\nRan to just before first sample.");

      if (!first_sample){
         total_diff = 0.0L;
         // Calculate delta ticks and sum
         for (state = 0; state < CPUSTATES; state++){
            uint64_t diff = cur[state] - last[state];
            last[state] = cur[state];
            total_diff += (long double)diff;
            cur[state] = diff; // reuse cur array to store deltas for clarity
         }
         if (total_diff == 0.0L){
            total_diff = 1.0L;
         } // avoid division by zero
            // Calculate CPU utilization as percentage (100% - idle%)
         cpucurrent = 100.0L - (100.0L * (long double)cur[CP_IDLE] / total_diff);
      }
      else{
         // For the first sample, just store values, no delta calc yet
         memcpy(last, cur, sizeof(cur));
         first_sample = 0;
      }
      usleep(localsleepvalue);
   }
    /***********************************************************************
   ░░░░▄▀░░░█▀▀░█▀█░█▀▄░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░█▀▀░█░█░█░█░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░▀░▀░▀▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    *************** WE GO BYE BYE NAO!  *SAD FACE*!! -FRAKEN **************/
   printf("CPU thread exiting.\n");
   return NULL;
}