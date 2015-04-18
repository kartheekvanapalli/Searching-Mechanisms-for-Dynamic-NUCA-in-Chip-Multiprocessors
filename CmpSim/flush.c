
#include "flush.h"

void flush_all(){
    flush_hashtable();
    flush_cache();
    flush_cores();
}

void flush_hashtable(){
    int i;
    node *prev,*curr;
    for(i = 0;i < cores;i++){
        curr = hashtable[i].next;
        while(curr){
            prev = curr;
            curr = curr->next;
            free(prev);
        }
    }
    
    free(hashtable);
}

void flush_cache()
{
    int i,j,k,l,m,n;
    for(i = 0;i < 4;i++){
        for(j = 0;j < cores/2;j++){
            for(k = 0;k < 2;k++){
                for(l = 0;l < 4;l++){
                    for(m = 0;m < nsets;m++){
                        for(n = 0;n < bassoc;n++){
                            free(cgrid.grid[i][j]->cachebank[k][l]->sets[m]->ways[n]);
                            free(cgrid1.grid[i][j]->cachebank[k][l]->sets[m]->ways[n]);
                            free(cgrid2.grid[i][j]->cachebank[k][l]->sets[m]->ways[n]);
                        }
                        free(cgrid.grid[i][j]->cachebank[k][l]->sets[m]->ways);
                        free(cgrid.grid[i][j]->cachebank[k][l]->sets[m]);
                        free(cgrid1.grid[i][j]->cachebank[k][l]->sets[m]->ways);
                        free(cgrid1.grid[i][j]->cachebank[k][l]->sets[m]);
                        free(cgrid2.grid[i][j]->cachebank[k][l]->sets[m]->ways);
                        free(cgrid2.grid[i][j]->cachebank[k][l]->sets[m]);
                    }
                    free(cgrid.grid[i][j]->cachebank[k][l]);
                    free(cgrid1.grid[i][j]->cachebank[k][l]);
                    free(cgrid2.grid[i][j]->cachebank[k][l]);
                }
            }
            free(cgrid.grid[i][j]->cachebank);
            free(cgrid1.grid[i][j]->cachebank);
            free(cgrid2.grid[i][j]->cachebank);
        }
    }
    free(cgrid.grid);
    free(cgrid1.grid);
    free(cgrid2.grid);
}
    
void flush_cores(){
    int i;
    
    free(thread);
//    free(threadmulticast);
    free(coreprop0);
    free(coreprop1);
    free(coreprop2);
    free(way);
    free(way1);
    free(way2);

}
