//Aluno: Eric Goulart da Cunha; Matrícula: 2110878
//Aluno: João Pedro Biscaia Fernandes; Matrícula: 2110361

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define MAX_NUM 128

int main() {
  int segmento, status;
  char *mensagem;
  sem_t *semaforo_escrita, *semaforo_leitura;
  pid_t processo1, processo2;

  // aloca a memória compartilhada
  segmento = shmget(IPC_PRIVATE, sizeof(char) * 15, IPC_CREAT | 0666);
  if (segmento == -1) {
    fprintf(stderr, "Não foi possível alocar memória\n");
    exit(1);
  }
  // associa a memória compartilhada ao processo
  mensagem = (char *)shmat(segmento, 0, 0);

  semaforo_escrita = sem_open("semaforo_escrita", O_CREAT, 0666, 0);
  semaforo_leitura = sem_open("semaforo_leitura", O_CREAT, 0666, 0);

  sem_init(semaforo_escrita, 1, 1);
  sem_init(semaforo_leitura, 1, 0);

  processo1 = fork();
  if (processo1 < 0) {
    printf("Erro na criacao do processo 1\n");
    exit(-1);
  }
  if (processo1 == 0) {
    char aux[15];
    for(int i = 1; i <= MAX_NUM; i++) {
      sem_wait(semaforo_escrita);
      sprintf(aux, "mensagem %d", i);
      strcpy(mensagem, aux);
      printf("escrevendo %s\n", mensagem);
      sem_post(semaforo_leitura);
    }
    exit(0);
  }

  processo2 = fork();
  if (processo2 < 0) {
    printf("Erro na criacao do processo 2\n");
    exit(-1);
  }
  if (processo2 == 0) {
    for(int i = 1; i <= MAX_NUM; i++) {
      sem_wait(semaforo_leitura);
      printf("lendo %s\n", mensagem);
      sem_post(semaforo_escrita);
    }
    exit(0);
  }

  for(int i = 0; i < 2; i++) {
    wait(&status);
  }

  shmdt(mensagem);
  sem_unlink("semaforo_escrita");
  sem_unlink("semaforo_leitura");
  shmctl(segmento, IPC_RMID, NULL);

  return 0;
}
