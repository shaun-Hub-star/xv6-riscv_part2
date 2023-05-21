#include "ustack.h"

static struct Header *base_stack = 0;
static struct Header *last_occupied = 0;
// static unsigned long allocted_size = 0;

// void *ustack_malloc(uint len)
// {
//     if (base_stack == 0)
//     {
//         char *address = sbrk(CALC_PAGE);
//         if ((int)address == -1)
//             return -1;

//         base_stack = (struct Header *)address;
//         last_occupied = base_stack;
//         last_occupied->len = len;
//         last_occupied->prev = base_stack;
//         last_occupied->dealloc_page = 1;
//         allocted_size += len;
//         return (void *)(last_occupied + 1);
//     }

//     uint old_size_mod = allocted_size % PAGE_SIZE;

//     if (old_size_mod == 0)
//     {
//         struct Header *new_page = (struct Header *)sbrk(CALC_PAGE);
//         if ((int)new_page == -1)
//             return -1;
//         new_page->prev = last_occupied;
//         new_page->len = len;
//         last_occupied = new_page;
//         allocted_size += len;
//         return (void *)(last_occupied + 1);
//     }

//     if (old_size_mod + len <= PAGE_SIZE)
//     {
//         // not allocate
//         struct Header *new_page = (struct Header *)(((void *)(last_occupied + 1)) + last_occupied->len);
//         new_page->len = len;
//         new_page->prev = last_occupied;
//         allocted_size += len;
//         last_occupied = new_page;
//         return (void *)(last_occupied + 1);
//     }
//     else
//     {
//         // allocate
//         struct Header *new_page = (struct Header *)(((void *)(last_occupied + 1)) + last_occupied->len);
//         new_page->len = len;
//         new_page->prev = last_occupied;
//         if ((int)sbrk(CALC_PAGE) == -1)
//             return -1;
//         last_occupied = new_page;
//         allocted_size += len;
//         return (void *)(last_occupied + 1);
//     }
// }

void *ustack_malloc(uint len)
{

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