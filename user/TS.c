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

void start_func_3()
{
    printf("Thread 3 running\n");
    uthread_exit();
}

void start_func_4()
{
    printf("Thread 4 running\n");
    uthread_exit();
}

void start_func_5()
{
    printf("Thread 5 running\n");
    uthread_exit();
}

int main()
{
    uthread_create(start_func_1, LOW);
    uthread_create(start_func_2, HIGH);
    uthread_create(start_func_3, MEDIUM);
    uthread_create(start_func_4, LOW);
    int result = uthread_create(start_func_5, HIGH);
    if (result == -1)
    {
        printf("Failed to create thread 5\n");
    }
    uthread_start_all();
    exit(0);
}