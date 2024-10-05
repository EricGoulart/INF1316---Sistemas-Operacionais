//Eric Goulart da Cunha - 2110878
//Joâo Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include "stdlib.h"
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#define TRUE 1
#define FALSE 0

int main (int argc, char *argv[]){
  int segmento, id, pid, status, *a;

  // aloca a memória compartilhada
  segmento = shmget (IPC_PRIVATE, sizeof (int*)*10000, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  


  // associa a memória compartilhada ao processo
  a = (int *) shmat (segmento, 0, 0); // comparar o retorno com -1
  
  int is_equal = TRUE;

  
  for(int i = 0; i < 10000; i++){
    *(a+i) = 5;
  }
  int temp;
  int i = 0;
  int verificador =5;
  while (i < 2){
    if((id = fork()) < 0){
      puts ("Erro na criação do novo processo");
      exit (-2);
    }else if(id == 0){
      for(int j = 0; j < 10000; j++){
        temp = a[j]*2+2;
        a[j] = temp;
      }
      exit(0);
    }else{
        for(int l = 0; l < 10000; l++){
          if(a[l] != verificador){
            printf("Número diferente: %d -- verificador = %d índice: %d Iteração %d\n", a[l],verificador, l, i+1);
            verificador = a[l];
          }
        }
        waitpid(-1, &status, 0);
    }
    i++;
  }
  
  // libera a memória compartilhada do processo
  shmdt (a);

  // libera a memória compartilhada
  shmctl (segmento, IPC_RMID, 0);
  
  return 0;
}
