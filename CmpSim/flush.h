/* 
 * File:   flush.h
 * Author: kartheek
 *
 * Created on 8 April, 2014, 4:53 PM
 */

#ifndef FLUSH_H
#define	FLUSH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "hashmap.h"
#include "coreconfig.h"
    
    void flush_all();
    void flush_hashtable();
    void flush_cache();
    void flush_cores();


#ifdef	__cplusplus
}
#endif

#endif	/* FLUSH_H */

