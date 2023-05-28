#ifndef USTACK_H
#define USTACK_H

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

typedef long Align;

union header
{
    struct
    {
        union header *ptr;
        uint headerUnits;
    } data;
    Align x;
};

typedef union header Header;

#define nunits(len) (((len) + sizeof(Header) - 1) / sizeof(Header) + 1)

#define PAGE_SIZE 4096
void *ustack_malloc(uint len);

int ustack_free(void);

#endif