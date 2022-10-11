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
 
void m_show_info(void)
{
    for (Metadata* m = metadata; m != NULL; m = m->next)
    {
        printf("Address: %p, Pointer: %p, Size: %ld, Free : %d , Next: %p\n", m, m->ptr, m->size, m->free, m->next);
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
    return NULL;
}
 
void* m_calloc(size_t nb, size_t size)
{
    
    return m_malloc(size);
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