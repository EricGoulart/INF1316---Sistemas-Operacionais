// Eric Goulart da Cunha - 2110878
// João Pedro Biscaia Fernandes - 2110361
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>

#define LINE_SIZE 10
#define TABLE_SIZE 16
#define SEM_P1 "/sem_p1"
#define SEM_P2 "/sem_p2"
#define SEM_P3 "/sem_p3"
#define SEM_MUTEX "/sem_mutex"
#define TRUE 1
#define FALSE 0 

typedef struct {
    int pagina_virtual;
    int BV; //NRU - Bit de Vailidacao
    int BM; //NRU - Bit de modificacao
    int BP; //Segunda Chance - Bit de protecao
    int process_id;
    int age;  // Adicionado para rastrear as iterações
    int tempo; // Working Set
} Pagina;

typedef struct {
    Pagina entries[TABLE_SIZE];
    int index;
    int page_faults;
} Tabela;

int segmento;
Tabela* tabela;
sem_t *sem_p1;
sem_t *sem_p2;
sem_t *sem_p3;
sem_t *mutex;

void print_table(Tabela *tabela);
void read_and_print_file_line_by_line(const char *filename, char *nome_processo, sem_t *sem_current, sem_t *sem_next, Tabela *tabela, sem_t *mutex, int process_id, const char *algoritmo, int rodadas, int window_size);
Tabela* trocador_de_paginas(Tabela *tabela, int pagina_virtual, int process_id, char modo, const char *algoritmo, int window_size);
int troca_NRU(Pagina *tab_paginas, Pagina entrada, int *substituido_id);
void age_pages(Tabela *tabela);
int troca_LRU(Pagina *tab_paginas, Pagina entrada, int *substituido_id);
int troca_second_chance(Pagina* tab_paginas, Pagina entrada, int *substituido_id);
int troca_working_set(Pagina *tab_paginas, Pagina entrada, int window_size, int *substituido_id);

void cleanup(int signum) {
    printf("Cleaning up resources...\n");

    // Close semaphores
    sem_close(sem_p1);
    sem_close(sem_p2);
    sem_close(sem_p3);
    sem_close(mutex);

    // Unlink semaphores
    sem_unlink(SEM_P1);
    sem_unlink(SEM_P2);
    sem_unlink(SEM_P3);
    sem_unlink(SEM_MUTEX);

    // Detach shared memory
    shmdt(tabela);

    // Remove shared memory
    shmctl(segmento, IPC_RMID, 0);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc < 3 || (strcmp(argv[1], "WORKING_SET") == 0 && argc != 4)) {
        fprintf(stderr, "Uso: %s <algoritmo> <rodadas> [window_size]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *algoritmo = argv[1];
    int rodadas = atoi(argv[2]);
    int window_size = 0;

    if (strcmp(algoritmo, "WORKING_SET") == 0) {
        window_size = atoi(argv[3]);
        printf("%d\n", window_size);
    }

    // Register cleanup handler for signals
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    sem_p1 = sem_open(SEM_P1, O_CREAT, 0644, 1);  // Inicializa P1 com 1
    sem_p2 = sem_open(SEM_P2, O_CREAT, 0644, 0);  // Inicializa P2 com 0
    sem_p3 = sem_open(SEM_P3, O_CREAT, 0644, 0);  // Inicializa P3 com 0
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1);  // Inicializa o mutex com 1

    if (sem_p1 == SEM_FAILED || sem_p2 == SEM_FAILED || sem_p3 == SEM_FAILED || mutex == SEM_FAILED) {
        perror("Erro ao criar semáforo");
        exit(EXIT_FAILURE);
    }

    // aloca a memória compartilhada
    segmento = shmget(1300, sizeof(Tabela), IPC_CREAT | 0666);

    // associa a memória compartilhada ao processo
    tabela = (Tabela *)shmat(segmento, 0, 0); // comparar o retorno com -1
    if (tabela == (void *)-1) {
        fprintf(stderr, "Não foi possível alocar memória\n");
        exit(1);
    }

    tabela->index = 0;
    tabela->page_faults = 0;

    pid_t pid1, pid2, pid3;

    // Criar o primeiro processo filho
    pid1 = fork();
    if (pid1 < 0) {
        perror("Erro no fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // Processo filho 1
        read_and_print_file_line_by_line("acessos_P1.txt", "P1", sem_p1, sem_p2, tabela, mutex, 1, algoritmo, rodadas, window_size);
        exit(EXIT_SUCCESS);
    }

    // Criar o segundo processo filho
    pid2 = fork();
    if (pid2 < 0) {
        perror("Erro no fork");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // Processo filho 2
        read_and_print_file_line_by_line("acessos_P2.txt", "P2", sem_p2, sem_p3, tabela, mutex, 2, algoritmo, rodadas, window_size);
        exit(EXIT_SUCCESS);
    }

    // Criar o terceiro processo filho
    pid3 = fork();
    if (pid3 < 0) {
        perror("Erro no fork");
        exit(EXIT_FAILURE);
    } else if (pid3 == 0) {
        // Processo filho 3
        read_and_print_file_line_by_line("acessos_P3.txt", "P3", sem_p3, sem_p1, tabela, mutex, 3, algoritmo, rodadas, window_size);
        exit(EXIT_SUCCESS);
    }

    // Processo pai espera os filhos terminarem
    wait(NULL);
    wait(NULL);
    wait(NULL);

    printf("Todos os processos filhos terminaram.\n");
    printf("Page faults total = %d\n", tabela->page_faults);

    cleanup(0);

    return 0;
}

void print_table(Tabela *tabela) {
    printf("Tabela Compartilhada:\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        printf("[%02d] Número: %02d, BV: %d, BM: %d, BP: %d, Process ID: %d, Age: %d\n", 
            i, tabela->entries[i].pagina_virtual, tabela->entries[i].BV, tabela->entries[i].BM,
            tabela->entries[i].BP, tabela->entries[i].process_id,
            tabela->entries[i].age);
    }
}

void read_and_print_file_line_by_line(const char *filename, char *nome_processo, sem_t *sem_current, sem_t *sem_next, Tabela *tabela, sem_t *mutex, int process_id, const char *algoritmo, int rodadas, int window_size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    char buffer[LINE_SIZE];
    int pagina_virtual;
    char modo;
    int rodada = 0;

    while (fgets(buffer, sizeof(buffer), file) && rodada < rodadas) {
        sem_wait(sem_current);  // Espera pelo semáforo atual
        sem_wait(mutex);  // Bloqueia o acesso à tabela compartilhada

        if (sscanf(buffer, "%d %c", &pagina_virtual, &modo) == 2) {
            tabela = trocador_de_paginas(tabela, pagina_virtual, process_id, modo, algoritmo, window_size);

            // Obtém o índice da página trocada ou nova
            int index = -1;
            for (int i = 0; i < TABLE_SIZE; i++) {
                if (tabela->entries[i].pagina_virtual == pagina_virtual && tabela->entries[i].process_id == process_id) {
                    index = i;
                    break;
                }
            }

            if (index != -1) {
                printf("Processo %s, Arquivo: %s, Índice: %02d, Número: %02d, BV: %d, BM: %d, BP: %d, Process ID: %d\n", 
                    nome_processo, filename, index, 
                    tabela->entries[index].pagina_virtual,
                    tabela->entries[index].BV, 
                    tabela->entries[index].BM, tabela->entries[index].BP,
                    tabela->entries[index].process_id);
            }

            print_table(tabela);  // Imprime a tabela após cada iteração

            age_pages(tabela);  // Atualiza a idade das páginas

        } else {
            fprintf(stderr, "Formato de linha inválido em %s: %s", filename, buffer);
        }

        sem_post(mutex);  // Libera o acesso à tabela compartilhada
        sem_post(sem_next);  // Sinaliza o próximo semáforo

        rodada++;
    }

    fclose(file);
}

void age_pages(Tabela *tabela) { // LRU
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (tabela->entries[i].pagina_virtual != 0) {  // Se a entrada não estiver vazia
            tabela->entries[i].age++;
            if (tabela->entries[i].age >= 4) {
                tabela->entries[i].BV = 0;  // Reseta BV após 4 iterações
            }
        }
    }
}

Tabela* trocador_de_paginas(Tabela *tabela, int pagina_virtual, int process_id, char modo, const char *algoritmo, int window_size) {
    Pagina aux;
    aux.pagina_virtual = pagina_virtual;
    aux.BP = 1;  // Inicializa BP como 1
    aux.process_id = process_id;
    aux.age = 0; // Inicia a idade da nova página como 0
    aux.tempo = 0; // Inicializa o tempo de trabalho como 0
    if (modo == 'R') {
        aux.BV = 1;
        aux.BM = 0;
    } else if (modo == 'W') {
        aux.BV = 1;
        aux.BM = 1;
    } else {
        fprintf(stderr, "Formato de modo inválido\n");
    }

    // Se não encontrou uma entrada existente, adiciona a nova entrada
    if (tabela->entries[tabela->index].pagina_virtual == 0 && strcmp(algoritmo, "WORKING_SET") != 0) { // Vazio, somente na primeira execução
        tabela->entries[tabela->index] = aux;
        tabela->index = (tabela->index + 1) % TABLE_SIZE;  // Atualiza o índice circularmente
        tabela->page_faults++;
    } else {
        int page_fault = FALSE;
        int substituido_id = -1;

        if (strcmp(algoritmo, "NRU") == 0) {
            page_fault = troca_NRU(tabela->entries, aux, &substituido_id);
        } else if (strcmp(algoritmo, "LRU") == 0) {
            page_fault = troca_LRU(tabela->entries, aux, &substituido_id);
        } else if (strcmp(algoritmo, "SECOND_CHANCE") == 0) {
            page_fault = troca_second_chance(tabela->entries, aux, &substituido_id);
        } else if (strcmp(algoritmo, "WORKING_SET") == 0) {
            page_fault = troca_working_set(tabela->entries, aux, window_size, &substituido_id); // Usa o window_size passado como argumento
        } else {
            fprintf(stderr, "Algoritmo de substituição de páginas inválido: %s\n", algoritmo);
            exit(EXIT_FAILURE);
        }

        if (page_fault == TRUE) {
            tabela->page_faults++;
            printf("----------------------------------------------------\n");
            printf("Page fault: Processo %d substituiu Processo %d\n", process_id, substituido_id);
            printf("----------------------------------------------------\n");
        }
    }

    return tabela;
}

int troca_NRU(Pagina *tab_paginas, Pagina entrada, int *substituido_id) {
    int i;
    int num_candidato_sair = 0;

    for (i = 0; i < TABLE_SIZE; i++) {
        if (tab_paginas[i].pagina_virtual == entrada.pagina_virtual && tab_paginas[i].process_id == entrada.process_id) {
            tab_paginas[i].pagina_virtual = entrada.pagina_virtual;
            tab_paginas[i].process_id = entrada.process_id;
            tab_paginas[i].BM = entrada.BM;
            tab_paginas[i].BV = entrada.BV;
            return FALSE; //Não teve page fault
        }
        if (tab_paginas[i].BV < tab_paginas[num_candidato_sair].BV) {
            if (tab_paginas[i].BM < tab_paginas[num_candidato_sair].BM) {
                num_candidato_sair = i;
            }
        } else {
            if (tab_paginas[i].BM < tab_paginas[num_candidato_sair].BM) {
                num_candidato_sair = i;
            }
        }
    }

    *substituido_id = tab_paginas[num_candidato_sair].process_id;

    tab_paginas[num_candidato_sair].pagina_virtual = entrada.pagina_virtual;
    tab_paginas[num_candidato_sair].process_id = entrada.process_id;
    tab_paginas[num_candidato_sair].BM = entrada.BM;
    tab_paginas[num_candidato_sair].BV = entrada.BV;

    return TRUE;
}

int troca_LRU(Pagina *tab_paginas, Pagina entrada, int *substituido_id) {
    int i;
    int candidato_sair = 0;

    for (i = 0; i < TABLE_SIZE; i++) {
        if (tab_paginas[i].pagina_virtual == entrada.pagina_virtual && tab_paginas[i].process_id == entrada.process_id) {
            tab_paginas[i].pagina_virtual = entrada.pagina_virtual;
            tab_paginas[i].process_id = entrada.process_id;
            tab_paginas[i].BM = entrada.BM;
            tab_paginas[i].BV = entrada.BV;
            tab_paginas[i].age = 0; // Reseta a idade da página correta
            return FALSE; // Não houve page fault
        }
        if (tab_paginas[i].age > tab_paginas[candidato_sair].age) {
            candidato_sair = i;
        }
    }

    *substituido_id = tab_paginas[candidato_sair].process_id;

    // Substituição da página menos recentemente usada (candidato_sair)
    tab_paginas[candidato_sair].pagina_virtual = entrada.pagina_virtual;
    tab_paginas[candidato_sair].process_id = entrada.process_id;
    tab_paginas[candidato_sair].BM = entrada.BM;
    tab_paginas[candidato_sair].BV = entrada.BV;
    tab_paginas[candidato_sair].age = 0;

    return TRUE; // Houve page fault
}

int troca_second_chance(Pagina* tab_paginas, Pagina entrada, int *substituido_id) {
    int i;
    int candidato_sair = 0;
    int fault = TRUE;

    for (i = 0; i < TABLE_SIZE; i++) {
        if (tab_paginas[i].pagina_virtual == entrada.pagina_virtual && tab_paginas[i].process_id == entrada.process_id) fault = FALSE;
        if (tab_paginas[i].age > tab_paginas[candidato_sair].age) {
            candidato_sair = i;
        }
    }

    if (tab_paginas[candidato_sair].BP == 1) {
        tab_paginas[candidato_sair].BP = 0;
        tab_paginas[candidato_sair].age = 0;
        fault = troca_second_chance(tab_paginas, entrada, substituido_id);
    } else {
        *substituido_id = tab_paginas[candidato_sair].process_id;

        tab_paginas[candidato_sair].pagina_virtual = entrada.pagina_virtual;
        tab_paginas[candidato_sair].process_id = entrada.process_id;
        tab_paginas[candidato_sair].BM = entrada.BM;
        tab_paginas[candidato_sair].BV = entrada.BV;
        tab_paginas[candidato_sair].BP = 1;
        tab_paginas[candidato_sair].age = 0;
    }

    return fault;
}

int troca_working_set(Pagina *tab_paginas, Pagina entrada, int window_size, int *substituido_id) {
    int i;
    int candidato_sair = -1;
    int found = FALSE;
    int num_process = 0;
    int id_zero = -1;
    int candidato = 0;

    for (i = 0; i< TABLE_SIZE; i++) {
        if ((tab_paginas[i].pagina_virtual == entrada.pagina_virtual) && tab_paginas[i].process_id == entrada.process_id) {
            // Caso em que é atualizada o tempo dentro da janela
            tab_paginas[i].pagina_virtual = entrada.pagina_virtual;
            tab_paginas[i].pagina_virtual = entrada.pagina_virtual;
            tab_paginas[i].process_id = entrada.process_id;
            tab_paginas[i].BM = entrada.BM;
            tab_paginas[i].BV = entrada.BV;
            tab_paginas[i].BP = 1;
            tab_paginas[i].age = 0;
            tab_paginas[i].tempo = 0;
            return FALSE;
        }
        if (tab_paginas[i].process_id == entrada.process_id) {
            num_process++;
        }
        if (tab_paginas[i].pagina_virtual == 0) id_zero = i;
        if (tab_paginas[i].age > 4 && tab_paginas[i].process_id == entrada.process_id) {
            if (tab_paginas[candidato].age - tab_paginas[candidato].tempo < (tab_paginas[i].age - tab_paginas[i].tempo)) 
                candidato = i;
        }
    }
    if (num_process < window_size) {
        tab_paginas[id_zero].pagina_virtual = entrada.pagina_virtual;
        tab_paginas[id_zero].pagina_virtual = entrada.pagina_virtual;
        tab_paginas[id_zero].process_id = entrada.process_id;
        tab_paginas[id_zero].BM = entrada.BM;
        tab_paginas[id_zero].BV = entrada.BV;
        tab_paginas[id_zero].BP = 1;
        tab_paginas[id_zero].age = 0;
        tab_paginas[id_zero].tempo = 0;
    } else {
        tab_paginas[candidato].pagina_virtual = entrada.pagina_virtual;
        tab_paginas[candidato].pagina_virtual = entrada.pagina_virtual;
        tab_paginas[candidato].process_id = entrada.process_id;
        tab_paginas[candidato].BM = entrada.BM;
        tab_paginas[candidato].BV = entrada.BV;
        tab_paginas[candidato].BP = 1;
        tab_paginas[candidato].age = 0;
        tab_paginas[candidato].tempo = 0;
    }

    return TRUE;  
}
