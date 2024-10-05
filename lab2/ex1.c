//Eric Goulart da Cunha - 2110878
//Joâo Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include "stdlib.h"
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>


int main (int argc, char *argv[]){
  int segmento, id, pid, status, *a, segmento_soma, *soma;

  // aloca a memória compartilhada
  segmento = shmget (IPC_PRIVATE, sizeof (int*)*10000, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  segmento_soma= shmget (IPC_PRIVATE, sizeof (int*)*10, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  
  // associa a memória compartilhada ao processo
  a = (int *) shmat (segmento, 0, 0); // comparar o retorno com -1
  soma = (int *) shmat (segmento_soma, 0, 0);

  
  for(int i=0;i<10;i++){
    soma[i] = 0;
  }
  
  for(int i = 0; i < 10000; i++){
    *(a+i) = 5;
  }

  int i = 0;
  while (i < 10){
    if((id = fork()) < 0){
      puts ("Erro na criação do novo processo");
      exit (-2);
    }else if(id == 0){
      for(int j = 0; j < 1000; j++){
        a[(i*1000)+j] = a[(i*1000)+j] * 2;
        soma[i] += a[(i*1000)+j];
      }
      exit(0);
    }else{
      
    }
    i++;
  }
  
  for(int k = 0; k<10; k++){
    waitpid(-1, &status, 0);
  }
  int soma_posicoes = 0;
  for(int l =0; l<10; l++){
    soma_posicoes += soma[l];
  }

  printf("Soma de posições: %d",soma_posicoes);
  // libera a memória compartilhada do processo
  shmdt (a);
  shmdt(soma);
  // libera a memória compartilhada
  shmctl (segmento, IPC_RMID, 0);
  shmctl (segmento_soma, IPC_RMID, 0);
  return 0;
}