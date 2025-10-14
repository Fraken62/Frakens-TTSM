

/*******************************************************

 ▄▄▄▄▄  ▄▄   ▄   ▄▄▄  ▄      ▄    ▄ ▄▄▄▄   ▄▄▄▄▄▄  ▄▄▄▄ 
   █    █▀▄  █ ▄▀   ▀ █      █    █ █   ▀▄ █      █▀   ▀
   █    █ █▄ █ █      █      █    █ █    █ █▄▄▄▄▄ ▀█▄▄▄ 
   █    █  █ █ █      █      █    █ █    █ █          ▀█
 ▄▄█▄▄  █   ██  ▀▄▄▄▀ █▄▄▄▄▄ ▀▄▄▄▄▀ █▄▄▄▀  █▄▄▄▄▄ ▀▄▄▄█▀

********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/if_mib.h>
#include <sys/socket.h>
#include <sys/types.h>    
#include <sys/sysctl.h>   

#include "network.h"
#include "globals.h"

 /*********************************************************************************************
░░░░░░▄▀░░░█▀▄░█▀▀░█▀▀░█░░░█▀█░█▀▄░█▀▀░█▀▀░░░█▀█░█▀█░█▀▄░░░█▀▀░▀█▀░█▀▄░█░█░█▀▀░▀█▀░█▀▀░░░▀▄░░░░░
░░░░░▀▄░░░░█░█░█▀▀░█░░░█░░░█▀█░█▀▄░█▀▀░▀▀█░░░█▀█░█░█░█░█░░░▀▀█░░█░░█▀▄░█░█░█░░░░█░░▀▀█░░░░▄▀░░░░
░░░░░░░▀░░░▀▀░░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀░▀░▀▀▀░▀▀▀░░░▀░▀░▀░▀░▀▀░░░░▀▀▀░░▀░░▀░▀░▀▀▀░▀▀▀░░▀░░▀▀▀░░░▀░░░░░░
 *********************************************************************************************/
 /* None! :P  Tee hee.  -Fraken. */

 /***********************************************************************
░░░░░░▄▀░░░█░█░█▀█░█▀▄░█░█░░░█▀▀░█░█░█▀█░█▀▀░▀█▀░▀█▀░█▀█░█▀█░█▀▀░░░▀▄░░░░░
░░░░░▀▄░░░░█▄█░█░█░█▀▄░█▀▄░░░█▀▀░█░█░█░█░█░░░░█░░░█░░█░█░█░█░▀▀█░░░░▄▀░░░░
░░░░░░░▀░░░▀░▀░▀▀▀░▀░▀░▀░▀░░░▀░░░▀▀▀░▀░▀░▀▀▀░░▀░░▀▀▀░▀▀▀░▀░▀░▀▀▀░░░▀░░░░░░
 ***********************************************************************/

// Per-interface tracking structure
typedef struct iface_stat {
    int ifindex;
    char name[IFNAMSIZ];
    unsigned long long last_ibytes;
    unsigned long long last_obytes;
    struct iface_stat *next;
} iface_stat_t;

static iface_stat_t *ifaces = NULL;

// ---------- PART 1: Initialization ----------
   void init_interfaces(void) {
      int ifcount = 0;
      size_t len = sizeof(ifcount);

      if (sysctlbyname("net.link.generic.system.ifcount",
                        &ifcount, &len, NULL, 0) < 0) {
         perror("sysctl ifcount");
         exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= ifcount; i++) {
         struct ifmibdata ifmd;
         int mib[6] = { CTL_NET, PF_LINK, NETLINK_GENERIC,
                        IFMIB_IFDATA, i, IFDATA_GENERAL };
         len = sizeof(ifmd);

         if (sysctl(mib, 6, &ifmd, &len, NULL, 0) < 0)
               continue;

         // Skip non-active interfaces
         if (!(ifmd.ifmd_data.ifi_link_state == LINK_STATE_UP))
               continue;

         iface_stat_t *ifs = calloc(1, sizeof(*ifs));
         if (!ifs) {
               perror("calloc");
               exit(EXIT_FAILURE);
         }

         ifs->ifindex = i;
         strncpy(ifs->name, ifmd.ifmd_name, IFNAMSIZ);
         ifs->last_ibytes = ifmd.ifmd_data.ifi_ibytes;
         ifs->last_obytes = ifmd.ifmd_data.ifi_obytes;
         ifs->next = ifaces;
         ifaces = ifs;

         //printf("Tracking interface: %s (index %d)\n",
         //      ifs->name, i);
    }
}

// ---------- PART 2: Main Loop ----------
double sample_interfaces(void) {
    double currentsample = 0.0;

    for (iface_stat_t *ifs = ifaces; ifs != NULL; ifs = ifs->next) {
        struct ifmibdata ifmd;
        size_t len = sizeof(ifmd);
        int mib[6] = { CTL_NET, PF_LINK, NETLINK_GENERIC,
                       IFMIB_IFDATA, ifs->ifindex, IFDATA_GENERAL };

        if (sysctl(mib, 6, &ifmd, &len, NULL, 0) < 0)
            continue;

        unsigned long long new_in = ifmd.ifmd_data.ifi_ibytes;
        unsigned long long new_out = ifmd.ifmd_data.ifi_obytes;

        unsigned long long delta_in = new_in - ifs->last_ibytes;
        unsigned long long delta_out = new_out - ifs->last_obytes;

        // Debug print
        //printf("[%s] Δin=%llu Δout=%llu\n",
        //       ifs->name, delta_in, delta_out);

        ifs->last_ibytes = new_in;
        ifs->last_obytes = new_out;

        currentsample += (double)(delta_in + delta_out);
    }

    return currentsample;
}

// ---------- PART 3: Cleanup ----------
void cleanup(void) {
    iface_stat_t *curr = ifaces;
    while (curr) {
        iface_stat_t *next = curr->next;
        free(curr);
        curr = next;
    }
    ifaces = NULL;
}

/**************************************************************************************

 ▄▄   ▄  ▄▄▄▄ ▄▄▄▄▄▄▄   ▄▄▄▄▄  ▄▄   ▄▄▄▄▄▄▄▄   ▄    ▄   ▄▄   ▄▄▄▄▄  ▄▄   ▄      █   █   
 █▀▄  █ ▄▀  ▀▄   █        █    █▀▄  █   █      ██  ██   ██     █    █▀▄  █     █     █  
 █ █▄ █ █    █   █        █    █ █▄ █   █      █ ██ █  █  █    █    █ █▄ █    █       █  
 █  █ █ █    █   █        █    █  █ █   █      █ ▀▀ █  █▄▄█    █    █  █ █     █     █  
 █   ██  █▄▄█    █      ▄▄█▄▄  █   ██   █      █    █ █    █ ▄▄█▄▄  █   ██      █   █   

******************  BUT IT SURE ACTS LIKE IT DUDN'T IT?  -FRAKEN   ********************/

void* network() {

   printf("Network thread started.\n");

    /*************************************************************************************************
   ░░░░▄▀░░░█▀▄░█▀▀░█▀▀░█░░░█▀█░█▀▄░█▀█░▀█▀░▀█▀░█▀█░█▀█░█▀▀░░░█▀█░█▀█░█▀▄░░░█▀▀░█▀▀░▀█▀░█░█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░█░█░█▀▀░█░░░█░░░█▀█░█▀▄░█▀█░░█░░░█░░█░█░█░█░▀▀█░░░█▀█░█░█░█░█░░░▀▀█░█▀▀░░█░░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀░░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀░▀░▀░▀░░▀░░▀▀▀░▀▀▀░▀░▀░▀▀▀░░░▀░▀░▀░▀░▀▀░░░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░░░░░▀░░░░
    ****   AND VARIOUS OTHER THINGS MAYBE... OR MAYBE NOT! WOO LOTS OF SPACE TO TYPE HERE!! :D   ****/

   /*<><><><><><><><><>((((((  GUI AND CONTROL VARIABLES AND SUCH  ))))))<><><><><><><><><>*/
   int localsleepvalue = (1000000/framerate)*3;
   init_interfaces();

    /*******************************************************************************
   ░░░░▄▀░░░█▀▀░▀█▀░█▀█░█▀▄░▀█▀░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░▀▀█░░█░░█▀█░█▀▄░░█░░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░░▀░░▀░▀░▀░▀░░▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    *************  THIS IS WHERE THE MAGIC HAPPENS BABY! WOO!  -FRAKEN  ***********/
   while (exitflag==0){

         //I might want to change this later.  I don't really care for the 0.05% CPU it uses at 30
         // calls per second at 3.1 ghz.  That's like a full 0.1% at full screen refresh. -Fraken
      networkcurrent = sample_interfaces();
      //This is where we NAP
      usleep(localsleepvalue);
   }
    /***********************************************************************
   ░░░░▄▀░░░█▀▀░█▀█░█▀▄░░░█▀█░█▀▀░░░█▄█░█▀█░▀█▀░█▀█░░░█░░░█▀█░█▀█░█▀█░░░▀▄░░░
   ░░░▀▄░░░░█▀▀░█░█░█░█░░░█░█░█▀▀░░░█░█░█▀█░░█░░█░█░░░█░░░█░█░█░█░█▀▀░░░░▄▀░░
   ░░░░░▀░░░▀▀▀░▀░▀░▀▀░░░░▀▀▀░▀░░░░░▀░▀░▀░▀░▀▀▀░▀░▀░░░▀▀▀░▀▀▀░▀▀▀░▀░░░░░▀░░░░
    **** SOMEONE DECIDED TO TERMINATE THE MAGIC.  *SAD FACE*!! -FRAKEN ****/

   cleanup();
   printf("Network thread exiting.\n");
   return NULL;
}