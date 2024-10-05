//Eric Goulart - 2110878
//João Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "stdlib.h"


#define TRUE 0
#define FALSE 1

int main(void) {
  int pid;
  int status;
  
    
    int n = 3;
    int i;
    // Resposta: Área de memória diferentes criadas no fork(). Com isso, duas variáveis n são
    // Criadas, para cada programa. Assim, ao printar as variáveis, a saída será diferente.
    if((pid = fork())!=0){
      // Parent code!
      for(i =0; i<10000;i++){
        n++;
      }
      printf("processo pai, pid=%d, n=%d\n", getpid(), n);
      waitpid(-1,&status,0);
    }else{
      // Child code
      for(i=0; i< 10000; i++){
        n+=10;
      }
      printf("processo filho, pid=%d, n=%d\n", getpid(), n);
    }
  return 0;
}