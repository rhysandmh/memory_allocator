#include "mem.h"
#define DEBUG 0                   
extern BLOCK_HEADER* first_header;

BLOCK_HEADER* Get_Next_Header(BLOCK_HEADER* current){
    int block_size = current->size_alloc & 0xFFFFFFFE;
    return (BLOCK_HEADER *)((unsigned long)current + block_size);
}

int Is_Free(BLOCK_HEADER* current){
    return !(current->size_alloc & 1);
}

// return a pointer to the payload
// if a large enough free block isn't available, return NULL
void* Mem_Alloc(int size){
    
    // find a free block that's big enough
    BLOCK_HEADER* current = first_header;
    while(1){
        //This is the last memory block
        if(current->size_alloc == 1)
            return NULL;
        //This block is free and is big enough
        if(Is_Free(current) && current->payload >= size)
            break; 
        //Move to the next block
        else
            current = Get_Next_Header(current);
    }

    // allocate the block
    int total_size = size + 8;
    while(total_size % 16)
        total_size++;
    
    //record the user pointer that this function need to return
    void* user_pointer = (void *)((unsigned long)current+8);

    //No need to split 
    if(current->payload - total_size < 16){
        //Set the block header
        current->size_alloc += 1;
        current->payload = size;
        return  user_pointer;
    }
    //Split the free block
    else{  
        //Set the next block header
        BLOCK_HEADER* next_header =(BLOCK_HEADER *)((unsigned long)current + total_size);
        next_header->size_alloc =  current->size_alloc - total_size;
        next_header->payload = current->payload - total_size;
        //Set the block header
        current->size_alloc = total_size + 1;
        current->payload = size;
        return  user_pointer;
    }
    return NULL;
}


// return 0 on success
// return -1 if the input ptr was invalid
int Mem_Free(void *ptr){
    
    BLOCK_HEADER* current = first_header;
    // traverse the list and check all pointers to find the correct block 
    while(1){    
        //This is the last memory block, the free isn't success
        if(current->size_alloc == 1)
            return -1;
        //Find the block to free
        if( current + 1 == ptr)
            break; 
        //Move to the next block
        else
            current = Get_Next_Header(current); 
    }
    
    //This block is free in the first place
    if(Is_Free(current))
        return 0;   
    int total_size = current->size_alloc - 1;
    
    //Find the previous block
    int prev_is_empty = 0;
    BLOCK_HEADER* previous = first_header;
    if(current != first_header){
        while(1){
            //Find the previous block header
            if(Get_Next_Header(previous) == current )
                break;
            else
                previous = Get_Next_Header(previous); 
        }
        //Merge it if it is free
        if (Is_Free(previous)){
            total_size += previous->size_alloc;
            prev_is_empty = 1; 
        }
    }
    
    //Find the next block, merge it if it is not allocated
    BLOCK_HEADER* next = Get_Next_Header(current); 
    if (Is_Free(next))
        total_size += next->size_alloc;

#if DEBUG
    printf("previous_ptr: %p\n", previous);
    printf("header_ptr: %p\n", current);
    printf("user_ptr: %p\n", ptr);
    printf("next_ptr: %p\n\n", next);
#endif

    if(prev_is_empty){
        previous->size_alloc = total_size;
        previous->payload = total_size - 8 ;
    } 
    else{
        current->size_alloc = total_size;
        current->payload = total_size - 8 ;
    }
    return 0;
}
