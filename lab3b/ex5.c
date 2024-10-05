#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

void handler(int signum);

int main(int argc, char* argv[]) {

  int filho1, filho2;

  if ((filho1 = fork()) < 0){
    fprintf(stderr, "Erro ao criar filho\n");
    exit(-1);
  }if (filho1 == 0){ /* child */
    execvp(argv[1],argv);
  }else{
    //codigo pai
    if ((filho2 = fork()) < 0){
      fprintf(stderr, "Erro ao criar filho\n");
      exit(-1);
    }if(filho2 == 0){ /* child */
      execvp(argv[1],argv);
    }
  }

  printf("1 - Pausando o processo pid %d\n", filho1);
  kill(filho1, SIGSTOP);
  printf("2 - Pausando o processo pid %d\n", filho2);
  kill(filho2,SIGSTOP);

  int flag = 0;
  for(int i = 0; i<15;i++){
    printf("\n------Iteracao %d------\n", i+1);
    if(flag == 0){
      printf("Continuando o processo de pid %d\n", filho1);
      kill(filho1, SIGCONT);
      sleep(1);
      printf("Pausando o processo pid %d\n", filho1);
      kill(filho1, SIGSTOP);
      flag = 1;
    }else if(flag == 1){
      printf("Continuando o processo pid %d\n", filho2);
      kill(filho2, SIGCONT);
      sleep(1);  
      printf("Pausando o processo pid %d\n", filho2);
      kill(filho2, SIGSTOP);
      flag = 0;
    }
  }
  printf("Encerrando os 2 programas\n");
  kill(filho1, SIGKILL);
  kill(filho2, SIGKILL);


  return 0;
}
