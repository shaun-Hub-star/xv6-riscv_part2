#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

struct spinlock random_lock;
uint8 lfsr_seed = 0x2A;
// Linear feedback shift register
// Returns the next pseudo-random number
// The seed is updated with the returned value
uint8 lfsr_char(uint8 lfsr)
{
    uint8 bit;
    bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 0x01;
    lfsr = (lfsr >> 1) | (bit << 7);
    return lfsr;
}
int randomwrite(int user_src, uint64 src, int n)
{
    if (n != 1)
        return -1;
    lfsr_seed = ((uint8 *)src)[0];
    return 1;
}
int randomread(int user_dst, uint64 dst, int n)
{
    printf("randomread\n");
    int i;
    uint8 lfsr = lfsr_seed;
    acquire(&random_lock);
    for (i = 0; i < n; i++)
    {
        char c;
        lfsr = lfsr_char(lfsr);
        c = lfsr;
        if (either_copyout(user_dst, dst + i, &c, 1) == -1)
            break;
    }
    lfsr_seed = lfsr;
    release(&random_lock);
    return i;
}
void randominit(void)
{
    initlock(&random_lock, "random");
    devsw[RANDOM].read = randomread;
    devsw[RANDOM].write = randomwrite;
}