#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct proc proc[NPROC];

void kthreadinit(struct proc *p)
{
  // Given a proc, it initializes the lock in charge of thread ID allocation
  initlock(&p->tid_lock, "tid");
  // p->tid_counter = 1;

  for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++) // for every kernel thread
  {
    // init lock
    initlock(&kt->thread_lock, "thread");
    // init state to unused
    kt->thread_state = T_UNUSED;
    // reference to the belong proc
    kt->my_proc = p;

    // WARNING: Don't change this line!
    // get the pointer to the kernel stack of the kthread
    kt->kstack = KSTACK((int)((p - proc) * NKT + (kt - p->kthread)));
  }
}

struct kthread *mykthread()
{
  push_off();
  struct cpu *c = mycpu();
  struct kthread *kt = c->thread;
  pop_off();
  return kt;
}

struct trapframe *get_kthread_trapframe(struct proc *p, struct kthread *kt)
{
  return p->base_trapframes + ((int)(kt - p->kthread));
}

// // TODO: delte this after you are done with task 2.2
// void allocproc_help_function(struct proc *p)
// {
//   p->kthread->trapframe = get_kthread_trapframe(p, p->kthread);

//   p->context.sp = p->kthread->kstack + PGSIZE;
// }
int alloctid(struct proc *p)
{
  int tid;
  acquire(&p->tid_lock);
  tid = p->tid_counter;
  p->tid_counter++;
  release(&p->tid_lock);

  return tid;
}
struct kthread *
alloc_kthread(struct proc *p)
{
  struct kthread *kt;
  for (kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    acquire(&kt->thread_lock);
    if (kt->thread_state == T_UNUSED)
    {
      goto found;
    }
    else
    {
      release(&kt->thread_lock);
    }
  }
  return 0;

found:
  kt->tid = alloctid(p);
  kt->thread_state = T_USED;
  // Allocate a trapframe page.
  if ((kt->trapframe = get_kthread_trapframe(p, kt)) == 0) // check!
  {
    free_kthread(kt);
    release(&kt->thread_lock);
    return 0;
  }
  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&kt->thread_context, 0, sizeof(kt->thread_context));
  kt->thread_context.ra = (uint64)forkret;
  kt->thread_context.sp = kt->kstack + PGSIZE; // i assume this is the size of the kstack, its seems that it should be 4000.

  return kt;
}

// free a proc structure and the data hanging from it,
// including user pages.
// kt->thread_lock must be held.
void free_kthread(struct kthread *kt)
{
  // if (kt->trapframe)
  //   kfree((void *)kt->trapframe);
  // kt->trapframe = 0;
  kt->tid = 0;
  // kt->name[0] = 0; could be done in the future
  kt->thread_chan = 0;
  kt->thread_killed = 0;
  kt->thread_xstate = 0;
  kt->thread_state = T_UNUSED;
  // should not free the stack!
}

int kthread_create(void *(*start_func)(), void *stack, uint stack_size)
{
  int tid;
  struct kthread *nkt; // new proc
  struct proc *p = myproc();
  // struct kthread *kt = mykthread(); // calling thread

  // Allocate process.
  if ((nkt = alloc_kthread(p)) == 0)
  {
    return -1;
  }
  // change epc
  nkt->trapframe->epc = (uint64)start_func; // this is unknown

  // copy context
  // nkt->thread_context = kt->thread_context;

  // set to RUNNABLE
  nkt->thread_state = T_RUNNABLE;

  // sp
  nkt->trapframe->sp = (uint64)stack + (uint64)stack_size; // user stack pointer TODO: maybe need -1

  tid = nkt->tid;

  release(&nkt->thread_lock);

  return tid;
}

int kthread_id(void)
{
  return mykthread()->tid;
}
int kthread_kill(int tid) // maybe need to add p lock
{
  struct proc *p = myproc();
  struct kthread *kt;
  for (kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    acquire(&kt->thread_lock);
    if (kt->tid == tid && kt->thread_state != T_UNUSED && kt->thread_state != T_ZOMBIE)
    {
      kt->thread_killed = 1;

      if (kt->thread_state == T_SLEEPING)
      {
        kt->thread_state = T_RUNNABLE;
      }

      release(&kt->thread_lock);
      return 0;
    }
    release(&kt->thread_lock);
  }
  return -1;
}
void kthread_exit(int status) // bug bug we need to lock thread
{
  struct proc *p = myproc();
  struct kthread *kt;
  acquire(&p->proc_lock);
  mykthread()->thread_state = T_ZOMBIE;
  mykthread()->thread_xstate = status;
  int last = 1;

  for (kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {

    if ((kt->thread_state != T_ZOMBIE) && (kt->thread_state != T_UNUSED) && (kt->thread_state != T_USED)) // added used
    {
      last = 0;
      break;
    }
  }
  release(&p->proc_lock);

  if (last)
  {
    exit(status);
    return;
  }
  acquire(&mykthread()->thread_lock);
  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

int kthread_join(int tid, int *status) // check if killed
{
  struct proc *p = myproc();
  struct kthread *kt;
  struct kthread *waiting_thread = 0;
  // waiting_thread = &p->kthread[tid];

  for (kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    if (kt->tid == tid)
      waiting_thread = kt;
  }
  if (!waiting_thread)
  {
    printf("kthread_join :no thread Id exist\n");
    return -1;
  }

  for (;;)
  {

    // for (kt = p->kthread; kt < &p->kthread[NKT];kt++)
    // {
    //   acquire(&kt->thread_lock);

    //   if(kt->tid==tid)
    //   {

    //   }
    // }

    acquire(&waiting_thread->thread_lock);

    if (waiting_thread->thread_state == T_ZOMBIE)
    {
      if (status != 0 && copyout(p->pagetable, (uint64)status, (char *)&waiting_thread->thread_xstate,
                                 sizeof(waiting_thread->thread_xstate)) < 0)
      {
        release(&waiting_thread->thread_lock);
        return -1;
      }
      free_kthread(waiting_thread); // we added so only if other thread join to tid he will be free (like wait)
      release(&waiting_thread->thread_lock);
      return 0;
    }
    release(&waiting_thread->thread_lock);
  }
}