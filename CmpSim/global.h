/* 
 * File:   global.h
 * Author: kartheek
 *
 * Created on 7 April, 2014, 8:01 PM
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
    
    int cores;                  //number of cores
    int csize;                  //cache size in MB
    int cn;                     //number of bits in cache
    int bsassoc;                //bank set associativity
    int bassoc;                 //bank associativity
    int bsize;                  //bank size in Bytes
    int lsize;                  //block size in Bytes
    int nbanks;                 //number of banks
    int nlines;                 //number of lines in cache
    int nsets;                  //number of sets per bank
    int nlinesb;                //number of lines per bank
    int nclusters;              //number of clusters
    int nbankscluster;          //number of banks per cluster
    
    FILE *output_fd;            //Output file descriptor 
    
   /* cache block (or line) definition */
    struct Cache_block{
        int address;
        int data;
        int naccesses;
        time_t last_accessed;
        int tag;
        int state;                  // 0 for None, 8(1) for Demoted, 128(2) for Promoted
        int busy;
    };    
    typedef struct Cache_block cache_block;

    /* cache set definition */
    struct Set{
        cache_block **ways;
        int *hk_nuka;
    };    
    typedef struct Set set;

    /* cache bank definition */
    struct Cache_bank
    {
        int banknum;
        int clustnum;
        int nlinesalloc;
        set **sets;
        int misses;
        int hits;
        int replacements;
    };
    typedef struct Cache_bank cache_bank;

    struct Cluster{
        int row;
        int column;
        cache_bank ***cachebank;
    };
    typedef struct Cluster cluster;
    
    struct Grid{
        int row;
        int column;
        cluster ***grid;
    };
    typedef struct Grid cache_grid;
    
    cache_grid cgrid;
    cache_grid cgrid1;
    cache_grid cgrid2;
    
    /* home parameters */
    struct homepar{
        int hr1;
        int hc1;
        int hr2;
        int hc2;
        int setnum;
    };
    typedef struct homepar home;
    
    /* core parameters */
    struct coreproperties{
        int naccesses;
        int blockmovements;
        int nhits;
        int missesperbank;
        int misses;
        int nrepl;
        int nalloc;
    };
    typedef struct coreproperties corethread;
    
    corethread *coreprop0,*coreprop1,*coreprop2;
    int *way,*way1,*way2,*skipsearches;
    
    char* file_name(int p);
    
#ifdef	__cplusplus
}
#endif

#endif	/* GLOBAL_H */

