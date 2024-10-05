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
#define TAMANHO_BUFFER 8

typedef struct {
  char mensagens[TAMANHO_BUFFER][15];
  int in;
  int out;
  int count;
} Buffer;

int main() {
  int segmento, status;
  Buffer *buffer;
  sem_t *semaforo_vazio, *semaforo_cheio, *mutex;
  pid_t processo1, processo2;

  // aloca a memória compartilhada
  segmento = shmget(60, sizeof(Buffer), IPC_CREAT | 0666);
  if (segmento == -1) {
    fprintf(stderr, "Não foi possível alocar memória\n");
    exit(1);
  }

  // associa a memória compartilhada ao processo
  buffer = (Buffer *)shmat(segmento, 0, 0);
  buffer->in = 0;
  buffer->out = 0;
  buffer->count = 0;

  // Inicializa os semáforos
  semaforo_vazio = sem_open("semaforo_vazio", O_CREAT, 0666, TAMANHO_BUFFER);
  semaforo_cheio = sem_open("semaforo_cheio", O_CREAT, 0666, 0);
  mutex = sem_open("mutex", O_CREAT, 0666, 1);

  if (semaforo_vazio == SEM_FAILED || semaforo_cheio == SEM_FAILED || mutex == SEM_FAILED) {
    fprintf(stderr, "Erro ao criar semáforos\n");
    exit(1);
  }

  processo1 = fork();
  if (processo1 < 0) {
    printf("Erro na criacao do processo 1\n");
    exit(-1);
  }
  if (processo1 == 0) {
    char aux[15];
    for (int i = 1; i <= MAX_NUM; i++) {
      sprintf(aux, "mensagem %d", i);

      sem_wait(semaforo_vazio); // Decrementa o contador de espaços vazios
      sem_wait(mutex);          // Entra na região crítica

      // Escreve a mensagem no buffer
      strcpy(buffer->mensagens[buffer->in], aux);
      buffer->in = (buffer->in + 1) % TAMANHO_BUFFER;
      buffer->count++;
      printf("Processo 1: escrevendo %s\n", aux);

      sem_post(mutex);          // Sai da região crítica
      sem_post(semaforo_cheio); // Incrementa o contador de mensagens disponíveis

    }
    exit(0);
  }

  processo2 = fork();
  if (processo2 < 0) {
    printf("Erro na criacao do processo 2\n");
    exit(-1);
  }
  if (processo2 == 0) {
    char mensagem[15];
    for (int i = 1; i <= MAX_NUM; i++) {
      sem_wait(semaforo_cheio); // Decrementa o contador de mensagens disponíveis
      sem_wait(mutex);          // Entra na região crítica

      // Lê a mensagem do buffer
      strcpy(mensagem, buffer->mensagens[buffer->out]);
      buffer->out = (buffer->out + 1) % TAMANHO_BUFFER;
      buffer->count--;
      printf("Processo 2: lendo %s\n", mensagem);

      sem_post(mutex);          // Sai da região crítica
      sem_post(semaforo_vazio); // Incrementa o contador de espaços vazios

    }
    exit(0);
  }

  for (int i = 0; i < 2; i++) {
    wait(&status);
  }

  shmdt(buffer);
  shmctl(segmento, IPC_RMID, NULL);

  sem_unlink("semaforo_vazio");
  sem_unlink("semaforo_cheio");
  sem_unlink("mutex");
  sem_close(semaforo_vazio);
  sem_close(semaforo_cheio);
  sem_close(mutex);

  return 0;
}
