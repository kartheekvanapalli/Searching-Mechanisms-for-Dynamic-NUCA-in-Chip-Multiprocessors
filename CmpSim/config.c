
#include "cachebank.h"
#include "config.h"


void ioConfig(){
    
    FILE *input_fd = NULL;
    
    input_fd = fopen("iofiles/input","r");
    
    
    if(input_fd){
        
        while(!feof(input_fd)){            
            fscanf(input_fd,"%d %d %d %d",&cores,
                                          &csize,                /*in MBytes*/
//                                        &bsassoc,              /* 2*cores */ 
                                          &bassoc,
                                          &lsize                 /*in Bytes*/
                    );                        
        }        
        fclose(input_fd);        
    }
}

char *file_name(int p){
    char ch,*s = "iofiles/input",*s1;
    int i = 0,c;
    s1 = (char *)malloc(50 * sizeof(char));
    
    c = strlen(s);
    
    while(i!= c){
        s1[i] = s[i];
        i++;
    }
    if(p < 10){
        ch = p + '0';
        s1[c++] = ch;
    }
    else{
        ch = 1 + '0';
        s1[c++] = ch;
        ch = p - 10 + '0';
        s1[c++] = ch;
    }
    s1[c] = '\0';

    return s1;
}

void inputfiles_create(){
    char *s;
    int i,p,addr,c = 0,count[cores];
    FILE **input;
    FILE *input_fd = NULL;
    
    for(i = 0;i < cores;i++)
        count[i] = 0;
    
    input = (FILE **)malloc(cores * sizeof(FILE *));

    for(i = 0;i < cores;i++){
	s = file_name(i);
	input[i] = fopen(s,"w");
    }
    
    // trace file containing main memory addresses trace8, contains 8 addresses, trace1000 contains 1000 addresses .......
    
    input_fd = fopen("iofiles/cache-body-addr","r");    

    if(input_fd){
        
        while(!feof(input_fd)){            
            fscanf(input_fd,"%d %d",&p,&addr);
            
            if(!addr)
            	return;
            if(count[p]++ < 10000000)
                fprintf(input[p],"%d\n",addr);
            c++;                                    
    
        }        
        
        fclose(input_fd);  
    }
    
    for(i = 0;i < cores;i++){
        fclose(input[i]);
    }
}

cluster* cluster_creat(int clustnum){
    int i,j,banknum;
    cluster *clust;
    clust = (cluster *)malloc(sizeof(cluster));
    
    clust->column = 4;
    clust->row = 2;
    
    clust->cachebank = (cache_bank***) malloc(clust->row * sizeof(cache_bank**));
    
    for(i = 0;i < clust->row;i++)
        clust->cachebank[i] = (cache_bank**) malloc(clust->column * sizeof(cache_bank*));
    
    for( i = 0 ; i < clust->row ; i++ ){
        for( j = 0 ; j < clust->column ; j++ ){
            banknum = i * clust->column + j;
            clust->cachebank[i][j] = cachebank_init(clustnum,banknum);
        }
    }
    
    return clust;
}

void cbankGridConfig(){
    
    int i,j,clustnum;
    cgrid.row = cgrid1.row = cgrid2.row = 4 ;
    cgrid.column = cgrid1.column = cgrid2.column = cores / 2;  
    nclusters = cgrid1.row * cgrid1.column;
    bsassoc = nclusters;                                                                //bsassoc = 2 * cores;
    
    cgrid.grid = (cluster ***)malloc(cgrid1.row * sizeof(cluster **));
    
    cgrid1.grid = (cluster ***)malloc(cgrid1.row * sizeof(cluster **));
    
    cgrid2.grid = (cluster ***)malloc(cgrid2.row * sizeof(cluster **));
    
    for(i = 0;i < cgrid1.row;i++)
        cgrid.grid[i] = (cluster **)malloc(cgrid1.column * sizeof(cluster *));
    
    for(i = 0;i < cgrid1.row;i++)
        cgrid1.grid[i] = (cluster **)malloc(cgrid1.column * sizeof(cluster *));
    
    for(i = 0;i < cgrid2.row;i++)
        cgrid2.grid[i] = (cluster **)malloc(cgrid2.column * sizeof(cluster *));
    
    nbankscluster = 8;
    nbanks = nbankscluster * bsassoc;
    
    nlines = (csize * 1024 * 1024) / lsize ;
    cn = __builtin_popcount(csize * 1024 * 1024 - 1);
    
    nlinesb = nlines / nbanks;
    nsets = nlinesb / bassoc;
    bsize = nlinesb * lsize;
    
    for( i = 0 ; i < cgrid.row ; i++ ){
        for( j = 0 ; j < cgrid.column ; j++ ){
            clustnum = i * cgrid.column + j;
            cgrid.grid[i][j] = cluster_creat(clustnum);
    }
    }
    
    for( i = 0 ; i < cgrid1.row ; i++ ){
        for( j = 0 ; j < cgrid1.column ; j++ ){
            clustnum = i * cgrid1.column + j;
            cgrid1.grid[i][j] = cluster_creat(clustnum);
    }
    }
    
    for( i = 0 ; i < cgrid2.row ; i++ ){
        for( j = 0 ; j < cgrid2.column ; j++ ){
            clustnum = i * cgrid2.column + j;
            cgrid2.grid[i][j] = cluster_creat(clustnum);
    }
    }
    
}
