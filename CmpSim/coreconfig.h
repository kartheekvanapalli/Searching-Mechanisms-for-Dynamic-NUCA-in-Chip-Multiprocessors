/* 
 * File:   coreconfig.h
 * Author: kartheek
 *
 * Created on 3 April, 2014, 5:37 PM
 */

#ifndef CORECONFIG_H
#define	CORECONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include<pthread.h>

    
#include "cachebank.h"
#include "hashmap.h"

pthread_t *thread,*threadmulticast;

//extern pthread_mutex_t mutex;
//    pthread_t tid = pthread_self();

struct multicastpar{
    cache_bank *cbank;
    cache_block *cblock;
    int address;
    int tag;
    int r;
    int c;
    int p;
}savepar;
typedef struct multicastpar param;


void init_coreprop();
void coresConfig();
int msbposition(int address);
void *dataproc(void *);


cache_block* core_search(int p,int address,int tag);

cache_block* linear_search(int row1,int col1,int row2,int col2,int address,int tag,int p);

cache_block *cacheblock_swap(cache_block *scacheblock,cache_block *dcacheblock);

void block_reset(cache_block *cacheblock);

cache_block* migration(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p);

cache_block* migration_swapping(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p);


void *multicastthread(void *p);

cache_block* multicast_search(int row1,int col1,int row2,int col2,int address,int tag,int p);


cache_block* hknuca_coresearch(int p,int address,int tag);

cache_block* hknuca_search(int row1,int col1,int row2,int col2,int address,int tag,int p);

cache_block* hknuca_migration(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p);



cache_block* hkstatenuca_coresearch(int p,int address,int tag);

cache_block* hkstatenuca_search(int row1,int col1,int row2,int col2,int address,int tag,int p);

cache_block* hkstatenuca_migration(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p);


#ifdef	__cplusplus
}
#endif

#endif	/* CORECONFIG_H */

