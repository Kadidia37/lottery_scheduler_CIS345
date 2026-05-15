#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstat.h"

int
main(void)
{
  struct pstat st;
  int pid1, pid2, pid3;

  pid1 = fork();
  if(pid1 == 0){
    settickets(30);
    while(1){
      for(volatile int i = 0; i < 100000; i++);
    }
  }

  pid2 = fork();
  if(pid2 == 0){
    settickets(20);
    while(1){
      for(volatile int i = 0; i < 100000; i++);
    }
  }

  pid3 = fork();
  if(pid3 == 0){
    settickets(10);
    while(1){
      for(volatile int i = 0; i < 100000; i++);
    }
  }

  sleep(500);

  getpinfo(&st);

  printf("PID\tTickets\tTicks\n");
  for(int i = 0; i < NPROC; i++){
    if(st.inuse[i] && (st.pid[i] == pid1 || st.pid[i] == pid2 || st.pid[i] == pid3)){
      printf("%d\t%d\t%d\n", st.pid[i], st.tickets[i], st.ticks[i]);
    }
  }

  kill(pid1);
  kill(pid2);
  kill(pid3);

  wait(0);
  wait(0);
  wait(0);

  exit(0);
}




