/* 
 * File:   main.c
 * Author: kartheek
 *
 * Created on March, 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cachebank.h"
#include "config.h"
#include "coreconfig.h"
#include "stats.h"
#include "hashmap.h"
#include "flush.h"


/* execution start/end times */
time_t cmp_start_time;
time_t cmp_end_time;
int cmp_elapsed_time;


/* random number generator seed */
static int rand_seed;


int main(int argc, char** argv) {
    
    ioConfig();
    
    cbankGridConfig();
    
    init_hashtable();
    
    inputfiles_create();
    
    coresConfig();
    
    ofileConfig();
    
    flush_all();
    
    return (EXIT_SUCCESS);
}
