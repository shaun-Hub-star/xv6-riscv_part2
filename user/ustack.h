// #ifndef USTACK_H
// #define USTACK_H

// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
// #include "kernel/param.h"

// #define nunits(len) (((len) + sizeof(Header) - 1) / sizeof(Header) + 1)

// const uint PAGE_SIZE = 4096;

// void *ustack_malloc(uint len);

// int ustack_free(void);

// #endif
#include "kernel/types.h"

void *ustack_malloc(uint len);
int ustack_free(void);
