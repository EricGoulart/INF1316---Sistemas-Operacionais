// Eric Goulart da Cunha - 2110878
// Joâo Pedro Biscaia Fernandes - 2110361


#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0
#define MAX_PROCESSOS 10

#define REAL_TIME 3
#define PRIORIDADE 2
#define ROUND_ROBIN 1


typedef struct processo {
  char nome[15];
  int prioridade;
  int inicio;
  int duracao;
} Processo;

FILE *open_file(char *filename, char *mode);
Processo monta_struct_comando(char **comando);
char **split_commands(char *line, char *separator);

int main(int argc, char *argv[]) {
  int segmento;
  Processo *vetor_programas;
  FILE *arq = fopen("exec.txt", "r");
  char *line = (char *)malloc(sizeof(char) * 20);

  // aloca a memória compartilhada
  segmento =
      shmget(80, sizeof(Processo) * MAX_PROCESSOS, IPC_CREAT | 0666);

  // associa a memória compartilhada ao processo
  vetor_programas = (Processo *)shmat(segmento, 0, 0); // comparar o retorno com -1
  if(vetor_programas == -1){
    fprintf(stderr, "Não foi possível alocar memória\n");
    exit(1);
  }


  int n = 0;
  while (fgets(line, 20, arq)) {
    printf("%s\n", line);
    char ** command ;
    command = split_commands(line, " ");
    vetor_programas[n] = monta_struct_comando(command);
    n++;

    sleep(1);
  }

  for(; n<MAX_PROCESSOS;n++){
    strcpy(vetor_programas[n].nome,"");
    vetor_programas[n].prioridade = -1;
    vetor_programas[n].inicio = -1;
    vetor_programas[n].duracao = -1;
    
  }


  char * comando = "./escalonador";
  char *args[] = {comando, NULL};
  shmdt(vetor_programas);


  execvp(args[0],args);

  // libera a memória compartilhada
  // shmctl(segmento, IPC_RMID, 0);


  return 0;
}

FILE *open_file(char *filename, char *mode) {
  FILE *f;
  f = fopen(filename, mode);
  if (!f) {
    perror(filename);
    exit(EXIT_FAILURE);
  }
  return f;
}

char **split_commands(char *line, char * separator) {
  char **params = (char **)malloc(10 * sizeof(char *));
  if (!params) {
    printf("Erro ao alocar memoria");
    exit(1);
  }
  char *word;
  int pos = 0;

  word = strtok(line, separator);
  while (word != NULL) {
    params[pos] = malloc(20 * sizeof(char));
    strcpy(params[pos], word);
    pos++;

    word = strtok(NULL, separator);
  }
  params[pos] = NULL;
  return params;
}

Processo monta_struct_comando(char **comando){
    Processo buffer;
    int len_comando = strlen(comando[1]);
    if(comando[1][len_comando-1] == '\n'){
      comando[1][len_comando-1] = '\0';
      strcpy(buffer.nome, comando[1]);
    }else{
      strcpy(buffer.nome, comando[1]);
    }
    buffer.inicio = -1;
    buffer.duracao = -1;
    buffer.prioridade = -1;
    int i;
    for(i = 1; i < 4; i++){
      if(comando[i+1] == NULL){
        break;
      }else if(i==3)break;
    }

    switch(i){
        case ROUND_ROBIN:
            break;
        case PRIORIDADE:
            char ** prioridade = split_commands(comando[2], "=");
            buffer.prioridade = atoi(prioridade[1]);
            break;
        case REAL_TIME:
            char ** inicio = split_commands(comando[2], "=");
            char ** duracao = split_commands(comando[3], "=");
            buffer.inicio  = atoi(inicio[1]);
            buffer.duracao  = atoi(duracao[1]);
            break;
        default:
            printf("Comando inválido. Saindo do interpretador.\n");
            printf("%d\n",i);
            exit(EXIT_FAILURE);
            break;
    }

    return buffer;

}