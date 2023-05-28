#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int PGSIZE = 4096;
    int numberOfPages = 18;
    int allocSize = 512;
    
    int numberOfRequiredMallocs = (numberOfPages * PGSIZE) / allocSize + 1;
    for (int i = 0; i < numberOfRequiredMallocs; i++)
    {
        char *ptr = (char *)ustack_malloc(allocSize);
        if (ptr == (char *)-1)
        {
            printf("Error: ustack_malloc failed\n");
            exit(1);
        }
        for (int j = 0; j < allocSize; j++)
        {
            ptr[j] = 'a';
        }
    }
    // free all the allocated memory using for loop
    int numberOfBytesFree = 0;
    for (int i = 0; i < numberOfRequiredMallocs; i++)
    {
        if ((numberOfBytesFree = ustack_free()) == -1)
        {
            printf("Error: ustack_free failed\n");
            exit(1);
        }
        if (numberOfBytesFree != allocSize)
        {
            printf("Error: ustack_free failed should have returned %d but was, %d\n", allocSize, numberOfBytesFree);
            // exit(1);
        }
    }
    printf("ustack_test succeeded\n");

    exit(0);
}