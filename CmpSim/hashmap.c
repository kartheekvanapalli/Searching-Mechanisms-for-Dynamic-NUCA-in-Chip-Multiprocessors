
#include "hashmap.h"

void node_assign(node *current,int address,int tag,node *next){
    current->address = address;
    current->tag = tag;
    current->next = next;
}

void init_hashtable(){
    int i;
    hashtable = (node *)malloc(cores * sizeof(node));
    
    for(i = 0;i < cores; i++){
        node_assign(&hashtable[i],0,0,NULL);
//        hashtable[i].address = 0;
//        hashtable[i].tag = 0;
//        hashtable[i].next = NULL;
    }
}

int hash(int key){
    return (key % cores);
}

// if insert fun returns 0 then another core is accessing same address block.

int insert(int address,int tag){
    
    int key = hash(hash(address) + hash(tag));
    node *curr,*temp;

    if(hashtable[key].address == 0){
        node_assign(&hashtable[key],address,tag,NULL);
        return 1;
    }
    
    curr = &hashtable[key];
    
    while(curr){
        if((curr->address == address) && (curr->tag == tag))
            return 0;
        else{
            temp = curr;
            curr = curr->next;
        }    
    }

    temp->next = (node *)malloc(sizeof(node));
    node_assign(temp->next,address,tag,NULL);
    
    return 1;        
}

void delete(int address,int tag){
    int key = hash(hash(address) + hash(tag));
    node *curr,*temp,*prev;
    
    curr = &hashtable[key];
    
    // deleting head node
    if((curr->address == address) && (curr->tag == tag)){
        temp = curr->next;
        if(temp){
            node_assign(curr,temp->address,temp->tag,temp->next);
            free(temp);
        }
        else
            node_assign(curr,0,0,NULL);
        return;
    }
    
    prev = curr;
    curr = curr->next;
    
    //deleting intermediate or tail node
    while(curr){
        if((curr->address == address) && (curr->tag == tag)){
            temp = curr;
            prev->next = curr->next;
            free(temp);
            break;
        }            
        else{
            prev = curr;
            curr = curr->next;
        }    
    }
}
