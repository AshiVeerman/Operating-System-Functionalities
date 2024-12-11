#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
// Include your Headers below


// You are not allowed to use the function malloc and calloc directly .
typedef struct node_t{
    struct node_t* prev;
    struct node_t* next;
    int free_space;
    int msize;
}node;

node* n1=NULL;
//Function to allocate memory using mmap
void* my_malloc(size_t size) {
    // Your implementation of my_malloc goes here
    if(n1==NULL){
        n1=(node*)mmap(n1,65536,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,-1,0);
        n1->prev=NULL;
        n1->next=NULL;
        n1->free_space=0;
        n1->msize=65536-sizeof(node);
    }
    node* p=n1;
    while (p !=NULL){
        if (p->free_space==0 && p->msize>=size){
            p->free_space=1;
            if (p->msize>size+sizeof(node)){
                node* n2 = (node*)((char*)p + sizeof(node)+ size);
                n2->msize=p->msize-sizeof(node)-size;
                n2->free_space=0;
                n2->next=p->next;
                n2->prev=p;
                if(p->next){
                    p->next->prev=n2;
                }
                p->next=n2;
                p->msize=size;
            }
            return (void*)(p+1);
        }
        p=p->next;
    }
    fprintf(stderr,"No new space found\n");
    return NULL;
}

// Function to allocate and initialize memory to zero using mmap
void* my_calloc(size_t nelem, size_t size) {
    // Your implementation of my_calloc goes here
    if(size==0)return NULL;
    if(size<0)return NULL;
    size_t  total = nelem*size;
 
   void *ptr = my_malloc(total);
   
   if(ptr==NULL){
    
    perror("There is some error in calloc_implementation\n");
    exit(1);
   }
   else if(ptr!=NULL){
    
    char* p1 = (char*)ptr;
    for (int i = 0; i < total; ++i) {
        p1[i] = 0;
    }
   }
   else{
       return ptr;
   }
   return ptr;
}

// Function to release memory allocated using my_malloc and my_calloc
void my_free(void* ptr) {
    // Your implementation of my_free goes here
    if (ptr ==NULL) return;
    node *p = (node*)((char*)ptr - sizeof(node));
    if(p==NULL)
    {
        fprintf(stderr,"NULL POINTER\n");
        return;
    }
    p->free_space=0;
    if(p->prev!=NULL && p->prev->free_space==0){
        p->prev->msize = p->prev->msize + sizeof(node) + p->msize;
        p->prev->next=p->next;
        if(p->next!=NULL)
        {
            p->next->prev=p->prev;
        }
        p=p->prev;
    }
    if(p->next!=NULL && p->next->free_space==0){
        p->msize=p->msize+sizeof(node)+p->next->msize;
        p->next = p->next->next;
        if(p->next){
            p->next->prev = p;
        }
    }
}


