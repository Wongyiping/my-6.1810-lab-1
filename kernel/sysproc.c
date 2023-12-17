#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
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
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
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


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 fva, uva; //这里fva需要对齐吗
  int num;
  uint64 abits = 0; //结果
  
  argaddr(0, &fva);
  argint(1, &num);
  argaddr(2, &uva);
  
  if(num < 0 || num > 64) return -1;
  //思考：找到当前页表地址
  pagetable_t pagetable = myproc()->pagetable;
  uint64 va = fva; 
  for(int i = 0; i < num; i++, va += PGSIZE){
    //依次访问虚拟地址
    pte_t *pte = walk(pagetable, va, 0); //获得叶子
    if(pte == 0) return -1;
    if(*pte & PTE_A){
      abits |= (1 << i); // 记录位
      *pte = (*pte) & (~PTE_A);
    }
  }
  if(copyout(pagetable, uva, (char*)&abits, sizeof(abits))<0) return -1; ///
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
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
