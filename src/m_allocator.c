#include <malloc.h> // Nécessaire pour les hooks
#include <unistd.h>
#include <m_allocator.h>
#include <stdbool.h>
#include "metadata.h"
//It's day 1 and I truly wanna give up.
static Metadata* metadata = NULL;

void* malloc_hook(size_t size, const void* caller)            { return m_malloc(size); }
void* realloc_hook(void* ptr, size_t size, const void* caller){ return m_realloc(ptr, size); }
void  free_hook(void* ptr, const void* caller)                { return m_free(ptr); }

void m_setup_hooks(void)
{
   __malloc_hook = malloc_hook;
   __realloc_hook = realloc_hook;
   __free_hook = free_hook;
}
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m" 
void m_show_info(void)
{
    printf(COLOR_YELLOW "Show the blocks (m_show_info):\n");
    for (Metadata* m = metadata; m != NULL; m = m->next)
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

Metadata* split_block(Metadata* m, size_t size)
{
    //m->size = size;
    char* temp = m + sizeof(Metadata) + size;
    Metadata* unusedData = temp;
    unusedData->size = m->size - size - sizeof(Metadata);
    unusedData->free = true;
    unusedData->ptr = unusedData + 1;
    unusedData->next = m->next;
    m->next = unusedData;
    return m;
}

void fusion_block(Metadata* m1, Metadata* m2)
{
    m1->size += m2->size + sizeof(Metadata);
    m1->next = m2->next;
}

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

Metadata* get_free_block(size_t size)
{
    for (Metadata* m = metadata; m != NULL; m = m->next)
        if (m->free)
        {
            if (m->size == size)
            {return m;}
            else if (m->size > size)
            {return split_block(m, size);}
        }
            
    return NULL; 
    
}
void* copy_value(void* newLocation, void* oldLocation)
{
    int* newVal = newLocation;
    int* oldVal = oldLocation;
    *newVal = *oldVal;
    return newVal;
}


void* m_malloc(size_t size)
{
    Metadata* data = get_free_block(size);
    if (data != NULL)
    {
        //Metadata* data = sbrk(sizeof(Metadata));
        void* new = data->ptr;
        data->free = false;
        return new;
    }
    else
    {
        Metadata* data = sbrk(sizeof(Metadata));
        void* new = sbrk(size);
        data->ptr = new;
        data->size = size;
        data->free = false;

        if (metadata == NULL)
        {
            metadata = data;
        }
        else
            for (Metadata* m = metadata; m != NULL; m = m->next)
            {
                if (m->next == NULL)
                {
                    m->next = data;
                    break;
                }
            }
        
        return new; 
    }
    
}
 
void* m_realloc(void* ptr, size_t size)
{
    if (size == 0) {
        m_free(ptr);
        return NULL;
    }

    if (ptr == NULL)
        return m_malloc(size);

    Metadata* data = NULL;
    for (Metadata* m = metadata; m != NULL; m = m->next)
        if (m->next != NULL)
            if (m->next->ptr == ptr) {
                data = m;
                break;
            }
    if (data == NULL)
        return NULL;

    switch (auto_fusion_block(data)) {
        case 1:
            if (data->next->size == size)
                return data->ptr;
            if (data->next->size > size) {
                if (split_block(data->next, size) == 0)
                    return copy_value(m_malloc(size), ptr);
                copy_value(data->ptr, ptr);
                if (data->next->next == NULL)
                    m_free(data->next->ptr);
                return data->ptr;
            }
            data->next->free = true;
            return copy_value(m_malloc(size), ptr);
            
        case 2:
            if (data->size == size)
                return data->ptr;
            if (data->size > size) {
                if (split_block(data, size) == 0)
                    return copy_value(m_malloc(size), ptr);
                copy_value(data->ptr, ptr);
                if (data->next->next == NULL)
                    m_free(data->next->ptr);
                return data->ptr;
            }
            data->free = true;
            return copy_value(m_malloc(size), ptr);

        case 0:
            data->next->free = true;
            return copy_value(m_malloc(size), ptr);
    }

    return NULL;
}
 
void* m_calloc(size_t nb, size_t size)
{
    char *ptr;
    size_t dat;

    if ((ptr = m_malloc(size * nb)) == NULL)
        return (NULL);
    dat = 0;
    while (dat < size)
    {
        ptr[dat] = 0;
        ++dat;
    }
    return (ptr);
}
 
void m_free(void* ptr)
{
    Metadata* prev = NULL;
    for (Metadata* m = metadata; m != NULL; m = m->next)
    {
        if (m != NULL && m->next != NULL)
        {
            m->free = true;
            Metadata* next = m->next;
            if (m->free && next->free)
                fusion_block(m, next);
            if (prev && m->free && prev->free)
                fusion_block(prev, m);

        }

        prev = m;
    }
}