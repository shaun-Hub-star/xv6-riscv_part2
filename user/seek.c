#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main()
{
    // create file
    int fd = open("seek.txt", O_RDWR | O_CREATE);
    char buf[10];
    int i;
    for (i = 0; i < 10; i++)
    {
        buf[i] = 'a' + i;
    }
    write(fd, buf, 10);
    close(fd);

    // seek and read
    fd = open("seek.txt", O_RDWR);
    char buf2[10];
    read(fd, buf2, 10);
    printf("buf2: %s\n", buf2);
    seek(fd, 0, SEEK_SET);
    char buf3[10];
    read(fd, buf3, 10);
    printf("buf3: %s\n", buf3);
    close(fd);
    return 0;
}