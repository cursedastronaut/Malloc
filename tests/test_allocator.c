#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <m_allocator.h>

void test_alloc()
{
    // MALLOC
    /*int* a = malloc(sizeof(int));
     *a = 10;*/
    char *array1 = m_malloc(128 * sizeof(char));
    strcpy(array1, "Bonjour");

    char *array2 = m_malloc(64 * sizeof(char));
    strcpy(array2, "salut");
    
    char *array3 = m_malloc(16 * sizeof(char));
    strcpy(array3, "Annyeong");
    m_free(array2);
    m_free(array1);

    /*printf("(malloc) a3 (%p) = %s\n", array3, array3);*/
    /*printf("(malloc) a4 (%p) = %s\n", array4, array4);*/
    m_show_info();

    
}

int main()
{
    m_setup_hooks();
    test_alloc();

    return 0;
}
