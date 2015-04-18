
#include "hashmap.h"

void node_assign(node *current,int address,int tag,node *next){
    current->address = address;
    current->tag = tag;
    current->next = next;
}

void init_hashtable(){
    int i;
    hashtable = (node **)malloc(cores * sizeof(node *));
    
    for(i = 0;i < cores; i++){
        hashtable[i] = NULL;
    }
//    node_assign(&hashtable[i],0,0,NULL);
//        hashtable[i].busy = 0;
//        hashtable[i].address = 0;
//        hashtable[i].tag = 0;
//        hashtable[i].next = NULL;
//    }
}

int hash(int key){
    return (key % cores);
}

// if insert fun returns 0 then another core is accessing same address block.

int insert(int address,int tag){
    
    int key = hash(hash(address) + hash(tag));
    node *head,*next;
    head = hashtable[key];
    
    pthread_mutex_lock(&mutex);
    if(!head){
        head = (node *)malloc(sizeof(node));
        node_assign(head,address,tag,NULL);
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    pthread_mutex_unlock(&mutex);
    
    
    next = head->next;
    
    while(head->busy || next->busy);
    
    while(curr){
        if((curr->address == address) && (curr->tag == tag))
            return 0;
        else{
            while(curr->busy);
            temp = curr;
            curr = curr->next;
        }    
    }
    
    while(temp->busy);
    temp->busy = 1;
    while(temp){
        if(!temp->next){
            
            temp->next = (node *)malloc(sizeof(node));
            node_assign(temp->next,address,tag,NULL);
            temp->busy = 0;
            break;
        }
        else if((temp->next->address == address) && (temp->next->tag == tag))
            return 0;
        else
            temp = temp->next;
    } 
    
    return 1;        
}

void delete(int address,int tag){
    int key = hash(hash(address) + hash(tag));
    node *curr,*temp,*next,*prev;
    
    curr = &hashtable[key];
    
    // deleting head node
    if((curr->address == address) && (curr->tag == tag)){
        curr->busy = 1;
        
        temp = curr->next;
        if(temp)
            temp->busy = 1;        
        
        if(temp){
            node_assign(curr,temp->address,temp->tag,temp->next);
            free(temp);
        }
        else
            node_assign(curr,0,0,NULL);
        
        curr->busy = 0;
        return;
    }
    
    prev = curr;
    curr = curr->next;
    
    //deleting intermediate or tail node
    while(curr){
        if((curr->address == address) && (curr->tag == tag)){
            prev->busy = 1;
            curr->busy = 1;
            next = curr->next;
            if(next)
                next->busy = 1;
            
            temp = curr;
            prev->next = curr->next;
            free(temp);
            
            prev->busy = 0;
            if(next)
                next->busy = 0;
            
            break;
        }            
        else{
            prev = curr;
            curr = curr->next;
        }    
    }
}

