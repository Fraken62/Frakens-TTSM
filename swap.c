#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "swap.h"
#include "globals.h"

/**************************************************************************************

 ▄▄   ▄  ▄▄▄▄ ▄▄▄▄▄▄▄   ▄▄▄▄▄  ▄▄   ▄▄▄▄▄▄▄▄   ▄    ▄   ▄▄   ▄▄▄▄▄  ▄▄   ▄      █   █   
 █▀▄  █ ▄▀  ▀▄   █        █    █▀▄  █   █      ██  ██   ██     █    █▀▄  █     █     █  
 █ █▄ █ █    █   █        █    █ █▄ █   █      █ ██ █  █  █    █    █ █▄ █    █       █  
 █  █ █ █    █   █        █    █  █ █   █      █ ▀▀ █  █▄▄█    █    █  █ █     █     █  
 █   ██  █▄▄█    █      ▄▄█▄▄  █   ██   █      █    █ █    █ ▄▄█▄▄  █   ██      █   █   

***************************************************************************************/

void* swap() {

   /*
      Until we get a chance to really dig in and sort out the swap stuff, we're just going to grab this
      stuff from 'pstat -s' and some shell crap.
   */

    /*************************************************************************************************
   ░░░░▄▀░░░█▀▄░█▀▀░█▀▀░█░░░█▀█░█▀▄░█▀█░▀█▀░▀█▀░█▀█░█▀█░█▀▀░░░█▀█░█▀█░█▀▄░░░█▀▀░█▀▀░▀█▀░█░█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░█░█░█▀▀░█░░░█░░░█▀█░█▀▄░█▀█░░█░░░█░░█░█░█░█░▀▀█░░░█▀█░█░█░█░█░░░▀▀█░█▀▀░░█░░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀░░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀░▀░▀░▀░░▀░░▀▀▀░▀▀▀░▀░▀░▀▀▀░░░▀░▀░▀░▀░▀▀░░░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░░░░░▀░░░░
    *************************************************************************************************/

   printf("Swap thread started.\n");
   int localsleepvalue = framerate/3;

    /*******************************************************************************
   ░░░░▄▀░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░▀▀█░░█░░█▀█░█▀▄░░█░░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    ************************* >THIS ONE IS DIFFERENT!< ****************************/
   while (exitflag==0){

      // "But Fraken!  You could just use some KVM interface and not have to do all this!"
      // I spent a WEEK fiddling and farting around with it and for the bloody life of me,
      // I just can't get it to work.  This only pops about once every 5 seconds anyway
      // and swap doesn't change THAT often so this will have to do for the initial release.
      // I mean, the damn thing uses 0.02% CPU on my machine at 30fps... It's fine... -Fraken.

      char buffer[300];
      FILE *fp = popen("pstat -s | tail -1 | awk '{sub(\"%\",\"\",$NF); print $NF}'", "r");
      if (fp == NULL) {
         perror("popen failed");
         exitflag=1;
         printf("\n\nThere was an error opening pstat, tail or awk.  Both must exist on system/path.\n\n");
         exit(EXIT_FAILURE);
      }
      if (fgets(buffer, sizeof(buffer), fp) != NULL) {
         swapcurrent = atoi(buffer);
      } else {
         perror("Failed to read pstat output, check that pstat, tail and awk exist on system. -Fraken.");
         exitflag = 1;
      }
      
      /*** MAKE IT SAFE OR THE GUI MIGHT EXPLODE (OR JUST LOOK WEIRD ***/
      if (swapcurrent<0)   {swapcurrent=0;}
      if (swapcurrent>100) {swapcurrent=100;}
      pclose(fp);

      sleep(localsleepvalue);
   }
    /***********************************************************************
   ░░░░▄▀░░░█▀▀░█▀█░█▀▄░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░█▀▀░█░█░█░█░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░▀░▀░▀▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    *************** WE GO BYE BYE NAO!  *SAD FACE*!! -FRAKEN **************/

   printf("Swap thread exiting.\n");
   return NULL; 
}