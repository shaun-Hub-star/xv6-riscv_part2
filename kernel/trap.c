#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;
extern int swapPages(pagetable_t pagetable, uint64 hardisk, uint64 memory, int swap);
extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

int get_nfua_index(struct proc *p)
{
  int min_index = -1;
  uint64 value = -1;
  for (int i = 0; i < MAX_PSYC_PAGES; i++)
  {
    if (p->physical_pages[i].counter < value)
    {
      min_index = i;
      value = p->physical_pages[i].counter;
    }
  }
  return min_index;
}
int num_of_ones(uint64 num)
{
  uint64 i = 0x8000000000000000;
  int counter = 0;
  while (i > 0)
  {
    if (i & num)
      counter++;
  }
  return counter;
}

int get_lapa_index(struct proc *p)
{

  uint min_value_ones = 65;
  int min_index = -1;
  uint64 min_value = -1;
  for (int i = 0; i < MAX_PSYC_PAGES; i++)
  {
    int counter_ones = num_of_ones(p->physical_pages[i].age);
    if (counter_ones < min_value_ones)
    {
      min_value_ones = counter_ones;
    }
  }

  for (int i = 0; i < MAX_PSYC_PAGES; i++)
  {
    int counter_ones = num_of_ones(p->physical_pages[i].age);
    if (counter_ones == min_value_ones && min_value > p->physical_pages[i].age)
    {
      min_index = i;
      min_value = p->physical_pages[i].age;
    }
  }

  return min_index;
}

int get_scfifo_index(struct proc *p)
{
  int min_index = -1;
  uint64 value = -1;
  while (1)
  {
    min_index = -1;
    value = -1;
    for (int i = 0; i < MAX_PSYC_PAGES; i++)
    {
      if (p->physical_pages[i].age < value)
      {
        min_index = i;
        value = p->physical_pages[i].age;
      }
    }
    pte_t *pte_min = walk(p->pagetable, p->physical_pages[min_index].virtual_address, 0);
    if (pte_min == 0)
      panic("get_scfifo_index");
    if (*pte_min & PTE_A)
    {
      *pte_min &= ~PTE_A;
      p->physical_pages[min_index].age = p->global_age;
      p->global_age++;
    }
    else
      break;
  }
  return min_index;
}

int get_physical_page_index(struct proc *p)
{
  return 1;
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void usertrap(void)
{
  int which_dev = 0;

  if ((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();

  // save user program counter.
  p->trapframe->epc = r_sepc();

  if (r_scause() == 13 || r_scause() == 15)
  {

    //  page fault
    uint64 va_hardisk = r_stval();
    pte_t *pte = walk(p->pagetable, va_hardisk, 0);
    if (*pte & PTE_PG)
    {
      printf("page fault\n");
      // access the va address of
      if (p->counter_physical_memory >= MAX_PSYC_PAGES)
      {
        for (int i = 0; i < MAX_PSYC_PAGES; i++)
        {
          if (p->physical_pages[i].status == ACTIVE) // put here the algorithm for page swaping in the future
          {
            swapPages(p->pagetable, va_hardisk, p->physical_pages[i].virtual_address, 1);
            break;
          }
        }
      }
      else
      {

        swapPages(p->pagetable, va_hardisk, 0, 0);
      }
    }
    else
    {
      panic("usertrap: page fault");
    }
    sfence_vma();
    if (killed(p))
      exit(-1);
    usertrapret();
    return;
  }

  if (r_scause() == 8)
  {
    // system call

    if (killed(p))
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sepc, scause, and sstatus,
    // so enable only now that we're done with those registers.
    intr_on();

    syscall();
  }
  else if ((which_dev = devintr()) != 0)
  {
    // ok
  }
  else
  {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    setkilled(p);
  }

  if (killed(p))
    exit(-1);

  // give up the CPU if this is a timer interrupt.
  if (which_dev == 2)
    yield();

  usertrapret();
}

//
// return to user space
//
void usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to uservec in trampoline.S
  uint64 trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
  w_stvec(trampoline_uservec);

  // set up trapframe values that uservec will need when
  // the process next traps into the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp(); // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.

  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to userret in trampoline.S at the top of memory, which
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64))trampoline_userret)(satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  if ((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if (intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if ((which_dev = devintr()) == 0)
  {
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void clockintr()
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int devintr()
{
  uint64 scause = r_scause();

  if ((scause & 0x8000000000000000L) &&
      (scause & 0xff) == 9)
  {
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if (irq == UART0_IRQ)
    {
      uartintr();
    }
    else if (irq == VIRTIO0_IRQ)
    {
      virtio_disk_intr();
    }
    else if (irq)
    {
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if (irq)
      plic_complete(irq);

    return 1;
  }
  else if (scause == 0x8000000000000001L)
  {
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if (cpuid() == 0)
    {
      clockintr();
    }

    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  }
  else
  {
    return 0;
  }
}
