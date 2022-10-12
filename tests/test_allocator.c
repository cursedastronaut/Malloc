#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <m_allocator.h>

// Var type to allocate
#define ALLOC_TYPE int
// Number of variable allocation
#define ALLOC_COUNT 100
// Size of the allocations
#define ALLOC_SIZE (sizeof(ALLOC_TYPE))
// Whether to print the allocated values
#define PRINT_ALLOC_VALUE true
// Whether to reallocate all the variables
#define REALLOC false
// Size of the reallocations
#define REALLOC_SIZE (sizeof(ALLOC_TYPE))
// Whether to start freeing from the end or from the beginning of the variables
#define DEALLOC_START_END false

void test_huge_allocation(bool allocValue)
{
    ALLOC_TYPE* first[ALLOC_COUNT];
    m_show_info();
    printf(COLOR_YELLOW "Allocating %d values of size %ld...\n", ALLOC_COUNT, ALLOC_SIZE);
    for (int i = 0; i < ALLOC_COUNT; i++) {
        if (i % 7 == 0)
            first[i] = m_calloc(1, ALLOC_SIZE);
        else
            first[i] = m_malloc(ALLOC_SIZE);

#if PRINT_ALLOC_VALUE
        printf("Calloc: %d, ", i % 7 == 0);
        if (allocValue) {
            printf("Value: %d, ", *first[i]);
            printf("%d\n", *first[i] = i);
        } else
            printf("Value: %d\n", *first[i]);
#endif
    }
    printf(COLOR_GREEN "Values allocated.\n");
    m_show_info();

#if REALLOC
    printf(COLOR_YELLOW "Reallocating memory with new size %ld...\n", REALLOC_SIZE);
    for (int i = 0; i < ALLOC_COUNT; i++)
        first[i] = m_realloc(first[i], REALLOC_SIZE);
    printf(COLOR_GREEN "Memory reallocated.\n");
    m_show_info();
#endif // REALLOC

    printf(COLOR_YELLOW "Freeing memory...\n");
#if DEALLOC_START_END
   for (int i = ALLOC_COUNT - 1; i >= 0; i--)
#else
   for (int i = 0; i < ALLOC_COUNT; i++)
#endif // DEALLOC_START_END
      m_free(first[i]);
   printf(COLOR_GREEN "Memory freed.\n");
   m_show_info();
}

void test_alloc()
{

    /*char *array1 = m_malloc(128 * sizeof(char));
    strcpy(array1, "Bonjour");

    char *array2 = m_malloc(64 * sizeof(char));
    strcpy(array2, "salut");
    
    char *array3 = m_malloc(16 * sizeof(char));
    strcpy(array3, "Annyeong");
    //m_free(array2);
    //m_free(array1);
    char* test2 = m_calloc(12, sizeof(int));
    strcpy(test2, "AAAA");
    printf("%s\n", test2);
    m_realloc(test2, 999 * sizeof(int));
    printf("%s\n", test2);
    //printf("(malloc) a3 (%p) = %s\n", array3, array3);
    //printf("(malloc) a4 (%p) = %s\n", array4, array4);*/
    int* test1 = m_malloc(sizeof(int));
    printf("%d\n", *test1);
    m_show_info();

    
}

int main()
{

    m_setup_hooks();
    //test_alloc();
    test_huge_allocation(true);
    test_huge_allocation(false);
    return 0;
}
