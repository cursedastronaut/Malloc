#include <stdbool.h>

typedef struct Metadata
{
   // Add metadata information needed
   // e.g. size of the block, pointer, free/occupied
    bool free;
    size_t size;
    struct Metadata* next;
    void* ptr;
} Metadata;