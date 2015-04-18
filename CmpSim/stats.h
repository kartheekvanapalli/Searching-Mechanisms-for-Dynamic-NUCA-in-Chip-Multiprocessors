/* 
 * File:   stats.h
 * Author: kartheek
 *
 * Created on 4 April, 2014, 8:01 AM
 */

#ifndef STATS_H
#define	STATS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "global.h"
#include "cachebank.h"
    
    int totallinesaccessed;
    
    int totalhits;
    int totalmisses;
    int totalmissesperbank;
    int totalreplacements;
    int totallinesalloc;
    int totalmovements;
    int totalhops;
    
    int totalhits1;
    int totalmisses1;
    int totalmissesperbank1;
    int totalreplacements1;
    int totallinesalloc1;
    int totalmovements1;
    int totalhops1;
    
    int totalhits2;
    int totalmisses2;
    int totalmissesperbank2;
    int totalreplacements2;
    int totallinesalloc2;
    int totalmovements2;
    int totalhops2;
    int skippedsearches;
    
void ofileConfig();

void add_hitmissreplace();
    


#ifdef	__cplusplus
}
#endif

#endif	/* STATS_H */

