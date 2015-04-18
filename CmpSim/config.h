/* 
 * File:   misc.h
 * Author: kartheek
 *
 * Created on 24 March, 2014, 6:53 PM
 */

#ifndef MISC_H
#define	MISC_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include<string.h>
    
#include "global.h"

    
    void ioConfig();
    void inputfiles_create();
    cluster* cluster_creat(int clustnum);
    void cbankGridConfig();
    
#ifdef	__cplusplus
}
#endif

#endif	/* MISC_H */
