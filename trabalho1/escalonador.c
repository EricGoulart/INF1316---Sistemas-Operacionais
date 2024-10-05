// Eric Goulart da Cunha - 2110878
// Joâo Pedro Biscaia Fernandes - 2110361

#include <signal.h> // Incluir para o uso de sinais
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0
#define MAX_PROCESSOS 10

#define COMPLETED 5
#define RUNNING 6
#define WAITING 7
#define PAUSED 8

typedef struct processo {
  char nome[15];
  int prioridade;
  int inicio;
  int duracao;
} Processo;

typedef struct processo_exec {
  pid_t pid;
  int running;
} Processo_exec;

typedef struct fila{
  int status;
  int started_time;
  pid_t pid;
  Processo process;
  struct fila * prox;
}Fila;


Fila * pop_fila(Fila * queue);
Fila * push_fila(Processo val, Fila * queue, pid_t pid, int status);
Fila * cria_fila();
int pode_entrar_realtime(Fila * atual, Processo inserido);
void ordenarPorPrioridade(Processo *processos, int tamanho);
void imprime(Fila * lst);
int realtime_pronto(Fila * ready, int sec);
int realtime_finished(Fila * ready, int sec);
int realtime_quer_comecar(Fila *ready, int sec);
Fila * coloca_realtime_comeco(Fila * ready, int sec, Fila * realtime);
Fila * preenche_real_time(Fila * q);
Fila * preenche_principal(Fila* q);
Fila * adiciona_realtime_denovo(Fila * realtime);
Fila * trata_waiting(Fila *ready, float print_sec, int sec, Fila*fila_realtime);


int main(void) {

  int segmento,status;
  Processo *vetor_programas;
  Processo_exec processo_exec[MAX_PROCESSOS];

  // aloca a memória compartilhada
  segmento = shmget(80, sizeof(Processo)*MAX_PROCESSOS, IPC_CREAT| 0666);

  if (segmento == -1) {
    fprintf(stderr, "Não foi possível alocar memória\n");
    exit(1);
  }
  // associa a memória compartilhada ao processo
  vetor_programas = (Processo *)shmat(segmento, 0, 0); // comparar o retorno com -1

  Fila *ready = NULL;
  Fila *fila_realtime = NULL;
  
  printf("\n\n--------------------Escalonador--------------------\n\n");


  struct timeval start_time, current_time;
  gettimeofday(&start_time, NULL);

  // Inicializa os processos
  int i = 0;

  while( i < MAX_PROCESSOS && (strcmp(vetor_programas[i].nome,"")!=0)) {
    if ((processo_exec[i].pid = fork()) < 0) {
      fprintf(stderr, "Erro ao criar filho");
      exit(1);
    }
    if (processo_exec[i].pid == 0) {
      /*inicializa a execução do programa filho, se puder ser executado*/
      char comando[15] = "./";
      char *args[] = {strcat(comando,vetor_programas[i].nome), NULL}; // Argumentos do programa
      execvp(args[0], args); // Execução do programa
      exit(EXIT_SUCCESS);
    } else { /*Processo pai*/
      kill(processo_exec[i].pid, SIGSTOP);   
    }
    i++;
  }

  ordenarPorPrioridade(vetor_programas, MAX_PROCESSOS);

  for(int i = 0; i< MAX_PROCESSOS && (strcmp(vetor_programas[i].nome,"")!=0); i++){
    if(vetor_programas[i].inicio == -1){
      ready = push_fila(vetor_programas[i],ready, processo_exec[i].pid, WAITING);
    }else{
      fila_realtime = push_fila(vetor_programas[i],fila_realtime, processo_exec[i].pid, WAITING);
    }
  }



  float sec, print_sec;
  while (TRUE) {
    gettimeofday(&current_time, NULL);
    sec = ((current_time.tv_sec - start_time.tv_sec));
    print_sec = (current_time.tv_sec - start_time.tv_sec) % 60;
    if(ready->status == RUNNING){
      if(realtime_finished(ready, print_sec) == TRUE){
        ready->status = COMPLETED;
      }else if(ready->process.prioridade > -1){
        if(sec - ready->started_time >=5){
          kill(ready->pid, SIGSTOP);
          ready->status = COMPLETED;
        }
      }else if(ready->process.prioridade == -1 && ready->process.inicio == -1){
        if(sec - ready->started_time >= 1){
          ready->status = COMPLETED;
          kill(ready->pid, SIGSTOP);
        }
      }
      if(realtime_quer_comecar(fila_realtime, print_sec) == TRUE && ready->process.inicio == -1){
        ready = coloca_realtime_comeco(ready, print_sec, fila_realtime);
        fila_realtime->status = RUNNING;
        kill(ready->pid, SIGCONT);
        ready->status = RUNNING;
      }
    }
    if(ready->status == COMPLETED){
      if(ready->process.prioridade > -1){
        ready = pop_fila(ready);
      }else{
        if(ready->process.inicio == -1){
          ready = push_fila(ready->process,ready, ready->pid, WAITING);
        }else if(ready->process.inicio > -1){
          ready->status = WAITING;
          fila_realtime->status = WAITING;
          fila_realtime = adiciona_realtime_denovo(fila_realtime);
        }
        ready = pop_fila(ready);
      }
      if(realtime_quer_comecar(fila_realtime, print_sec) == TRUE && ready->process.inicio == -1){
        ready = coloca_realtime_comeco(ready, print_sec, fila_realtime);
        fila_realtime->status = RUNNING;
        kill(ready->pid, SIGCONT);
        ready->status = RUNNING;
      }
      
    }
    if(ready->status == WAITING){
      ready = trata_waiting(ready, print_sec, sec, fila_realtime);
    }
    if(ready->status == PAUSED){
      kill(ready->pid, SIGCONT);
      ready->status = RUNNING;
    }
    printf("%.1f %s\n",print_sec + 1,ready->process.nome);
    
    sleep(1);
  }


  shmdt(vetor_programas);

  // libera a memória compartilhada
  shmctl(segmento, IPC_RMID, 0);

  return 0;
}

int pode_entrar_realtime(Fila * atual, Processo inserido){
  int final_atual = atual->process.inicio + atual->process.duracao;
  int final_scheduled = inserido.inicio + inserido.duracao -1;
  if(inserido.inicio > -1 && atual->process.inicio > -1){
    if (inserido.inicio < (final_atual) && (final_scheduled >= atual->process.inicio)){
      return FALSE;
    }
  }
  return TRUE;
}

Fila * cria_fila(){
  Fila *new_queue = (Fila*) malloc(sizeof(Fila));
  new_queue->prox = NULL;
  return new_queue;
}

Fila * push_fila(Processo val, Fila * queue, pid_t pid, int status){
  Fila * novo_val = (Fila*) malloc(sizeof(Fila));
  if(novo_val==NULL){
    printf("Falha ao alocar memória, retornando NULL");
    return NULL;
  }
  novo_val->process = val;
  novo_val->pid = pid;
  novo_val->status = status;
  novo_val->prox=NULL;
  if(queue==NULL){
    return novo_val;
  }
  Fila * aux = queue;
  Fila * ant = NULL;
  while(aux->prox!=NULL){
    if(pode_entrar_realtime(aux, val) == FALSE) return queue;
    ant = aux;
    aux = aux->prox;
  }
  if(pode_entrar_realtime(aux, val) == FALSE) return queue;
  if(ant==NULL){
    queue->prox = novo_val;
    return queue;
  }
  aux->prox = novo_val;
  return queue;
}


Fila * pop_fila(Fila * queue){
  if(!queue){
    printf("Fila vazia\n");
    return NULL;
  }
  Fila * aux = queue;
  queue = queue->prox;
  free(aux);
  return queue;

}

void imprime(Fila * lst){
  int quant = 0;
  printf("\n----Print da Fila Atualmente:----\n");
  if(lst==NULL){
    printf("Fila Vazia!\n\n");
    return;
  }
  for(Fila * aux = lst;aux != NULL; aux = aux->prox){
    printf("%s\n", aux->process.nome);
    quant++;
  }
  printf("Quantidade de itens na fila: %d\n\n",quant);
}

// Função de ordenação por prioridade
void ordenarPorPrioridade(Processo *processos, int tamanho) {
    int i, j;
    Processo chave;

    for (i = 1; i < tamanho && (strcmp(processos[i].nome,"")!=0); i++) {
        chave = processos[i];
        j = i - 1;

        // Move os elementos do vetor que têm prioridade maior que a chave para uma posição à frente de sua posição atual
        while (j >= 0 && processos[j].prioridade > chave.prioridade) {
            processos[j + 1] = processos[j];
            j = j - 1;
        }

        // Insere a chave na posição correta
        processos[j + 1] = chave;
    }
}

int realtime_pronto(Fila * ready, int sec){
  if(ready->process.inicio>-1){
    if(sec == ready->process.inicio){
      return TRUE;
    }
  }
  return FALSE;
}

int realtime_finished(Fila * ready, int sec){
  if(ready->process.inicio>-1){
    if(ready->process.duracao <= (sec - ready->started_time)){
      return TRUE;
    }
  }
  return FALSE;
}
int realtime_quer_comecar(Fila *ready, int sec){
  Fila * aux = ready;
  if(aux == NULL){
    printf("Fila vazia\n");
    return FALSE;
  }
  while(aux->prox != NULL){
    if(aux->process.inicio>-1){
      int fim = aux->process.inicio + aux->process.duracao -1;
      if( sec >= (aux->process.inicio-1 ) && sec <= fim) return TRUE;
    }
    aux = aux->prox;
  }

  return FALSE;
}

Fila * coloca_realtime_comeco(Fila * ready, int sec, Fila * realtime){
  Fila *aux_realtime = realtime;
  Fila * temp = ready;
  Fila *substitute = (Fila*) malloc(sizeof(Fila));
  
  while(aux_realtime->prox != NULL){
    if(aux_realtime->process.inicio>-1 && aux_realtime->status == WAITING){
      int fim = aux_realtime->process.inicio + aux_realtime->process.duracao -1;
      if( sec >= aux_realtime->process.inicio-1 && sec <= fim){
        substitute->pid = aux_realtime->pid;
        substitute->process = aux_realtime->process;
        substitute->status = aux_realtime->status;
        substitute->started_time = sec;
        temp->status = PAUSED;
        kill(temp->pid,SIGSTOP);
        substitute->prox = temp;
        return substitute;
      }
    }
  }
  return ready;
}

Fila * preenche_real_time(Fila * q){
  Fila * retorno;
  Fila * aux = q;
  while(aux != NULL){
    if(q->process.inicio > -1){
      retorno = push_fila(aux->process,retorno, aux->pid, WAITING);
    }
    aux = aux->prox;
  }
  return retorno;
}

Fila * preenche_principal(Fila* q){
  Fila * retorno;
  Fila * aux = q;
  while(aux != NULL){
    if(q->process.inicio == -1){
      retorno = push_fila(q->process,retorno, q->pid, WAITING);
    }
    aux = aux->prox;
  }
  return retorno;
}
Fila * adiciona_realtime_denovo(Fila * realtime){
  Fila * aux = (Fila*) malloc(sizeof(Fila));
  aux->status = WAITING;
  aux->process = realtime->process;
  aux->pid = realtime->pid;
  aux->prox = NULL;
  realtime = pop_fila(realtime);
  realtime = push_fila(aux->process, realtime, aux->pid, WAITING);
  return realtime;
}

Fila * trata_waiting(Fila *ready, float print_sec, int sec, Fila*fila_realtime){
  if(realtime_pronto(ready, (int) print_sec ) == TRUE){
    kill(ready->pid, SIGCONT);
    ready->started_time = sec;
  }else if(ready->process.prioridade > -1){
    int can_start_priority = atoi(&ready->process.nome[1]) - 1;
    if(sec >= can_start_priority){  
      kill(ready->pid, SIGCONT);
      ready->started_time = sec;
    }else{
      while(ready->process.prioridade > -1 && sec < (atoi(&ready->process.nome[1]))){
        ready = push_fila(ready->process, ready, ready->pid, WAITING);
        ready = pop_fila(ready);
      }
    }
  }
  
  if(ready->process.prioridade == -1 && ready->process.inicio == - 1){/*Round Robin*/
    kill(ready->pid, SIGCONT);
    ready->started_time = sec;
  }if(realtime_quer_comecar(fila_realtime, sec)==TRUE){
    ready = coloca_realtime_comeco(ready, sec, fila_realtime);
    fila_realtime->status = RUNNING;
    kill(ready->pid, SIGCONT);
    ready->status = RUNNING;
    ready->started_time = sec;
  }
  ready->status = RUNNING;
  ready->started_time = sec;

  return ready;
}
