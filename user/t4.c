#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void *exectest()
{
    int exitStatus = 0;
    char *command = "echo";
    char *argv[] = {"echo", "exec test", 0};
    exec(command, argv);
    exitStatus = 1; // clear warning
    printf("exec failed with status: %d\n", exitStatus);
    return 0; // warning
}
int main(int argc, char **argv)
{
    int kernelStackSize = 4096;
    void *kernelStack = malloc(kernelStackSize);
    kthread_create(exectest, kernelStack, kernelStackSize);
    kthread_exit(0);
    printf("should go back to terminal\n");
    return 0;
}