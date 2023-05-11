#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int exitStatus = 0;
void *exectest()
{

    char *command = "echo";
    char *argv[] = {"echo", "SUCCESS", 0};
    int exec_val = exec(command, argv);
    if (exec_val != -1)
    {
        printf("exec failed with status: %d\n", exec_val);
        exitStatus = 1;
        return 0;
    }
    return 0; // warning
}
int main(int argc, char **argv)
{
    int kernelStackSize = 4096;
    void *kernelStack = malloc(kernelStackSize);
    kthread_create(exectest, kernelStack, kernelStackSize);
    if (exitStatus == 0)
        printf("should go back to terminal\n");
    kthread_exit(0);

    return 0;
}