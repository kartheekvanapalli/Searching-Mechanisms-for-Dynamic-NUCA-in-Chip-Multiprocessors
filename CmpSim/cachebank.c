
#include "cachebank.h"

pthread_mutex_t hashmutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t replmutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t replmutex1 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t replmutex2 = PTHREAD_MUTEX_INITIALIZER;


cache_block * line_init(){
    cache_block* line;
    
    line = (cache_block *)malloc(sizeof(cache_block));
    line->address = INT_MIN;
    line->tag = INT_MIN;
    line->data = 0;
    line->naccesses = 0;    
    line->last_accessed = time(NULL);
    line->busy = 0;
    line->state = 0;
    
    return line;
}

set * set_init(){
    int i;
    set *cbset = (set *)malloc(sizeof(set));
    
    
    cbset->ways = (cache_block **)malloc( bassoc * sizeof(cache_block *));
    cbset->hk_nuka = (int *)malloc(bsassoc * sizeof(int));
            
    for( i = 0 ; i < bassoc ; i++ ){
        cbset->ways[i] = line_init();
    }
    
    for( i = 0 ; i < bsassoc ; i++ ){
        cbset->hk_nuka[i] = 0;
    }
    
    return cbset;
}

cache_bank * cachebank_init(int clustnum,int banknum){
    int i;
    cache_bank *bank;
      
    bank = (cache_bank *)malloc(sizeof(cache_bank));
    
    bank->banknum = banknum;
    bank->clustnum = clustnum;
    bank->sets = ( set ** ) malloc ( nsets * sizeof( set *));
    
    for( i = 0 ; i < nsets ; i++ ){
        bank->sets[i] = set_init();
    }
            
    bank->nlinesalloc = 0;
    bank->hits = 0;
    bank->misses = 0;
    bank->replacements = 0;
    
    return bank;
}



void block_par_assign(cache_block *cblock,int data,int address,int tag,time_t accesstime,int naccesses){
    cblock->address = address;
    cblock->data = data;
    cblock->tag = tag;
    cblock->last_accessed = accesstime;
    cblock->naccesses = naccesses;
}

int lrublock(set *s){
    time_t current;
    int i,k = 0;
    
    current = time(NULL);
    
    for(i = 0;i < bassoc;i++)
    {
        if(s->ways[i]->last_accessed < current)                 //(difftime(s->ways[i]->last_accessed,current) < 0)
        {
            current = s->ways[i]->last_accessed;
            k = i;
        }
    }
    return k;    
}



cache_block * cachebank_search(cache_bank *bank,int address,int tag,int p){
    cache_block *cacheblock = NULL;
    int i,setnum;
    setnum = address % nsets;
    
    for(i = 0 ; i < bassoc ; i++){
        if( (bank->sets[setnum]->ways[i]->tag == tag ) && (bank->sets[setnum]->ways[i]->address == address)){
            
            (bank->sets[setnum]->ways[i]->naccesses) ++;
            bank->sets[setnum]->ways[i]->last_accessed = time(NULL);
            
            //saving way number for migration purpose
            way[p] = i;
            cacheblock = bank->sets[setnum]->ways[i];
            
            return cacheblock;           
        }
    }
    
    //Cache block not found, MISS
    return cacheblock;
}

cache_block * cacheblock_assign(cache_bank *bank,int address,int tag,int data,int p,int naccesses){
    cache_block *cacheblock = NULL;
    int i,setnum;
    setnum = address % nsets;
    
    for(i = 0 ; i < bassoc ; i++){
        if((bank->sets[setnum]->ways[i]->data == 0) && (bank->sets[setnum]->ways[i]->busy == 0)){
            bank->sets[setnum]->ways[i]->busy = 1;
            
            block_par_assign(bank->sets[setnum]->ways[i],data,address,tag,time(NULL),naccesses);            
            
            bank->sets[setnum]->ways[i]->busy = 0;            
            
            cacheblock = bank->sets[setnum]->ways[i];
            return cacheblock;            
        }
    }
    
    //cache set is full, need replacement    
    return cacheblock;
}

cache_block * cacheblock_replacement(cache_bank *bank,int address,int tag,int data,int p){
    
    int i,l = 0,c,setnum,count = 0;
    
    cache_block *localblock = NULL,*centralblock = NULL;
    int row1,row2,col1,col2;
    
    int banknum = (address / nsets) % nbankscluster;
    
    if(p < cores / 2){
        row1 = 0;
        col1 = p;
    }
    else{
        row1 = 3;
        col1 = p - (cores / 2);
    }
    
    row2 = banknum / 4;    
    col2 = banknum % 4;   
    
    setnum = address % nsets;
    
    l = lrublock(cgrid.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
    
    while(l == -1)
        l = lrublock(cgrid.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
    
    localblock = cgrid.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
    
    if(row1 == 0)
    {
        for(i = 0;i < bassoc;i++)
        {   
            centralblock = cgrid.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]->ways[i];
            if(centralblock->data == 0){
                c = i;              
                break;
            }
            else
                count++;
        }
        if(count == bassoc){
            c = lrublock(cgrid.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]);
            
            (coreprop0[p].nrepl)++;;
            
            // write back the existing central block
        }
        
        centralblock = cgrid.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]->ways[c];       
        
        block_par_assign(centralblock,localblock->data,localblock->address,localblock->tag,localblock->last_accessed,localblock->naccesses);
        
        (coreprop0[p].nrepl)++;
                       
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    
    else if(row1 == 3)
    {
        for(i = 0;i < bassoc;i++)
        {
            centralblock = cgrid.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]->ways[i];
            if(centralblock->data == 0){
                c = i;              
                break;
            }              
            else
                count++;
        }
        if(count == bassoc){
            c = lrublock(cgrid.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]);
            
            (coreprop0[p].nrepl)++;
            // write back the existing central block
        }
        
        centralblock = cgrid.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]->ways[c];        
        
        block_par_assign(centralblock,localblock->data,localblock->address,localblock->tag,localblock->last_accessed,localblock->naccesses);
        
        (coreprop0[p].nrepl)++;
        
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    else
    {
        // write back to main memory
        block_reset(localblock);
        
        (coreprop0[p].nrepl)++;
        
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    
    return localblock;
    
}



cache_bank *home_cachebank(int address){
    int hr1,hc1,hr2,hc2,hclustnum,hbanknum,hsetnum;
    
    hclustnum = (address / (nbankscluster * nsets)) % bsassoc;
    hbanknum = (address / nsets) % nbankscluster; 
    
    hr1 = hclustnum / (cores / 2);
    hc1 = hclustnum % (cores / 2);
    
    hr2 = hbanknum / 4;    
    hc2 = hbanknum % 4;
    
    return cgrid1.grid[hr1][hc1]->cachebank[hr2][hc2];
}

set *home_cacheset(int address){
    set *cacheset;
    int hr1,hc1,hr2,hc2,hclustnum,hbanknum,hsetnum;
    
    hclustnum = (address / (nbankscluster * nsets)) % bsassoc;
    hbanknum = (address / nsets) % nbankscluster;    
    hsetnum = address % nsets;
    
    
    hr1 = hclustnum / (cores / 2);
    hc1 = hclustnum % (cores / 2);
    
    hr2 = hbanknum / 4;    
    hc2 = hbanknum % 4;
    
    cacheset = cgrid1.grid[hr1][hc1]->cachebank[hr2][hc2]->sets[hsetnum];
    
    return cacheset;
}


cache_block * cachebank_search1(cache_bank *bank,int address,int tag,int p){
    cache_block *cacheblock = NULL;
    int i,setnum;
    setnum = address % nsets;
    
    for(i = 0 ; i < bassoc ; i++){
        if( (bank->sets[setnum]->ways[i]->tag == tag ) && (bank->sets[setnum]->ways[i]->address == address)){
            
            (bank->sets[setnum]->ways[i]->naccesses) ++;
            bank->sets[setnum]->ways[i]->last_accessed = time(NULL);
            
            //saving way number for migration purpose
            way1[p] = i;
            cacheblock = bank->sets[setnum]->ways[i];
            
            return cacheblock;           
        }
    }
    
    //Cache block not found, MISS
    return cacheblock;
}

cache_block * hknuca_blockassign(cache_bank *bank,int address,int tag,int data,int p,int naccesses){
    cache_bank *homebank = NULL;
    cache_block *cacheblock = NULL;
    set *cacheset = NULL;
    int i,setnum;
    setnum = address % nsets;
    
    for(i = 0 ; i < bassoc ; i++){
        if(bank->sets[setnum]->ways[i]->data == 0){
            
            cacheset = home_cacheset(address);
            homebank = home_cachebank(address);
            
            if(bank != homebank)
                (cacheset->hk_nuka[bank->clustnum])++; 
            
            block_par_assign(bank->sets[setnum]->ways[i],data,address,tag,time(NULL),naccesses);  
                        
            cacheblock = bank->sets[setnum]->ways[i];
            return cacheblock;            
        }
    }
    
    //cache set is full, need replacement    
    return cacheblock;
}

cache_block * hknuca_replacement(cache_bank *bank,int address,int tag,int data,int p){
    
    int i,l = 0,c,setnum,count = 0;
    
    cache_block *localblock = NULL,*centralblock = NULL;
    set *homecacheset = NULL,*replcacheset = NULL,*rereplcacheset = NULL;
    int localclustnum,centralclustnum,rerepl = 0;
    
    int row1,row2,col1,col2;
    
    int banknum = bank->banknum;
    
    if(p < cores / 2){
        row1 = 0;
        col1 = p;
    }
    else{
        row1 = 3;
        col1 = p - (cores / 2);
    }
    
    row2 = banknum / 4;    
    col2 = banknum % 4;   
    
    setnum = address % nsets;
    
    homecacheset = home_cacheset(address);
    
    localclustnum = bank->clustnum;
    
    l = lrublock(cgrid1.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
    
    while(l == -1)
        l = lrublock(cgrid1.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
    
    localblock = cgrid1.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
    
    replcacheset = home_cacheset(localblock->address);
    
    if(row1 == 0)
    {
        centralclustnum = cgrid1.grid[row1+1][col1]->cachebank[row2][col2]->clustnum;
        for(i = 0;i < bassoc;i++)
        {   
            centralblock = cgrid1.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]->ways[i];
            if(centralblock->data == 0){                
                c = i;              
                break;
            }
            else
                count++;
        }
        
        if(count == bassoc){
            c = lrublock(cgrid1.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]);
            rerepl = 1;
            (coreprop1[p].nrepl)++;;
            
            // write back the existing central block
        }
        
        centralblock = cgrid1.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]->ways[c];  
        
        if(rerepl){
            rereplcacheset = home_cacheset(centralblock->address);
            (rereplcacheset->hk_nuka[centralclustnum])--;
        }
        
        (replcacheset->hk_nuka[localclustnum])--;
        (replcacheset->hk_nuka[centralclustnum])++;        
        
        block_par_assign(centralblock,localblock->data,localblock->address,localblock->tag,localblock->last_accessed,localblock->naccesses);
        
        (coreprop1[p].nrepl)++;
        
        (homecacheset->hk_nuka[localclustnum])++;
                       
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    
    else if(row1 == 3)
    {
        centralclustnum = cgrid1.grid[row1-1][col1]->cachebank[row2][col2]->clustnum;
        for(i = 0;i < bassoc;i++)
        {
            centralblock = cgrid1.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]->ways[i];
            if(centralblock->data == 0){
                c = i;              
                break;
            }              
            else
                count++;
        }
        
        if(count == bassoc){
            c = lrublock(cgrid1.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]);
            rerepl = 1;
            (coreprop1[p].nrepl)++;
            // write back the existing central block
        }
        
        centralblock = cgrid1.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]->ways[c]; 
        
        if(rerepl){
            rereplcacheset = home_cacheset(centralblock->address);
            (rereplcacheset->hk_nuka[centralclustnum])--;
        }
        
        (replcacheset->hk_nuka[localclustnum])--;
        (replcacheset->hk_nuka[centralclustnum])++;          
        
        block_par_assign(centralblock,localblock->data,localblock->address,localblock->tag,localblock->last_accessed,localblock->naccesses);
        
        (coreprop1[p].nrepl)++;
        
        (homecacheset->hk_nuka[localclustnum])++;
        
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    else
    {
        // write back to main memory
        (replcacheset->hk_nuka[localclustnum])--;
        
        block_reset(localblock);
        
        (coreprop1[p].nrepl)++;
        
        (homecacheset->hk_nuka[localclustnum])++;
        
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    
    return localblock;
    
}



void hkptr_setstate(int *hk_ptr,int new){
    *hk_ptr = *hk_ptr + new + 1;
}

void hkptr_resetstate(int *hk_ptr,int old){
    *hk_ptr = *hk_ptr - old - 1;
}

void hkptr_swapstates(cache_block *cacheblock,int *hk_ptrd,int *hk_ptrs,int new){
    
    *hk_ptrd = *hk_ptrd + new + 1;
    
    if(!(cacheblock->state))       
        (*hk_ptrs)--;
    else
        *hk_ptrs = *hk_ptrs - cacheblock->state - 1;
}


cache_block * cachebank_search2(cache_bank *bank,int address,int tag,int p){
    cache_block *cacheblock = NULL;
    int i,setnum;
    setnum = address % nsets;
    
    for(i = 0 ; i < bassoc ; i++){
        if( (bank->sets[setnum]->ways[i]->tag == tag ) && (bank->sets[setnum]->ways[i]->address == address)){
            
            (bank->sets[setnum]->ways[i]->naccesses) ++;
            bank->sets[setnum]->ways[i]->last_accessed = time(NULL);
            
            //saving way number for migration purpose
            way2[p] = i;
            cacheblock = bank->sets[setnum]->ways[i];
            
            return cacheblock;           
        }
    }
    
    //Cache block not found, MISS
    return cacheblock;
}

cache_block * hkstatenuca_blockassign(cache_bank *bank,int address,int tag,int data,int p,int naccesses,int state){
    cache_bank *homebank = NULL;
    cache_block *cacheblock = NULL;
    set *cacheset = NULL;
    int i,setnum;
    setnum = address % nsets;
    
    for(i = 0 ; i < bassoc ; i++){
        cacheblock = bank->sets[setnum]->ways[i];
        if((cacheblock->data == 0) && (cacheblock->busy == 0)){
            cacheblock->busy = 1;
            
            cacheset = home_cacheset(address);
            homebank = home_cachebank(address);
            
            if(bank != homebank)
                cacheset->hk_nuka[bank->clustnum] = cacheset->hk_nuka[bank->clustnum] + state + 1; 
            
            block_par_assign(cacheblock,data,address,tag,time(NULL),naccesses);  
            
            cacheblock->state = state;
            
            cacheblock->busy = 0;
            
            
            return cacheblock;            
        }
    }
    
    //cache set is full, need replacement    
    return NULL;
}

cache_block * hkstatenuca_replacement(cache_bank *bank,int address,int tag,int data,int p){
    
   int i,l = 0,c,setnum,count = 0;
    
    cache_block *localblock = NULL,*centralblock = NULL;
    set *homecacheset = NULL,*replcacheset = NULL,*rereplcacheset = NULL;
    int localclustnum,centralclustnum,rerepl = 0;
    
    int row1,row2,col1,col2;
    
    int banknum = bank->banknum;
    
    if(p < cores / 2){
        row1 = 0;
        col1 = p;
    }
    else{
        row1 = 3;
        col1 = p - (cores / 2);
    }
    
    row2 = banknum / 4;    
    col2 = banknum % 4;   
    
    setnum = address % nsets;
    
    homecacheset = home_cacheset(address);
    
    localclustnum = bank->clustnum;
    
    l = lrublock(cgrid2.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
    
    while(l == -1)
        l = lrublock(cgrid2.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
    
    localblock = cgrid2.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
    
    replcacheset = home_cacheset(localblock->address);
    
    if(row1 == 0)
    {
        centralclustnum = cgrid2.grid[row1+1][col1]->cachebank[row2][col2]->clustnum;
        for(i = 0;i < bassoc;i++)
        {   
            centralblock = cgrid2.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]->ways[i];
            if(centralblock->data == 0){                
                c = i;              
                break;
            }
            else
                count++;
        }
        
        if(count == bassoc){
            c = lrublock(cgrid2.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]);
            rerepl = 1;
            (coreprop2[p].nrepl)++;;
            
            // write back the existing central block
        }
        
        centralblock = cgrid2.grid[row1+1][col1]->cachebank[row2][col2]->sets[setnum]->ways[c];  
        
        if(rerepl){
            rereplcacheset = home_cacheset(centralblock->address);
            hkptr_resetstate(&(rereplcacheset->hk_nuka[centralclustnum]),centralblock->state);
        }
        
        hkptr_swapstates(localblock,&(replcacheset->hk_nuka[centralclustnum]),&(replcacheset->hk_nuka[localclustnum]),8);        
        
        block_par_assign(centralblock,localblock->data,localblock->address,localblock->tag,localblock->last_accessed,localblock->naccesses);
        
        (coreprop2[p].nrepl)++;
        
        (homecacheset->hk_nuka[localclustnum])++;
                       
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    
    else if(row1 == 3)
    {
        centralclustnum = cgrid2.grid[row1-1][col1]->cachebank[row2][col2]->clustnum;
        for(i = 0;i < bassoc;i++)
        {
            centralblock = cgrid2.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]->ways[i];
            if(centralblock->data == 0){
                c = i;              
                break;
            }              
            else
                count++;
        }
        
        if(count == bassoc){
            c = lrublock(cgrid2.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]);
            rerepl = 1;
            (coreprop2[p].nrepl)++;
            // write back the existing central block
        }
        
        centralblock = cgrid2.grid[row1-1][col1]->cachebank[row2][col2]->sets[setnum]->ways[c]; 
        
        if(rerepl){
            rereplcacheset = home_cacheset(centralblock->address);
            hkptr_resetstate(&(rereplcacheset->hk_nuka[centralclustnum]),centralblock->state);
        }
        
        hkptr_swapstates(localblock,&(replcacheset->hk_nuka[centralclustnum]),&(replcacheset->hk_nuka[localclustnum]),8);            
        
        block_par_assign(centralblock,localblock->data,localblock->address,localblock->tag,localblock->last_accessed,localblock->naccesses);
        
        (coreprop2[p].nrepl)++;
        
        (homecacheset->hk_nuka[localclustnum])++;
        
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
    else
    {
        // write back to main memory
        hkptr_resetstate(&(replcacheset->hk_nuka[localclustnum]),localblock->state);
        
        block_reset(localblock);
        
        (coreprop2[p].nrepl)++;
        
        (homecacheset->hk_nuka[localclustnum])++;
        
        block_par_assign(localblock,data,address,tag,time(NULL),1);
    }
        
    return localblock;
}



