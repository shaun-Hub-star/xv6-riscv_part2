#include "kernel/types.h"
#include "uthread.h"
#include "user.h"

struct uthread uthreads[MAX_UTHREADS];
struct uthread *my_thread = 0;
static int started = 0;

int uthread_yield_internal();
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
    uthreads[i].context.sp = (uint64)&uthreads[i].ustack[STACK_SIZE - 1]; // need to check if at 0 or STACK_SIZE or STACK_SIZE-1

    uthreads[i].state = RUNNABLE;
    return 0;
}
void uthread_yield()
{
    if (uthread_yield_internal() == -1) // if we passed this line we got an error
        exit(69);
}
int uthread_yield_internal()
{
    int startingIndex = (my_thread - uthreads);
    int i, modI;
    int minPos = -1, minPriority = 3;
    for (i = startingIndex + 1; i < startingIndex + MAX_UTHREADS; i++)
    {
        modI = i % MAX_UTHREADS;
        // printf("Thread num:%d   state:%d\n", modI, uthreads[modI].state);
        if (uthreads[modI].state == RUNNABLE && uthreads[modI].priority < minPriority)
        {
            minPos = modI;
            minPriority = uthreads[modI].priority;
        }
    }
    if (minPos == -1)
        return -1;

    struct uthread *old = my_thread;

    my_thread = &uthreads[minPos]; // we preformed this before the next line because after the next line the processor will run code of the next thread
    uswtch(&old->context, &my_thread->context);
    return 0;
}

void uthread_exit()
{
    my_thread->state = FREE;
    if (uthread_yield_internal() == -1) // if we dont pass this line there is another thread to run
        exit(0);
}
enum sched_priority uthread_set_priority(enum sched_priority priority)
{
    enum sched_priority prev = my_thread->priority;
    my_thread->priority = priority;
    return prev;
}

enum sched_priority uthread_get_priority()
{
    return my_thread->priority;
}

int uthread_start_all() // need to init my_thread
{
    if (started)
        return -1;
    started = 1;
    int i;
    int minPos = -1, minPriority = 3;
    for (i = 0; i < MAX_UTHREADS; i++)
    {
        if (uthreads[i].state == RUNNABLE && uthreads[i].priority < minPriority)
        {
            minPos = i;
            minPriority = uthreads[i].priority;
        }
    }
    if (minPos == -1)
        return -1;

    struct uthread old;

    my_thread = &uthreads[minPos]; // we preformed this before the next line because after the next line the processor will run code of the next thread

    uswtch(&old.context, &my_thread->context);
    return 0;
}
struct uthread *uthread_self()
{
    return my_thread;
}
