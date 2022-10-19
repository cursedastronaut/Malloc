#include <malloc.h> // Nécessaire pour les hooks
#include <unistd.h>
#include <m_allocator.h>
#include <stdbool.h>
#include "metadata.h"

#define DEBUG true
//It's day 1 and I truly wanna give up.
//It's day 4, and despite having made some significant advancement,
//partially with the help of some classmate, I still wish to give up.
//It's one day before you receive this! I'm working on making it better.
static Metadata* metadata = NULL;

//Hooks
static int gHooked = 0;
extern void *__libc_malloc(size_t);
extern void __libc_free(void*);
extern void *__libc_calloc(size_t, size_t);
extern void *__libc_realloc(void*, size_t);

void* malloc(size_t size)             { return gHooked ? m_malloc(size)       : __libc_malloc(size); }
void  free(void* ptr)                 {        gHooked ? m_free(ptr)          : __libc_free(ptr); }
void* calloc(size_t num, size_t size) { return gHooked ? m_calloc(num, size)  : __libc_calloc(num, size); }
void* realloc(void* ptr, size_t size) { return gHooked ? m_realloc(ptr, size) : __libc_realloc(ptr, size); }
 
void m_setup_hooks(void)
{
    gHooked = 1;
}

/* M SHOW INFO - Shows useful debugging informations about the blocks.*/
void m_show_info(void)
{
    //Skips the first block caused by printf (It is irrelevant).
    Metadata* m = metadata; 
    printf(COLOR_YELLOW "Show the blocks (m_show_info):\n");
    if (DEBUG == true) //Do not enable for your test! It justs skips the first if you used printf.
        m = m->next;
    for (; m != NULL; m = m->next)
    {
        printf(COLOR_GREEN "Adress : %p", m);
        printf(COLOR_RESET " | ");
        printf(COLOR_MAGENTA "Pointer : %p", m->ptr);
        printf(COLOR_RESET " | ");
        printf(COLOR_BLUE "Size : %4.ld", m->size);
        printf(COLOR_RESET " | ");
        printf(COLOR_YELLOW "Free : %d", m->free);
        printf(COLOR_RESET " | ");
        printf(COLOR_CYAN "Next : %14.p\n", m->next);
        printf(COLOR_RESET "");
        //printf(COLOR_RED "Address: %p, Pointer: %p, Size: %ld, Free : %d , Next: %p\n", m, m->ptr, m->size, m->free, m->next);
    }
}

/* SPLIT BLOCK - Split blocks when called */
Metadata* split_block(Metadata* m, size_t size)
{
    void* maddress = m;
    //m->size = size;
    Metadata* unusedData = maddress + sizeof(Metadata) + size;
    unusedData->size = m->size - size - sizeof(Metadata);
    unusedData->free = true;
    unusedData->ptr = unusedData + 1;
    unusedData->next = m->next;
    m->next = unusedData;
    return m;
}

/* FUSION BLOCK - Fusions two blocks (m1 and m2) when called*/
void fusion_block(Metadata* m1, Metadata* m2)
{
    m1->size += m2->size + sizeof(Metadata);
    m1->next = m2->next;
}
/* AUTO FUSION BLOCK - Advanced fusion block */
int auto_fusion_block(Metadata* prev)
{
    Metadata* block = prev->next;
    if (prev->free) {
        prev->size += block->size + sizeof(Metadata);
        prev->next = block->next;
        if (block->next != NULL)
            if (block->next->free) {
                prev->size += block->next->size + sizeof(Metadata);
                prev->next = block->next->next;
                return 2;
            }
        return -1;
    } else if (block->next != NULL)
        if (block->next->free) {
            block->size += block->next->size + sizeof(Metadata);
            block->next = block->next->next;
            return 1;
        }
    return 0;
}
/* COPY VALUE - Copies the values of the oldLocation to the newLocation*/
void* copy_value(void* newLocation, void* oldLocation)
{
    int* newVal = newLocation;
    int* oldVal = oldLocation;
    *newVal = *oldVal;
    return newVal;
}
/*Realloc processing.*/
Metadata* treal(Metadata* data, Metadata* data2, size_t size, void* ptr)
{
            if (data2->size == size)
                return data->ptr;
            if (data2->size > size) {
                if (split_block(data2, size) == 0)
                    return copy_value(m_malloc(size), ptr);
                copy_value(data->ptr, ptr);
                if (data2->next == NULL)
                    m_free(data->next->ptr); //Must not be data2->ptr
                return data->ptr;
            }
            data->next->free = true;
            return copy_value(m_malloc(size), ptr);
}
/* GET FREE BLOCK - Searches for free blocks of the right size or below, and returns their adress*/
Metadata* get_free_block(size_t size)
{
    for (Metadata* m = metadata; m != NULL; m = m->next)
    {
        if (m->free)
        {
            if (m->size == size)
                return m;
            else if (m->size > size)
                return split_block(m, size);
        }
    }
            
    return NULL; 
    
}
/* MALLOC - Allocates memory. */
void* m_malloc(size_t size)
{
    //Puts the size at a multiple of 8.
    for(; size % 8 != 0; size ++);
    //Searches for free blocks of the correct size or below.
    Metadata* data = get_free_block(size);
    if (data != NULL) //It found a free block with the right size or lower!
    {
        void* new = data->ptr;
        data->free = false;
        return new;
    }
    else //It didn't find a free block with the right size.
    {
        Metadata* data = sbrk(sizeof(Metadata));
        void* new = sbrk(size);
        data->ptr = new;
        data->size = size;
        data->free = false;

        if (metadata == NULL) //If it's the first block.
        {
            metadata = data;
        }
        else 
            for (Metadata* m = metadata; m != NULL; m = m->next)    //Searches for the block
            {                                                       //before the one we are
                if (m->next == NULL)                                //allocating right now.
                {
                    m->next = data;                                 //Says the current block
                    break;                                          //is the next of the previous one.
                }
            }
        
        return new; 
    }
    
}

/* REALLOC - Changes the size of a memory block previously allocated */ 
void* m_realloc(void* ptr, size_t size)
{
    for(; size % 8 != 0; size ++);
    if (size == 0)  //Just in case a developer is a bit dumb (hi future me)
    {
        m_free(ptr);
        return NULL;
    }

    if (ptr == NULL) //Saving the user if they put NULL as ptr.
        return m_malloc(size);

    Metadata* data = NULL;
    for (Metadata* m = metadata; m != NULL; m = m->next) //Searching our block
        if (m->next != NULL)
            if (m->next->ptr == ptr) 
            {
                data = m; //Data = PREVIOUS ; Data->next = CURRENT BLOCK ; Data->next->next = NEXT BLOCK
                break;
            }
    if (data == NULL) //If your pointer points nowhere.
        return NULL;
    if (size < data->next->size) // If you're trying to reduce the size of the block
    {
        if (data->next->next != NULL)   //If there is a block after this one
        {                               //We split this block so the other part is free.
            Metadata* unusedData = data->next + size;
            unusedData->free = true;
            unusedData->next = data->next->next;
            data->next->next = unusedData;
            unusedData->size = data->next->size - size;
            unusedData->ptr = unusedData + sizeof(Metadata*);
            data->next->size = size;
            return data;
        }
        else
        {
            data->next->size = size;
            return data;
        }
    }
    if (data->next->next == NULL) //If there is no block after this one
    {
        data->next->size = size;
        return ptr;
    }
    else
    {
        switch (auto_fusion_block(data))
        {
        case 1:
            return treal(data, data->next, size, &ptr);
            
        case 2:
            return treal(data, data, size, &ptr);

        case 0:
            data->next->free = true;
            return copy_value(m_malloc(size), ptr);
        }
    }
    

    return NULL;
}

/* CALLOC - Allocates memory, and sets everything to zero.*/
void* m_calloc(size_t nb, size_t size)
{
    char *ptr;
    size_t dat;

    if ((ptr = m_malloc(size * nb)) == NULL)
        return (NULL);
    dat = 0;
    //Actually sets the values to zeros
    for (;dat < size; dat ++)
    {
        ptr[dat] = 0;
    }
    return (ptr);
}
 
/*FREE - Frees a previously allocated block */
void m_free(void* ptr)
{
    Metadata* prev = NULL;
    for (Metadata* m = metadata; m != NULL; m = m->next) //Searches the block at the adress ptr
    {
        if (m->ptr == ptr)  //Verifies the block has the adress we are searching for
        {                   //Forgetting this results in all the blocks being freed.
            m->free = true;
            if (prev && prev->free)     //If there is a previous block that is free,
                fusion_block(prev, m);  //we fusion them. fusion_block(previous, current)

            Metadata* next = m->next;   //Helps with clarity.
            if (next && next->free)     //If the next block is free, we fusion them.
                fusion_block(m, next);
            if (next == NULL)           //If there is no block after, we reduce the brk size.
                sbrk(-(m->size + sizeof(Metadata)));
            break;
        }

        prev = m;
    }
}
