#include "stats.h"

void ofileConfig(){

 output_fd = NULL;

 output_fd = fopen("iofiles/output","w");

 if(output_fd){
        
        time_t ltime; /* calendar time */
        ltime=time(NULL); /* get current cal time */

        fprintf(output_fd,"TimeStamp : %s",asctime( localtime(&ltime) ));
        fprintf(output_fd,"Input data\n");
        fprintf(output_fd,"------------------------------------------------------------\n"); 
        fprintf(output_fd,"Number of cores in the system : %d \n",cores);  
        fprintf(output_fd,"cache size : %d MB \ncache line size : %d Bytes \n",csize,lsize);
        fprintf(output_fd,"bank set associativity : %d \nbank associativity : %d \n\n",bsassoc,bassoc);

        add_hitmissreplace();
        
        fprintf(output_fd,"\n------------------------------------------------------------\n"); 
        fprintf(output_fd,"Results for two basic algorithms & my algorithm (HKState-NUCA)\n");        
        fprintf(output_fd,"------------------------------------------------------------\n");
        
        fprintf(output_fd,"cache lines accessed : %d\n\n",totallinesaccessed);
        
        fprintf(output_fd,"\n------------------------------------------------------------\n");        
        fprintf(output_fd,"Linear Search Results\n");
        fprintf(output_fd,"------------------------------------------------------------\n"); 
        fprintf(output_fd,"hits : %d\nmisses : %d\nreplacements : %d\n\nmisses for all the banks: %d\nhops (search requests) : %d\nblock movements : %d\n"
                ,totalhits,totalmisses,totalreplacements,totalmissesperbank,totalhops,totalmovements);

        fprintf(output_fd,"\n------------------------------------------------------------\n"); 
        fprintf(output_fd,"HK-NUCA Results\n");
        fprintf(output_fd,"------------------------------------------------------------\n");
        fprintf(output_fd,"hits : %d\nmisses : %d\nreplacements : %d\n\nmisses for all the banks: %d\nhops (search requests) : %d\nblock movements : %d\n"
                ,totalhits1,totalmisses1,totalreplacements1,totalmissesperbank1,totalhops1,totalmovements1);

        fprintf(output_fd,"\n------------------------------------------------------------\n"); 
        fprintf(output_fd,"HKState-NUCA Results\n");
        fprintf(output_fd,"------------------------------------------------------------\n");
        fprintf(output_fd,"hits : %d\nmisses : %d\nreplacements : %d\n\nmisses for all the banks: %d\nhops (search requests) : %d\nblock movements : %d\n"
                ,totalhits2,totalmisses2,totalreplacements2,totalmissesperbank2,totalhops2,totalmovements2);

        fclose(output_fd);        
 }
}

void add_hitmissreplace(){
    int i;    
    
    for(i = 0;i < cores;i++){
        totallinesaccessed = totallinesaccessed + coreprop1[i].naccesses;
        
        totalmovements = totalmovements + coreprop0[i].blockmovements;
        totalhits = totalhits + coreprop0[i].nhits;
        totalmissesperbank = totalmissesperbank + coreprop0[i].missesperbank;
        totalmisses = totalmisses + coreprop0[i].misses;
        totalreplacements = totalreplacements + coreprop0[i].nrepl;
        totallinesalloc = totallinesalloc + coreprop0[i].nalloc;
        
                
        totalmovements1 = totalmovements1 + coreprop1[i].blockmovements;
        totalhits1 = totalhits1 + coreprop1[i].nhits;
        totalmissesperbank1 = totalmissesperbank1 + coreprop1[i].missesperbank;
        totalmisses1 = totalmisses1 + coreprop1[i].misses;
        totalreplacements1 = totalreplacements1 + coreprop1[i].nrepl;
        totallinesalloc1 = totallinesalloc1 + coreprop1[i].nalloc;
        
        totalmovements2 = totalmovements2 + coreprop2[i].blockmovements;
        totalhits2 = totalhits2 + coreprop2[i].nhits;
        totalmissesperbank2 = totalmissesperbank2 + coreprop2[i].missesperbank;
        totalmisses2 = totalmisses2 + coreprop2[i].misses;
        totalreplacements2 = totalreplacements2 + coreprop2[i].nrepl;
        totallinesalloc2 = totallinesalloc2 + coreprop2[i].nalloc;
        
        skippedsearches = skippedsearches + skipsearches[i];
    }
    
    totalhops = totalmissesperbank + totalhits;
    totalhops1 = totalmissesperbank1 + totalhits1;
    totalhops2 = totalmissesperbank2 + totalhits2;
    
}

