#include "types.h"
#include "uthread.h"

struct uthread uthreads[MAX_UTHREADS];
struct uthread *my_thread;

int uthread_create(void (*start_func)(), enum sched_priority priority)
{
    return 0;
}
