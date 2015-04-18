/* 
 * File:   hashmap.h
 * Author: kartheek
 *
 * Created on 8 April, 2014, 12:48 PM
 */

#ifndef HASHMAP_H
#define	HASHMAP_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "global.h"
#include "cachebank.h"
    
    struct hashnode{
        int address;
        int tag;
        struct hashnode *next;
    };
    
    typedef struct hashnode node;
    node *hashtable;
    
    void init_hashtable();
    void node_assign(node *current,int address,int tag,node *next);
    int hash(int key);
    int insert(int address,int tag);
    void delete(int address,int tag);


#ifdef	__cplusplus
}
#endif

#endif	/* HASHMAP_H */

