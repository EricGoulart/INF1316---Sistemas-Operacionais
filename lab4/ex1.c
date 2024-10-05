//Eric Goulart da Cunha - 2110878
//Joâo Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include "stdlib.h"
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define NUM_THREADS 10
#define TAM 10000
#define CHUNK 1000


int a[TAM];
int soma = 0;

void * itera_vetor(void * threadid){
  int ind = (int) threadid;

  for(int j = 0; j < CHUNK; j++){
    a[(ind*CHUNK)+j] = a[(ind*CHUNK)+j] * 2;
    soma += a[(ind*CHUNK)+j];
  }

}


int main (int argc, char *argv[]){
  int segmento, id, pid, status;
  pthread_t threads[NUM_THREADS];

  for(int i = 0; i < 10000; i++){
    a[i] = 5;
  }
  int t;
  int i=0;
  for(t=0;t < NUM_THREADS;t++){
    printf("Creating thread %d\n", t);
    pthread_create(&threads[t], NULL, itera_vetor, (void *)t);
    i++;
  }

  for(t=0; t < NUM_THREADS; t++)
    pthread_join(threads[t],NULL); 


  printf("Soma de posições: %d",soma);

  return 0;
}
