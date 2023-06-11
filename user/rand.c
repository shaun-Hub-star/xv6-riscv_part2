#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main()
{
    // test random
    int fd = open("random", O_RDWR);
    printf("fd: %d\n", fd);
    char buf[10];
    read(fd, buf, 10);
    printf("buf: %s\n", buf);
    close(fd);
    for (int i = 0; i < 10; i++)
    {
        if (buf[i] == 0)
        {
            printf("0\n");
        }
    }
    char buffer[256 - 10];
    fd = open("random", O_RDWR);
    read(fd, buffer, 256 - 10);
    close(fd);
    if (buf[0] == buffer[256 - 10 - 1])
        printf("random test success\n");
    else
        printf("random test failed\n");

    return 0;
}