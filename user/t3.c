#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void *test_exit();

int main(int argc, char **argv)
{

    int kernelStackSize = 4096;
    void *kernelStack1 = malloc(kernelStackSize);
    void *kernelStack2 = malloc(kernelStackSize);
    int threadId = kthread_create(test_exit, kernelStack1, kernelStackSize);
    threadId++; // return value warning
    threadId = kthread_create(test_exit, kernelStack2, kernelStackSize);

    printf("exited from child threads succesfully\n");
    kthread_exit(0);
    threadId = 0; // clear warning
    return 1;
}
void *test_exit()
{
    // simple exit
    kthread_exit(0);
    return 0;
}