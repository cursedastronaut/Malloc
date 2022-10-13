#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <m_allocator.h>

void case1()
{

}

void test_alloc()
{
    printf("Choose what test you want to initialize:\n"
    "1 - Malloc 3 blocks, then free everything but the block you choose.\n"
    "(Useful to test Malloc, Free, Split, and Fusion)\n\n"
    "2 - Compares Calloc and Malloc variable results.\n"
    "(Useful to test the calloc function)\n\n"
    "3 - Reduce break size after freeing last block\n"
    "(Useful to test the break optimization of the free function)"
    "4 - Your own code.\n");
    int choice = 0;
    scanf("%d", &choice);
    system("clear");
    switch(choice)
    {
        case 1:
        {
            printf("Explanation: 3 blocks are created. Free the one you choose.\n");
            char *array1 = m_malloc(128 * sizeof(char));
            char *array2 = m_malloc(64 * sizeof(char));
            char *array3 = m_malloc(32 * sizeof(char));
            m_show_info();
            printf("Which block will not be freed? (1-3)\n");
            choice = 0;
            scanf("%d", &choice);
            switch (choice)
            {
            case 1:
                free(array2);
                free(array3);
                break;
            
            case 2:
                free(array1);
                free(array3);
                break;
            
            case 3:
                free(array1);
                free(array2);

            default:
                break;
            }
            m_show_info();
            break;
        }


        case 2:
        {
            
            printf("Explanation: This test initializes 100 values with malloc, and sets them to one, \n"
            "It then frees the array, and callocs at the same place. If calloc failed to\n"
            "set the values to zero, the values left by the malloc function will be 1.\n"
            "Type any character to begin.");
            int *test2[100];
            int testmal[100];
            int testcal[100];
            scanf("%d", &choice);
            system("clear");
            for (int i = 0; i < 100; i++)
            {
                test2[i] = m_malloc(sizeof(int));
                *test2[i] += 1;
                testmal[i] = *test2[i];
            }
            free(*test2);
            for (int i = 0; i < 100; i++)
            {
                test2[i] = m_calloc(1,sizeof(int));
                testcal[i] = *test2[i];
            }
            
            for (int i = 0; i < 100; i++)
            {
                printf("Malloc: %d | Calloc %d | Test: %s\n", testmal[i], testcal[i], testmal[i] != testcal[i] ? "Test réussi" : "Test raté");
            }
            break;
        }

        default:
            break;
    }

}

int main()
{
    printf(" ");
    m_setup_hooks();
    test_alloc();
    // test_huge_allocation(false);
    return 0;
}
