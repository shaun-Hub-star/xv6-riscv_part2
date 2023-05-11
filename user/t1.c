#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Function to check if a number is prime
int is_prime(int n)
{
    if (n < 2)
    {
        return 0;
    }
    for (int i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
        {
            return 0;
        }
    }
    return 1;
}

// Function to find the nth prime number
void *find_prime_50000000()
{
    int n = 50000000;
    int count = 0;
    int i = 2;
    while (count < n)
    {
        if (is_prime(i))
        {
            count++;
        }
        if (count == n)
        {
            printf("prime 50000000 = %d\n", i);
            kthread_exit(1);
            return 0;
        }
        i++;
    }
    return 0;
    // return -1; // Error: nth prime not found
}

void *prime50000()
{
    int n = 50000;
    int count = 0;
    int i = 2;
    while (count < n)
    {
        if (is_prime(i))
        {
            count++;
        }
        if (count == n)
        {
            printf("prime 50000 = %d\n", i);
            kthread_exit(1);
            return 0;
        }
        i++;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int status;
    int temp = 0;
    temp += temp + 4096;
    int stackSize = 4096;
    void *firstThreadStack = malloc(stackSize);
    void *secondThreadStack = malloc(stackSize);
    int tid = kthread_create(find_prime_50000000, firstThreadStack, stackSize);
    kthread_kill(tid);
    if (kthread_join(tid, &status) != 0)
    {
        printf("Failed to join, the first thread should not have finished\n");
        return -1;
    }

    int tid2 = kthread_create(prime50000, secondThreadStack, stackSize);
    printf("tid2 created = %d\n", tid2);
    kthread_join(tid2, &status);
    if (status != 1)
    {
        printf("Failed to join, the first thread should not have finished\n");
        return -1;
    }
    printf("Noder!\n");
    temp++;
    return 1;
}