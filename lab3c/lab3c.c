//Eric Goulart da Cunha - 2110878
//Joâo Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define EVER ;;
#define TRUE 1
#define FALSE 0
#define TEMPO_FIM_SEGUNDOS 181

int main(void){
    int filho1, filho2, filho3;

    struct timeval start;
    gettimeofday(&start, NULL);

    if((filho1 = fork()) < 0){
        fprintf(stderr, "Erro ao criar filho\n");
        exit(-1);
    }if(filho1 == 0){
        for(EVER);
    }else{
        if((filho2 = fork()) < 0){
            fprintf(stderr, "Erro ao criar filho\n");
            exit(-1);
        }
        if(filho2 == 0){ /* child */
            for(EVER);
        }else{
            if((filho3 = fork()) < 0){
                fprintf(stderr, "Erro ao criar filho\n");
                exit(-1);
            }if(filho3 == 0){ /* child */
                for(EVER);
            }
        }       
    }

    printf("Pausando o processo pid %d\n", filho1);
    kill(filho1, SIGSTOP);
    printf("Pausando o processo pid %d\n", filho2);
    kill(filho2,SIGSTOP);

    struct timeval curr;
    struct timeval temp;
    struct timeval end;
    gettimeofday(&end, NULL);
    
    int i=1;
    while((end.tv_sec-start.tv_sec)<TEMPO_FIM_SEGUNDOS){
        printf("Iteração %d\n",i);
        if(filho1 != 0 && filho2 !=0 && filho3 != 0){
            gettimeofday(&curr,NULL);
            while((curr.tv_sec-start.tv_sec) < 5){
                gettimeofday(&curr,NULL);
            }
            printf("Pausando o processo pid %d\n", filho3);
            kill(filho3,SIGSTOP);
            temp = curr;
            printf("Continuando o processo de pid %d\n", filho1);
            kill(filho1, SIGCONT);
            while((curr.tv_sec-temp.tv_sec)<20){
                gettimeofday(&curr,NULL);
            }
            printf("Pausando o processo pid %d\n", filho1);
            kill(filho1, SIGSTOP);
            printf("Continuando o processo de pid %d\n", filho3);
            kill(filho3, SIGCONT);
            while((curr.tv_sec-start.tv_sec)<45){
                gettimeofday(&curr,NULL);
            }
            printf("Pausando o processo pid %d\n", filho3);
            kill(filho3,SIGSTOP);
            temp = curr;
            printf("Continuando o processo de pid %d\n", filho2);
            kill(filho2, SIGCONT);
            while((curr.tv_sec-temp.tv_sec)<15){
                gettimeofday(&curr,NULL);
            }
            printf("Pausando o processo pid %d\n", filho2);
            kill(filho2, SIGSTOP);
        } 
        gettimeofday(&end, NULL);
        i++;
    }
    return 0;
}