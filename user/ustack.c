#include "ustack.h"

static struct Header *base_stack = 0;
static struct Header *last_occupied = 0;

void *ustack_malloc(uint len)
{
    // struct Header *new_block = (struct Header *)sbrk((int)((len + sizeof(struct Header) - 1) / sizeof(struct Header) + 1));
    struct Header *new_block = (struct Header *)sbrk(sizeof(struct Header) + len);
    if (new_block == (struct Header *)-1)
        return (void *)-1;
    new_block->len = len;
    new_block->prev = last_occupied;
    last_occupied = new_block;
    if (base_stack == 0)
    {
        new_block->prev = 0;
        base_stack = new_block;
        last_occupied = new_block;
    }
    return (void *)(new_block + 1);
}

int ustack_free(void)
{
    if (base_stack == 0)
        return -1;

    int len = last_occupied->len;
    if (last_occupied == base_stack)
    {
        sbrk(-(len + sizeof(struct Header)));
        base_stack = 0;
        last_occupied = 0;
        return len;
    }
    last_occupied = last_occupied->prev;
    sbrk(-(len + sizeof(struct Header)));
    return len;
}