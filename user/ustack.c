#include "ustack.h"

static Header *stackPointer = 0;
static long totalNumberOfNUnits = 0;
static Header *baseStackPointer = 0;
static int _numberOfPages = 0;
static int _numberOfHeaders = 0;
int balanceChanging(long newLenUnits, unsigned long numberOfNunitsInHeader)
{
    _numberOfHeaders++;
    _numberOfPages = newLenUnits / numberOfNunitsInHeader;

    return ((totalNumberOfNUnits / numberOfNunitsInHeader) != (newLenUnits / numberOfNunitsInHeader));
}

void *
ustack_malloc(uint len)
{
    uint nunits = nunits(len);
    int sum = 0;
    if (stackPointer == 0)
    {
        sum = PAGE_SIZE + nunits * sizeof(Header);
        char *p1 = sbrk(PAGE_SIZE);
        if (p1 == (char *)-1)
            return (void *)-1;
        stackPointer = (Header *)p1;
        baseStackPointer = (Header *)p1;

        stackPointer->data.headerUnits = 0;
        stackPointer->data.ptr = 0;
        baseStackPointer->data.headerUnits = 0;
        baseStackPointer->data.ptr = 0;
    }
    sum += totalNumberOfNUnits + nunits + 1;
    unsigned long numberOfNunitsInHeader = PAGE_SIZE / sizeof(Header);
    long newLenUnits = totalNumberOfNUnits + nunits + 1;

    if (balanceChanging(newLenUnits, numberOfNunitsInHeader))
    {
        char *p2 = sbrk(PAGE_SIZE);
        if (p2 == (char *)-1)
            return (void *)-1;
    }

    totalNumberOfNUnits = newLenUnits;
    Header *newHeader = stackPointer + 1 + stackPointer->data.headerUnits;
    newHeader->data.ptr = stackPointer;
    newHeader->data.headerUnits = nunits;

    stackPointer = newHeader;
    return stackPointer + 1;
}

int ustack_free(void)
{
    int returnInCaseOfFailure = -1;
    if (stackPointer == baseStackPointer)
    {
        return returnInCaseOfFailure;
    }
    uint freeNumberOfBytes = (stackPointer->data.headerUnits - 1) * sizeof(Header);
    uint newDataSizeUnits = totalNumberOfNUnits - stackPointer->data.headerUnits - 1;
    stackPointer = stackPointer->data.ptr;
    unsigned long numberOfNunitsInHeader = PAGE_SIZE / sizeof(Header);
    if (balanceChanging(newDataSizeUnits, numberOfNunitsInHeader))
    {
        sbrk(-PAGE_SIZE);
    }

    totalNumberOfNUnits = newDataSizeUnits;

    return freeNumberOfBytes;
}
