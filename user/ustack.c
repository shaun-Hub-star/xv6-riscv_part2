// #include "ustack.h"

// typedef long Align;

// union header // total of 16 bytes
// {
//     struct
//     {
//         union header *prev;
//         uint len;
//     } data;
//     Align x;
// };

// typedef union header Header;
// static Header *base_stack = 0;
// static Header *last_occupied = 0;
// static uint numberOfTotalUnits = 0;
// static int numberOfHeaders = 0;
// void *ustack_malloc(uint len)
// {
//     uint nunits = nunits(len);

//     if (base_stack == 0)
//     {
//         Header *new_block = (Header *)sbrk(PAGE_SIZE);
//         if (new_block == (Header *)-1)
//             return (void *)-1;
//         new_block->data.len = nunits;
//         last_occupied = new_block;
//         new_block->data.prev = 0;
//         base_stack = new_block;
//         numberOfHeaders++;

//         return (void *)(new_block + 1);
//     }
//     long numberOfUpdatedUnits = numberOfTotalUnits + nunits + sizeof(Header) / sizeof(Header);
//     unsigned long numberOfUnitsInPage = (PAGE_SIZE / sizeof(Header));
//     if ((numberOfTotalUnits / numberOfUnitsInPage) != (numberOfUpdatedUnits / numberOfUnitsInPage))
//     {
//         char *new_block = sbrk(PAGE_SIZE);
//         if (new_block == (char *)-1)
//             return (void *)-1;
//     }
//     Header *new_block = last_occupied + 1 + last_occupied->data.len; // skip the current header and add data

//     numberOfTotalUnits = numberOfUpdatedUnits;
//     new_block->data.len = nunits;
//     new_block->data.prev = last_occupied;
//     last_occupied = new_block;
//     numberOfHeaders++;

//     return (void *)(new_block + 1);
// }

// int ustack_free(void)
// {
//     if (numberOfHeaders == 0)
//         return -1;

//     int numberOfBytes = last_occupied->data.len * sizeof(Header);
//     long numberOfUpdatedUnits = numberOfTotalUnits - last_occupied->data.len - sizeof(Header) / sizeof(Header);
//     last_occupied = last_occupied->data.prev; // move to the previous header
//     unsigned long numberOfUnitsInPage = (PAGE_SIZE / sizeof(Header));
//     if ((numberOfTotalUnits / numberOfUnitsInPage) != (numberOfUpdatedUnits / numberOfUnitsInPage))
//     {
//         char *new_block = sbrk(-PAGE_SIZE);
//         if (new_block == (char *)-1)
//             return -1;
//     }
//     numberOfTotalUnits = numberOfUpdatedUnits;
//     return numberOfBytes;
// }

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include "kernel/riscv.h"

typedef long Align;

union header
{
    struct
    {
        union header *ptr;
        uint size;
    } s;
    Align x;
};

typedef union header Header;

static Header *bottom = 0;
static Header *topHeader = 0;
static long dataSizeUnits = 0; // 256 units is 1 page
// static int numberOfHeaders = 0;
// task1
void *
ustack_malloc(uint len)
{
    // how many header sizes does len take
    uint nunits = (len + sizeof(Header) - 1) / sizeof(Header) + 1;

    // init first time
    if (topHeader == 0)
    {
        char *p1 = sbrk(PGSIZE);
        if (p1 == (char *)-1)
            return (void *)-1;
        bottom = (Header *)p1;
        bottom->s.size = 0;
        bottom->s.ptr = 0;
        topHeader = (Header *)p1;
        topHeader->s.size = 0;
        topHeader->s.ptr = 0;
    }

    // check if we need a new page, compare nunits (not len) before and after adding the data.
    long newLenUnits = dataSizeUnits + nunits + 1;
    if ((dataSizeUnits / (PGSIZE / sizeof(Header))) != (newLenUnits / (PGSIZE / sizeof(Header))))
    {
        // dividing long by int, should round down.
        // create new page
        char *p2 = sbrk(PGSIZE);
        if (p2 == (char *)-1)
            return (void *)-1;
    }

    dataSizeUnits = newLenUnits;
    // top points at current top header.
    // now it should point to the new header
    Header *newTop = topHeader + 1 + topHeader->s.size;
    newTop->s.size = nunits;
    // stack (next) points downwards instead of upwards (in order to free in O(1))
    newTop->s.ptr = topHeader;
    // topHeader->s.ptr = newTop;
    topHeader = newTop;
    // return address of data (without header)
    // numberOfHeaders++;
    return topHeader + 1;
}

int ustack_free(void)
{
    // check if stack empty
    if (topHeader == bottom)
    {
        return -1;
    }
    // how many bytes do we free from the buffer (data)
    uint returnSize = topHeader->s.size * sizeof(Header);
    uint newDataSizeUnits = dataSizeUnits - topHeader->s.size - 1;
    topHeader = topHeader->s.ptr;

    // check if we need to close page (PGSIZE / sizeof(Header) is 256 units)
    if ((dataSizeUnits / (PGSIZE / sizeof(Header))) != (newDataSizeUnits / (PGSIZE / sizeof(Header))))
    {
        sbrk(-PGSIZE);
    }

    dataSizeUnits = newDataSizeUnits;
    // numberOfHeaders--;
    return returnSize;
}
