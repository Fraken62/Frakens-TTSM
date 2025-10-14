

   /* STANDARD C LIBRARIES AND SUCH - Fraken */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <devstat.h>   // for devstat API
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>

   /* THESE ARE MINE  -Fraken */
#include "disk.h"
#include "globals.h"

   // This reports the stats to anyone who asks -Fraken
// Global variable provided by your project
extern float diskcurrent;
useconds_t localsleepvalue;  // A.I. figured this should be an extern usec_t var.  It be wrong. -Fraken.


   // openai claims this is the same approach as iostat.  I shall check its claims later.  -Fraken
static struct statinfo cur, last;

void init_disks(void) {
    if (devstat_checkversion(NULL) == -1) {
        perror("devstat_checkversion");
        exit(1);
    }

    memset(&cur, 0, sizeof(cur));
    memset(&last, 0, sizeof(last));

    cur.dinfo = calloc(1, sizeof(struct devinfo));
    last.dinfo = calloc(1, sizeof(struct devinfo));

    if (!cur.dinfo || !last.dinfo) {
        perror("calloc");
        exit(1);
    }

    if (devstat_getdevs(NULL, &cur) == -1) {
        perror("devstat_getdevs(cur)");
        exit(1);
    }
    if (devstat_getdevs(NULL, &last) == -1) {
        perror("devstat_getdevs(last)");
        exit(1);
    }
}

void sample_disks(void) {

    // swap current and last
    // Note that last is NOT disklast used for global GUI stuff.
    // Also, disklast should have been local go graphics.c, but v2.0 amirite? :P  -Fraken.
    struct statinfo tmp = last;
    last = cur;
    cur = tmp;

    if (devstat_getdevs(NULL, &cur) == -1) {
        perror("devstat_getdevs(cur)");
        return;
    }

    double total_delta = 0.0;

    for (int i = 0; i < cur.dinfo->numdevs; i++) {
        struct devstat *cds = &cur.dinfo->devices[i];
        struct devstat *lds = &last.dinfo->devices[i];

        // don’t filter on type for now, just to confirm something changes
        uint64_t rbytes = 0, wbytes = 0;

        if (devstat_compute_statistics(cds, lds, 1,
                                       DSM_TOTAL_BYTES_READ, &rbytes,
                                       DSM_TOTAL_BYTES_WRITE, &wbytes,
                                       DSM_NONE) != 0) {
            continue;
        }

        total_delta += (double)rbytes + (double)wbytes;
    }

    double mb = total_delta / (1024.0 * 1024.0);
    double sec = (double)localsleepvalue / 1e6;

    diskcurrent = (sec > 0.0) ? (float)(mb / sec) : 0.0f;

    //printf("Total Delta: %.2f MB, diskcurrent=%.2f MB/s\n", mb, diskcurrent);
}

void cleanup_disks(void) {
    free(cur.dinfo);
    free(last.dinfo);
}


/**************************************************************************************

 ▄▄   ▄  ▄▄▄▄ ▄▄▄▄▄▄▄   ▄▄▄▄▄  ▄▄   ▄▄▄▄▄▄▄▄   ▄    ▄   ▄▄   ▄▄▄▄▄  ▄▄   ▄      █   █   
 █▀▄  █ ▄▀  ▀▄   █        █    █▀▄  █   █      ██  ██   ██     █    █▀▄  █     █     █  
 █ █▄ █ █    █   █        █    █ █▄ █   █      █ ██ █  █  █    █    █ █▄ █    █       █  
 █  █ █ █    █   █        █    █  █ █   █      █ ▀▀ █  █▄▄█    █    █  █ █     █     █  
 █   ██  █▄▄█    █      ▄▄█▄▄  █   ██   █      █    █ █    █ ▄▄█▄▄  █   ██      █   █   

***************************************************************************************/

void* disk() {

   printf("Disk thread started.\n");
      // local sleep value was originally an int, but A.I. is right,
      // I SHOULD use a useconds_t datatype. -Fraken
   localsleepvalue = (1000000/framerate)*3;
   init_disks();

    /*******************************************************************************
   ░░░░▄▀░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░▀▀█░░█░░█▀█░█▀▄░░█░░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    ************************* >THIS ONE IS DIFFERENT!< ****************************/
   while (exitflag==0){
      sample_disks();
      //printf("Disk Current Sum: %f\n", diskcurrent);
      usleep(localsleepvalue);
   }
    /***********************************************************************
   ░░░░▄▀░░░█▀▀░█▀█░█▀▄░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░█▀▀░█░█░█░█░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░▀░▀░▀▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    *************** WE GO BYE BYE NAO!  *SAD FACE*!! -FRAKEN **************/
   cleanup_disks();

   printf("Disk thread exiting.\n");
   return NULL;
}