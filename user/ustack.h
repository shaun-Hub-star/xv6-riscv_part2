#ifndef USTACK_H
#define USTACK_H

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

struct Header // if not working - union needed
{
    uint len;
    struct Header *prev;
};

const uint PAGE_SIZE = 4096;
const uint CALC_PAGE = PAGE_SIZE + PAGE_SIZE * sizeof(struct Header);

void *ustack_malloc(uint len);

int ustack_free(void);

#endif