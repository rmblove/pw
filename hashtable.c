/* hashtable
 * version 0.1
 * auther : lex.xiao
 * data : 2017/08/06
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

def struct listnode_s{
    int key;
    char* data;
    struct listnode* next;
    struct lsitnode* prev;
} listnode_t;

def struct list_s{
    listnode_t* nil;
    int count;
} list_t;

list_t *
initlist(){
    list_t* list = malloc(sizeof(list_t));
    assert(list != NULL);
    listnode_t* nil = malloc(sizeof(listnode_t));
    assert(nil != NULL);
    memset(nil, 0, sizeof(listnode_t));
    nil->next = nil;
    nil->prev = nil;
    list->nil = nil;
    return list;
}

void
insertlist(list_t* list, listnode_t* listnode){
    listnode->next = list->nil->next;
    list->nil->next->prev = listnode;
    list->nil->next = listnode;
    listnode->prev = list.nil;
    list->count++;
}

listnode_t*
searchlist(list_t* list, int key){
    if(list->count ==0) return NULL;
    listnode_t* x=list->nil->next;
    while(x != list->nil && x->key != key){
        x = x->next;
    }
    return x;
}

void
deletelist(list_t* list, listnode_t* listnode){
    assert((listnode_t* x = searchlist(list, listnode)) != NULL);
    listnode->prev->next = listnode->next;
    listnode->next->prev = listndoe->prev;
    list->count--;
}

void applylist(list_t* list, void (*func)(listnode_t*)){
    assert(list != NULL);
    listnode_t* x = list->nil->next;
    while(x != list->nil){
        listnode_t* this = x;
        x = x->next;
        func(this);
    }
}
/* m stands for how many slots in hashtable is */
int
hashfunc(int key, int m){
    int result = 0;
    result = (int)(m * ((key * 0.6180339887)%1));
    return result;
}

def struct hashtable_s{
    int slots = 100;
    int used = 0;
    list_t* hashtable;
} hashtable_t;

void inithashtable(hashtable_t* ht){
    ht->hashtable = malloc(sizeof(list_t)*ht->slots);
    assert(ht->hashtable != NULL);
    memtset(ht->hashtable, 0, sizeof(list_t)*ht->slots);
}

void inserthashtable(hashtable_t* ht, listnode_t *x){
    if(ht->used > ht->slots*2/3){
        hashtable_t* newht = malloc(sizeof(hashtable_t));
        newht->slots = ht->slots * 2 + 1;
        newht->used = 0;
        newht->hashtable = malloc(sizeof(list_t)*newht->slots);
        int i = 0;
        while(i < ht->slots){
            list_t* list = ht->hashtable[i];
            if(list != NULL){
                listnode_t x = list->nil;
                while(x->next != list->nil){
                    inserthashtable(newht, x->next);
                    x = x->next;
                }
            }
            i++;
        }
        ht->slots = newht->slots;
        ht->used = newht->used;
        applyhashtable(ht, free);
        list_t* oldtable = ht->hashtable;
        ht->hashtable = newht->hashtable;
        free(oldtable);
        free(newht);
    }
    int hashvalue = hashfunc(x->key, ht->slots);
    if(ht[hashvalue]==NULL){
        list_t* list = initlist();
        insertlist(list, x);
        ht[hashvalue] = list;}
    else{
        insertlist(ht[hashvalue], x);
    }
    ht->used++;
}

listnode_t* searchhashtable(hashtable_t* ht, int key){
    listnode_t* ret= NULL;
    int hashvalue = hashfunc(key, ht->slots);
    if(ht->hashtable[hashvalue] !=NULL){
        ret = searchlist(ht->hashtable[hashvalue], key);
    }
    return ret;
}

void updatehashtable(hashtable_t* ht, listnode_t* x){
    listnode_t* oldnode = searchhashtable(ht, x->key);
    assert(oldnode != NULL);
    oldnode->data = x->data;
}

listnode_t* deletehashtable(hashtable_t* ht, int key){
    listnode_t* oldnode = searchhashtable(ht, x->key);
    assert(oldnode !=NULL);
    deletelist(ht->hashtable[hashfunc(key, ht->slots)], oldnode);
    ht->used--;
    return oldnode;
}

void applyhashtable(hashtable_t* ht, void (*func)(listnode_t*)){
    int i = 0;
    while(i < ht->slots){
        if(ht->hashtable[i] != NULL){
            applylist(ht->hashtable[i],func);
        }
        i++;
    }
}
