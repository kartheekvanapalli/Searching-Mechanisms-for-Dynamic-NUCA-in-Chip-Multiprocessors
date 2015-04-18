/* 
 * File:   cache.h
 * Author: kartheek
 *
 * Created on 24 March, 2014, 3:38 PM
 */

#ifndef CACHE_H
#define	CACHE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "global.h"
    
extern pthread_mutex_t hashmutex,mutex,replmutex,mutex1,replmutex1,mutex2,replmutex2;

/* create and initialize a general cache structure */

cache_block * line_init();

set * set_init();

cache_bank * cachebank_init(int clustnum,int banknum);


int lrublock(set *s);

void block_par_assign(cache_block *cblock,int data,int address,int tag,time_t accesstime,int naccesses);


cache_block * cachebank_search(cache_bank *bank,int address,int tag,int p);
cache_block * cacheblock_assign(cache_bank *bank,int address,int tag,int data,int p,int naccesses);

cache_block * cacheblock_replacement(cache_bank *bank,int address,int tag,int data,int p);


set *home_cacheset(int address);

cache_bank *home_cachebank(int address);

cache_block * cachebank_search1(cache_bank *bank,int address,int tag,int p);

cache_block * hknuca_blockassign(cache_bank *bank,int address,int tag,int data,int p,int naccesses);

cache_block * hknuca_replacement(cache_bank *bank,int address,int tag,int data,int p);


void hkptr_setstate(int *hk_ptr,int new);

void hkptr_resetstate(int *hk_ptr,int old);

void hkptr_swapstates(cache_block *cacheblock,int *hk_ptrd,int *hk_ptrs,int new);

cache_block * cachebank_search2(cache_bank *bank,int address,int tag,int p);

cache_block * hkstatenuca_blockassign(cache_bank *bank,int address,int tag,int data,int p,int naccesses,int state);

cache_block * hkstatenuca_replacement(cache_bank *bank,int address,int tag,int data,int p);

#ifdef	__cplusplus
}
#endif

#endif	/* CACHE_H */
