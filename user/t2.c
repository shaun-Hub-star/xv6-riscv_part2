#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
void childCode();
void *thread();

void *thread()
{
    int status;
    int pid = fork();
    if (pid == 0)
    {
        childCode();
    }
    else
    {
        if (wait(&status) != -1)
        {
            if (status != 1)
            {
                printf("Error: child exited with status %d\n", status);
                status = -1;
                kthread_exit(-1);
            }
        }
        kthread_exit(0);
    }
    return 0;
}

void childCode()
{
    printf("Child: Noder im working\n");
    exit(1);
}
int main(int argc, char **argv)
{
    int temp = 0;
    int stackSize = 4096;
    void *kernelStack = malloc(stackSize);
    int thread_status;
    int tid = kthread_create(thread, kernelStack, stackSize);
    if (tid == -1)
    {
        temp++;
        printf("kthread_create failed\n");
        return 420;
    }
    kthread_join(tid, &thread_status);
    return 0;
}