#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "qlock.h"
#include "sem.h"
#include "pstat.h"

extern struct proc proc[];


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_getyear(void) {
return 2026;
}

uint64
sys_getprocs(void) {
extern struct proc proc[];
int count = 0;
struct proc *p;
for(p = proc; p < &proc[NPROC]; p++) {
if(p->state == RUNNABLE || p->state == RUNNING) count++;
}
return count;
}

uint64
sys_sleep(void)
{
int n;
uint ticks0;
argint(0, &n);
acquire(&tickslock);
 ticks0 = ticks;
while(ticks - ticks0 < n) {
sleep (&ticks, &tickslock);
}
release(&tickslock);
return 0;
}

uint64
sys_qlock_init(void)
{
struct qlock *q = (struct qlock*)kalloc();
if(q == 0) return -1;
qlock_init(q, "qlock");
return (uint64)q;
}

uint64
sys_qlock_acquire(void)
{
uint64 addr;
argaddr(0, &addr);
qlock_acquire((struct qlock*)addr);
return 0;
}

uint64
sys_qlock_release(void)
{
uint64 addr;
argaddr(0, &addr);
qlock_release((struct qlock*)addr);
return 0;
}

uint64
sys_sem_init(void)
{
struct sem *s = (struct sem*)kalloc();
if(s == 0) return -1;
int count;
argint(0, &count);
sem_init(s, count);
return (uint64)s;
}

uint64
sys_sem_wait(void)
{
uint64 addr;
argaddr(0, &addr);
sem_wait((struct sem*)addr);
return 0;
}

uint64
sys_sem_post(void)
{
uint64 addr;
argaddr(0, &addr);
sem_post((struct sem*)addr);
return 0;
}

uint64
sys_settickets(void)
{
  int n;
argint(0, &n);
if(n < 1) return -1;
myproc()->tickets = n;
return 0;
}

uint64
sys_getpinfo(void)
{
  uint64 addr;
  argaddr(0, &addr);
  struct pstat ps;
  struct proc *p;
  int i = 0;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    ps.inuse[i] = (p->state != UNUSED);
    ps.tickets[i] = p->tickets;
    ps.pid[i] = p->pid;
    ps.ticks[i] = p->ticks;
    release(&p->lock);
    i++;
  }
  if(copyout(myproc()->pagetable, addr, (char*)&ps, sizeof(ps)) < 0)
    return -1;
  return 0;
}

