
#include "coreconfig.h"


void init_coreprop(){
    int i;
    
    for(i = 0 ; i < cores ; i++){
        way[i] = INT_MIN;
        way1[i] = INT_MIN;
        way2[i] = INT_MIN;
        
        skipsearches[i]= 0;
        
        coreprop0[i].naccesses = 0;
        coreprop0[i].nhits = 0;
        coreprop0[i].misses = 0;
        coreprop0[i].missesperbank = 0;
        coreprop0[i].nalloc = 0;
        coreprop0[i].nrepl = 0;
        coreprop0[i].blockmovements = 0;
        
        coreprop1[i].naccesses = 0;
        coreprop1[i].nhits = 0;
        coreprop1[i].misses = 0;
        coreprop1[i].missesperbank = 0;
        coreprop1[i].nalloc = 0;
        coreprop1[i].nrepl = 0;
        coreprop1[i].blockmovements = 0;
        
        coreprop2[i].naccesses = 0;
        coreprop2[i].nhits = 0;
        coreprop2[i].misses = 0;
        coreprop2[i].missesperbank = 0;
        coreprop2[i].nalloc = 0;
        coreprop2[i].nrepl = 0;
        coreprop2[i].blockmovements = 0;
    }
}

void coresConfig(){
    
    int i;
    
    thread = (pthread_t *)malloc(cores * sizeof(pthread_t));  
    
    coreprop0 = (corethread *)malloc(cores * sizeof(corethread));
    coreprop1 = (corethread *)malloc(cores * sizeof(corethread));
    coreprop2 = (corethread *)malloc(cores * sizeof(corethread));
    
    way = (int *)malloc(cores * sizeof(int));
    way1 = (int *)malloc(cores * sizeof(int));
    way2 = (int *)malloc(cores * sizeof(int));
    
    skipsearches = (int *)malloc(cores * sizeof(int));
    
    init_coreprop();
    
    for(i = 0 ; i < cores ; i++)
	 {
            pthread_create(&thread[i],NULL,dataproc,(void *)&i);
	 }
    
    for(i = 0 ; i < cores ; i++)
	 {
	       pthread_join(thread[i], NULL);
	 }
}

int msbposition(int address){
    int msb = 0;
    while(address){
        address = address >> 1;
        msb++;
    }
    return msb;
}

void *dataproc(void *proc){

    int i = 0,maddress = 0,caddress,tag = 0;
    int p = *(int *)proc;
    FILE *input_fd = NULL;
    char *s = file_name(p);
    cache_block *cacheblock;

    input_fd = fopen(s,"r");

    if(input_fd){
        
        while(!feof(input_fd)){            
            fscanf(input_fd,"%d",&maddress);
            (coreprop1[p].naccesses)++;
            
            //converting 32 bit(4GB RAM) address into 10 bit tag and 22 bit(4MB CACHE) address 
            
            if(!maddress)
                return;
                    
            if(msbposition(maddress) >= cn){
                tag = maddress >> cn ;
                caddress = (maddress & ~((~0) << cn));
            }
            else{
                caddress = maddress;
            }
            
            //busy waiting
            while(!i){
                pthread_mutex_lock(&hashmutex);
                i = insert(caddress,tag);
                pthread_mutex_unlock(&hashmutex);
            }
            
            //linear searching
            pthread_mutex_lock(&mutex);
            cacheblock = core_search(p,caddress,tag);
            pthread_mutex_unlock(&mutex);

            //HK-NUCA searching
            pthread_mutex_lock(&mutex1);
            cacheblock = hknuca_coresearch(p,caddress,tag);
            pthread_mutex_unlock(&mutex1);
            
            //HKState-NUCA modified searching
            pthread_mutex_lock(&mutex2);
            cacheblock = hkstatenuca_coresearch(p,caddress,tag);
            pthread_mutex_unlock(&mutex2);

            //data processing, we should put some delay
            for (i=1 ; i<100 ; i++) ;
            
            pthread_mutex_lock(&hashmutex);
            delete(caddress,tag);
            pthread_mutex_unlock(&hashmutex);
        }        
    fclose(input_fd);        
    }
}


void block_reset(cache_block *cacheblock){

    cacheblock->address = 0;
    cacheblock->tag = 0;
    cacheblock->data = 0;
    cacheblock->naccesses = 0;    
    cacheblock->last_accessed = time(NULL);
}

cache_block *cacheblock_swap(cache_block *scacheblock,cache_block *dcacheblock){
    cache_block cacheblock;    
    
    cacheblock = *dcacheblock;
    *dcacheblock = *scacheblock;
    *scacheblock = cacheblock;    
    
    return dcacheblock;
}


cache_block* core_search(int p,int address,int tag){
    
    cache_block* cacheblock = NULL;
    
    int row1,row2,col1,col2;
    
    int banknum = (address / nsets) % nbankscluster;
    
    /*Local bank search*/
    
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
    
    //local bank search
    cacheblock = cachebank_search(cgrid.grid[row1][col1]->cachebank[row2][col2],address,tag,p);
           
    if(!cacheblock)
        (coreprop0[p].missesperbank)++;
    
    /*cache block not found in local bank*/
    if(!cacheblock)
        cacheblock = linear_search(row1,col1,row2,col2,address,tag,p);
    
    if(cacheblock)
        (coreprop0[p].nhits)++;                                  //Cache block found, HIT
    
    else{    
        /*cache block not found in all banks, bringing block from Main memory and assigning it to local bank*/
        if(!cacheblock){
            (coreprop0[p].misses)++;
            cacheblock = cacheblock_assign(cgrid.grid[row1][col1]->cachebank[row2][col2],address,tag,1,p,1);
            if(cacheblock)
                (coreprop0[p].nalloc)++;
        }

        /*local cache bank is full, replacement needed*/
        if(!cacheblock){
            pthread_mutex_lock(&replmutex);
            cacheblock = cacheblock_replacement(cgrid.grid[row1][col1]->cachebank[row2][col2],address,tag,1,p);
            pthread_mutex_unlock(&replmutex);
        }
    }
    return cacheblock;        
}

cache_block* linear_search(int row1,int col1,int row2,int col2,int address,int tag,int p){
    int i,j,r,c;
    cache_block *cacheblock = NULL,*cacheblockmoved = NULL;
    
    for(i = 0; i < 4 ; i++){
        for(j = 0; j < cores/2 ; j++){
            if(!((i == row1) && (j == col1))){
                cacheblock = cachebank_search(cgrid.grid[i][j]->cachebank[row2][col2],address,tag,p);    
                r = i;
                c = j;
                
                if(!cacheblock)
                    (coreprop0[p].missesperbank)++;
            }           
        }
    }
    
    /*if cache block found, movement needed */
    if(cacheblock)
        cacheblockmoved = migration_swapping(r,c,row1,col1,row2,col2,address,tag,p); 
        
    return cacheblockmoved;
}

cache_block* migration(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p){
    
    cache_block *cacheblock = NULL;
    int l,setnum,data,naccesses;
    
    setnum = address % nsets;
    
    data = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]->data;
    naccesses = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]->naccesses;
    
    (coreprop0[p].blockmovements)++;
    
    /*Cache block found in its own central bank moving it to its local bank*/
    
    if(((row1 == r-1) || (row1 == r+1)) && (c == col1))        
        cacheblock = cacheblock_assign(cgrid.grid[row1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
        
    /*Cache block found in one of the others local bank moving it to theirs central bank*/
    else if(r == 0)
        cacheblock = cacheblock_assign(cgrid.grid[1][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
    
    else if(r == 3)
        cacheblock = cacheblock_assign(cgrid.grid[2][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
    
    /*Cache block found in one of the central bank moving it to requested cores central bank*/
    else{
        if(row1 == 0)
            cacheblock = cacheblock_assign(cgrid.grid[1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
        else
            cacheblock = cacheblock_assign(cgrid.grid[2][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
    }
        
    return cacheblock;
}

cache_block* migration_swapping(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p){
    
    cache_block *cacheblock = NULL,*scacheblock = NULL,*dcacheblock = NULL;
    int l,setnum,data,naccesses;
    
    setnum = address % nsets;
    
    data = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]->data;
    naccesses = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]->naccesses;
    (coreprop0[p].blockmovements)++;
    
    /*Cache block found in its own central bank moving it to its local bank*/
    
    if(((row1 == r-1) || (row1 == r+1)) && (c == col1)){
        
        cacheblock = cacheblock_assign(cgrid.grid[row1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
        
        if(!cacheblock){
            l = lrublock(cgrid.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]];
            dcacheblock = cgrid.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);
        }
        else
            block_reset(cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]);
    }
    /*Cache block found in one of the others local bank moving it to theirs central bank*/
    else if(r == 0){
        cacheblock = cacheblock_assign(cgrid.grid[1][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
        
        if(!cacheblock){
            l = lrublock(cgrid.grid[1][c]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]];
            dcacheblock = cgrid.grid[1][c]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);
        }
        else
            block_reset(cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]);
    }
    
    else if(r == 3){
        cacheblock = cacheblock_assign(cgrid.grid[2][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
        
        if(!cacheblock){
            l = lrublock(cgrid.grid[2][c]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]];
            dcacheblock = cgrid.grid[2][c]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);
        }
        else
            block_reset(cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]);
    }
    
    /*Cache block found in one of the central bank moving it to requested cores central bank*/
    else{
        if(row1 == 0){
            cacheblock = cacheblock_assign(cgrid.grid[1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
            
            if(!cacheblock){
                l = lrublock(cgrid.grid[1][col1]->cachebank[row2][col2]->sets[setnum]);
                scacheblock = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]];
                dcacheblock = cgrid.grid[1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
                cacheblock = cacheblock_swap(scacheblock,dcacheblock);
            }
            else
                block_reset(cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]);
        }
        else{
            cacheblock = cacheblock_assign(cgrid.grid[2][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
            
            if(!cacheblock){
                l = lrublock(cgrid.grid[2][col1]->cachebank[row2][col2]->sets[setnum]);
                scacheblock = cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]];
                dcacheblock = cgrid.grid[2][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
                cacheblock = cacheblock_swap(scacheblock,dcacheblock);
            }
            else
                block_reset(cgrid.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way[p]]);
        }
    }
        
    return cacheblock;
}


void *multicastthread(void *p){
    (*(param *)p).cblock = cachebank_search((*(param *)p).cbank,(*(param *)p).address,(*(param *)p).tag,(*(param *)p).p);
    if((*(param *)p).cblock)
        savepar = *(param *)p;
}

cache_block* multicast_search(int row1,int col1,int row2,int col2,int address,int tag,int p){
    int i,j,r,c;
    cache_block *cacheblock,*cacheblockmoved;
    param par[2 * cores];
    threadmulticast = (pthread_t *)malloc(nclusters * sizeof(pthread_t));
    
    for(i = 0; i < 4 ; i++){
        for(j = 0; j < cores/2 ; j++){
                par[i+j].address = address;
                par[i+j].tag = tag;
                par[i+j].cbank = cgrid.grid[i][j]->cachebank[row2][col2];
                par[i+j].r = i;
                par[i+j].c = j;
                par[i+j].p = p;
                pthread_create(&threadmulticast[i+j],NULL,multicastthread,(void *)&par[i+j]);              
                
        }
    }
    
    for(i = 0; i < 4 ; i++){
        for(j = 0; j < cores/2 ; j++){
            pthread_join(threadmulticast[i+j], NULL);
        }
    }
    
    cacheblock = savepar.cblock;
    /*if cache block found, movement needed */
    if(cacheblock){        
        cacheblockmoved = migration_swapping(savepar.r,savepar.c,row1,col1,row2,col2,address,tag,p);
        block_reset(cacheblock);
    }
}


cache_block* hknuca_coresearch(int p,int address,int tag){
    
    cache_block *cacheblock = NULL;
    cache_bank *homebank = NULL;
    
    int row1,row2,col1,col2;
    
    int banknum = (address / nsets) % nbankscluster;
    
    /*Local bank search*/
    
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
    
    //local bank search
    cacheblock = cachebank_search1(cgrid1.grid[row1][col1]->cachebank[row2][col2],address,tag,p);
           
    if(!cacheblock)
        (coreprop1[p].missesperbank)++;
    
    /*cache block not found in local bank*/
    if(!cacheblock)
        cacheblock = hknuca_search(row1,col1,row2,col2,address,tag,p);
    
        
    if(cacheblock)
        (coreprop1[p].nhits)++;                                  //Cache block found, HIT
    
    else{
        homebank = home_cachebank(address);
        
        /*cache block not found in all banks, bringing block from Main memory and assigning it to local bank*/
        if(!cacheblock){
            (coreprop1[p].misses)++;
            cacheblock = hknuca_blockassign(homebank,address,tag,1,p,1);
            if(cacheblock)
                (coreprop1[p].nalloc)++;
        }
        
        /*local cache bank is full, replacement needed*/
        if(!cacheblock){
            pthread_mutex_lock(&replmutex1);
            cacheblock = hknuca_replacement(homebank,address,tag,1,p);
            pthread_mutex_unlock(&replmutex1);
        }
    }
    
    return cacheblock;        
}

cache_block* hknuca_search(int row1,int col1,int row2,int col2,int address,int tag,int p){
    int i,hr,hc,hclustnum,tr,tc,hit = 0;
    cache_bank *homebank = NULL;
    set *cacheset = NULL;
    cache_block *cacheblock = NULL,*tempcacheblock = NULL,*cacheblockmoved = NULL;
    
    homebank = home_cachebank(address);
    
    //searching home bank
    cacheblock = cachebank_search1(homebank,address,tag,p);
    
    if(!cacheblock){
        (coreprop1[p].missesperbank)++;
        cacheset = home_cacheset(address);    
        for(i = 0; i < bsassoc ; i++){
            tr = i / (cores / 2);
            tc = i % (cores / 2);
            if((cacheset->hk_nuka[i]) && !((tr == row1) && (tc == col1))){               
                    cacheblock = cachebank_search1(cgrid1.grid[tr][tc]->cachebank[row2][col2],address,tag,p);    
                    hr = tr;
                    hc = tc;
                    if(!cacheblock)
                        (coreprop1[p].missesperbank)++;
                    else{
                        hit = 1;
                        tempcacheblock = cacheblock;
                    } 
                        
            }
        }
    }
    else{
        hclustnum = (address / (nbankscluster * nsets)) % bsassoc;
        hr = hclustnum / (cores / 2);
        hc = hclustnum % (cores / 2);
    }
    
    if(hit)
        cacheblock = tempcacheblock;
    
    /*if cache block found, movement needed */
    if(cacheblock)
        cacheblockmoved = hknuca_migration(hr,hc,row1,col1,row2,col2,address,tag,p);  
        
    return cacheblockmoved;
}

cache_block* hknuca_migration(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p){
    
    cache_block *cacheblock = NULL,*scacheblock = NULL,*dcacheblock = NULL;
    set *homecacheset = NULL,*swappedcacheset = NULL;
    int l,dclustnum,sclustnum,setnum,data,naccesses,swapaddress,swap = 0;
    
    setnum = address % nsets;
    
    data = cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]]->data;
    naccesses = cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]]->naccesses;
        
    homecacheset = home_cacheset(address);
    
    (coreprop1[p].blockmovements)++;
    
    sclustnum = cgrid1.grid[r][c]->cachebank[row2][col2]->clustnum;
        
    /*Cache block found in its own central bank moving it to its local bank*/
    
    if(((row1 == r-1) || (row1 == r+1)) && (c == col1)){
        cacheblock = hknuca_blockassign(cgrid1.grid[row1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);  
        dclustnum = cgrid1.grid[row1][col1]->cachebank[row2][col2]->clustnum;
        
        if(!cacheblock){
            l = lrublock(cgrid1.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]];
            dcacheblock = cgrid1.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            swap = 1;
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);            
        }
        else
            block_reset(cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]]);
    }
    /*Cache block found in one of the others local bank moving it to theirs central bank*/
    else if(r == 0){
        cacheblock = hknuca_blockassign(cgrid1.grid[1][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
        dclustnum = cgrid1.grid[1][c]->cachebank[row2][col2]->clustnum;
        
        if(!cacheblock){
            l = lrublock(cgrid1.grid[1][c]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]];
            dcacheblock = cgrid1.grid[1][c]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            swap = 1;
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);
        }
        else
            block_reset(cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]]);
    }
    
    else if(r == 3){
        cacheblock = hknuca_blockassign(cgrid1.grid[2][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
        dclustnum = cgrid1.grid[2][c]->cachebank[row2][col2]->clustnum;
        
        if(!cacheblock){
            l = lrublock(cgrid1.grid[2][c]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]];
            dcacheblock = cgrid1.grid[2][c]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            swap = 1;
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);
        }
        else
            block_reset(cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]]);
    }
    /*Cache block found in one of the central bank moving it to requested cores central bank*/
    else{
        if(row1 == 0){
            cacheblock = hknuca_blockassign(cgrid1.grid[1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
            dclustnum = cgrid1.grid[1][col1]->cachebank[row2][col2]->clustnum;
            
            if(!cacheblock){
                l = lrublock(cgrid1.grid[1][col1]->cachebank[row2][col2]->sets[setnum]);
                scacheblock = cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]];
                dcacheblock = cgrid1.grid[1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
                
                swap = 1;
                cacheblock = cacheblock_swap(scacheblock,dcacheblock);
            }
            else
                block_reset(cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]]);
        }
        else{
            cacheblock = hknuca_blockassign(cgrid1.grid[2][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
            dclustnum = cgrid1.grid[2][col1]->cachebank[row2][col2]->clustnum;
            
            if(!cacheblock){
                l = lrublock(cgrid1.grid[2][col1]->cachebank[row2][col2]->sets[setnum]);
                scacheblock = cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]];
                dcacheblock = cgrid1.grid[2][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
                
                swap = 1;
                cacheblock = cacheblock_swap(scacheblock,dcacheblock);
            }
            else
                block_reset(cgrid1.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way1[p]]);
        }
    }
    
    (homecacheset->hk_nuka[dclustnum])++;
    (homecacheset->hk_nuka[sclustnum])--;
    
    if(swap){
        swapaddress = scacheblock->address;
        swappedcacheset = home_cacheset(swapaddress);
        (swappedcacheset->hk_nuka[dclustnum])--;
        (swappedcacheset->hk_nuka[sclustnum])++; 
    }
    return cacheblock;
}



cache_block* hkstatenuca_coresearch(int p,int address,int tag){
    
    cache_block *cacheblock = NULL;
    
    int row1,row2,col1,col2;
    
    int banknum = (address / nsets) % nbankscluster;
    
    /*Local bank search*/
    
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
    
    //local bank search
    cacheblock = cachebank_search2(cgrid2.grid[row1][col1]->cachebank[row2][col2],address,tag,p);
           
    if(!cacheblock)
        (coreprop2[p].missesperbank)++;
    
    /*cache block not found in local bank*/
    if(!cacheblock)
        cacheblock = hkstatenuca_search(row1,col1,row2,col2,address,tag,p);
    
    
    if(cacheblock)
        (coreprop2[p].nhits)++;                                  //Cache block found, HIT
    
    else{  
        
        /*cache block not found in all banks, bringing block from Main memory and assigning it to local bank, state - None (0)*/
        if(!cacheblock){
            (coreprop2[p].misses)++;
            cacheblock = hkstatenuca_blockassign(cgrid2.grid[row1][col1]->cachebank[row2][col2],address,tag,1,p,1,0);
            if(cacheblock)
                (coreprop2[p].nalloc)++;
        }

        /*local cache bank is full, replacement needed*/
        if(!cacheblock){
            pthread_mutex_lock(&replmutex2);
            cacheblock = hkstatenuca_replacement(cgrid2.grid[row1][col1]->cachebank[row2][col2],address,tag,1,p);
            pthread_mutex_unlock(&replmutex2);
        }
    }
    return cacheblock;        
}

cache_block* hkstatenuca_search(int row1,int col1,int row2,int col2,int address,int tag,int p){
    int i,r,c,tr,tc,hclustnum,hit = 0;
    cache_bank *homebank = NULL;
    set *cacheset = NULL;
    cache_block *cacheblock = NULL,*tempcacheblock = NULL,*cacheblockmoved = NULL;
    
    homebank = home_cachebank(address);
    
    //searching home bank
    cacheblock = cachebank_search2(homebank,address,tag,p);
    
    cacheset = home_cacheset(address);
    
    // Searching promoted and demoted states  (Multicast Search)
    
    if(!cacheblock){            
        for(i = 0; i < bsassoc ; i++){
            tr = i / (cores / 2);
            tc = i % (cores / 2);
            if((cacheset->hk_nuka[i] > 8) && !((tr == row1) && (tc == col1))){                
                cacheblock = cachebank_search2(cgrid2.grid[tr][tc]->cachebank[row2][col2],address,tag,p);    
                r = tr;
                c = tc;
                if(!cacheblock)
                    (coreprop2[p].missesperbank)++;          
                else{
                    hit = 1;
                    tempcacheblock = cacheblock;
                } 
            }
            else if((cacheset->hk_nuka[i] <= 8) && (cacheset->hk_nuka[i] > 0))
                skipsearches[p]++;
        }
    }
    else{
        hclustnum = (address / (nbankscluster * nsets)) % bsassoc;
        r = hclustnum / (cores / 2);
        c = hclustnum % (cores / 2);
    }
    
    if(hit)
        cacheblock = tempcacheblock;
    
    // if cache block not found in promoted or demoted states searching none state
    
    if(!cacheblock){   
        for(i = 0; i < bsassoc ; i++){
            tr = i / (cores / 2);
            tc = i % (cores / 2);
            if((cacheset->hk_nuka[i] <= 8) && !((tr == row1) && (tc == col1))){
                cacheblock = cachebank_search2(cgrid2.grid[tr][tc]->cachebank[row2][col2],address,tag,p);    
                r = tr;
                c = tc;
                if(!cacheblock)
                    (coreprop2[p].missesperbank)++;          
                else{
                    hit = 1;
                    tempcacheblock = cacheblock;
                }  
            }
        }
    }
    
    
    if(hit)
        cacheblock = tempcacheblock;
    
    /*if cache block found, movement needed */
    if(cacheblock)
        cacheblockmoved = hkstatenuca_migration(r,c,row1,col1,row2,col2,address,tag,p);  
        
    return cacheblockmoved;
}

cache_block* hkstatenuca_migration(int r,int c,int row1,int col1,int row2,int col2,int address,int tag,int p){
    
    cache_block *cacheblock = NULL,*scacheblock = NULL,*dcacheblock = NULL;
    set *homecacheset = NULL,*swappedcacheset = NULL;
    int l,dclustnum,sclustnum,setnum,data,naccesses,swapaddress,swap = 0,new,swapnew;
    
    setnum = address % nsets;
    
    data = cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]]->data;
    naccesses = cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]]->naccesses;
        
    homecacheset = home_cacheset(address);
    
    (coreprop2[p].blockmovements)++;
    
    sclustnum = cgrid2.grid[r][c]->cachebank[row2][col2]->clustnum;
        
    /*Cache block found in its own central bank moving it to its local bank*/
    
    if(((row1 == r-1) || (row1 == r+1)) && (c == col1)){
        cacheblock = hknuca_blockassign(cgrid2.grid[row1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);  
        dclustnum = cgrid2.grid[row1][col1]->cachebank[row2][col2]->clustnum;
        new = 64;
        
        if(!cacheblock){
            l = lrublock(cgrid2.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]];
            dcacheblock = cgrid2.grid[row1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            swap = 1;
            swapnew = 8;
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);            
        }                           
    }
    /*Cache block found in one of the others local bank moving it to theirs central bank*/
    else if(r == 0){
        cacheblock = hknuca_blockassign(cgrid2.grid[1][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
        dclustnum = cgrid2.grid[1][c]->cachebank[row2][col2]->clustnum;
        new = 8;
        
        if(!cacheblock){
            l = lrublock(cgrid2.grid[1][c]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]];
            dcacheblock = cgrid2.grid[1][c]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            swap = 1;
            swapnew = 64;
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);
        }
    }
    
    else if(r == 3){
        cacheblock = hknuca_blockassign(cgrid2.grid[2][c]->cachebank[row2][col2],address,tag,data,p,naccesses);
        dclustnum = cgrid2.grid[2][c]->cachebank[row2][col2]->clustnum;
        new = 8;
        
        if(!cacheblock){
            l = lrublock(cgrid2.grid[2][c]->cachebank[row2][col2]->sets[setnum]);
            scacheblock = cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]];
            dcacheblock = cgrid2.grid[2][c]->cachebank[row2][col2]->sets[setnum]->ways[l];
            
            swap = 1;
            swapnew = 64;
            cacheblock = cacheblock_swap(scacheblock,dcacheblock);
        }
    }
    
    /*Cache block found in one of the central bank moving it to requested cores central bank*/
    else{
        if(row1 == 0){
            cacheblock = hknuca_blockassign(cgrid2.grid[1][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
            dclustnum = cgrid2.grid[1][col1]->cachebank[row2][col2]->clustnum;
            new = 64;
            
            if(!cacheblock){
                l = lrublock(cgrid2.grid[1][col1]->cachebank[row2][col2]->sets[setnum]);
                scacheblock = cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]];
                dcacheblock = cgrid2.grid[1][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
                
                swap = 1;
                swapnew = 8;
                cacheblock = cacheblock_swap(scacheblock,dcacheblock);
            }
        }
        else{
            cacheblock = hknuca_blockassign(cgrid2.grid[2][col1]->cachebank[row2][col2],address,tag,data,p,naccesses);
            dclustnum = cgrid2.grid[2][col1]->cachebank[row2][col2]->clustnum;
            new = 64;
            
            if(!cacheblock){
                l = lrublock(cgrid2.grid[2][col1]->cachebank[row2][col2]->sets[setnum]);
                scacheblock = cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]];
                dcacheblock = cgrid2.grid[2][col1]->cachebank[row2][col2]->sets[setnum]->ways[l];
                
                swap = 1;
                swapnew = 8;
                cacheblock = cacheblock_swap(scacheblock,dcacheblock);
            }
        }
    }
 
    
    if(swap){
        hkptr_swapstates(cacheblock,&(homecacheset->hk_nuka[dclustnum]),&(homecacheset->hk_nuka[sclustnum]),new);
        
        swapaddress = scacheblock->address;
        swappedcacheset = home_cacheset(swapaddress);
        hkptr_swapstates(scacheblock,&(swappedcacheset->hk_nuka[sclustnum]),&(swappedcacheset->hk_nuka[dclustnum]),swapnew); 
        scacheblock->state = swapnew;
    }
    else{
        hkptr_swapstates(cacheblock,&(homecacheset->hk_nuka[dclustnum]),&(homecacheset->hk_nuka[sclustnum]),new);
        block_reset(cgrid2.grid[r][c]->cachebank[row2][col2]->sets[setnum]->ways[way2[p]]);
    }
    cacheblock->state = new;    
    
    return cacheblock;
}
