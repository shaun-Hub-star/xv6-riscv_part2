#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
void *simpleCreateThreadTest()
{
    kthread_exit(0);
    return 0;
}

int main(int argc, char **argv)
{
    int kernelStackSize = 4096;
    void *kernelStack = malloc(kernelStackSize);
    int tid;

    if ((tid = kthread_create(simpleCreateThreadTest, kernelStack, kernelStackSize)) != 2)
    {
        printf("kthread_create failed\n");
        return 420;
    }
    printf("YAY\n");
    kthread_exit(0);
    return 1;
}