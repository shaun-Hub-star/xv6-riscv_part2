#include "types.h"
#include "uthread.h"

struct uthread uthreads[MAX_UTHREADS];
struct uthread *my_thread;

int uthread_create(void (*start_func)(), enum sched_priority priority)
{
    int i;
    for (i = 0; i < MAX_UTHREADS; i++) // no locks - only one proccessor at the time
        if (uthreads[i].state == FREE)
            break;

    if (i == MAX_UTHREADS)
        return -1;

    uthreads[i].priority = priority;
    uthreads[i].context.ra = (uint64)start_func;
    uthreads[i].context.sp = (uint64)&uthreads[i].ustack[STACK_SIZE]; // need to check if at 0 or STACK_SIZE or STACK_SIZE-1

    uthreads[i].state = RUNNABLE;
    return 0;
}
void uthread_yield()
{
}
