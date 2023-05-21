#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    char *hello0 = (char *)ustack_malloc(2000);

    strcpy(hello0, "hello0\n");

    char *hello1 = (char *)ustack_malloc(3300);

    for (int i = 0; i < 3300; i++)
    {
        hello1[i] = 'a';
    }

    char *hello2 = (char *)ustack_malloc(2000);

    strcpy(hello2, "hello2\n");

    printf(hello2);

    printf("Hello2 len:%d\n", ustack_free());
    printf("Hello len:%d\n", ustack_free());
    printf(hello0);

    exit(0);
}