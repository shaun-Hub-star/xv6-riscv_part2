#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "uthread.h"

void start_func_1()
{
    printf("Thread 1 running\n");
    uthread_exit();
}

void start_func_2()
{
    printf("Thread 2 running\n");
    uthread_exit();
}

int main()
{
    int a = uthread_create(start_func_1, HIGH);
    int b = uthread_create(start_func_2, LOW);
    printf("LOW: %d\n", a);
    printf("HIGH:  %d\n", b);
    int x = uthread_start_all();
    printf("aaaaa:  %d\n", x);
    return 0;
}