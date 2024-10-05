//Eric Goulart da Cunha - 2110878
//João Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MSG_LEN 4

struct msgbuf {
    long mtype;
    int mtext;
};

int main(void) {
    pid_t sender, receiver;
    int msqid, status;
    struct msqid_ds buf;
    struct msgbuf aux;
    struct msgbuf aux2;

    msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if (msgctl(msqid, IPC_STAT, &buf) == -1) {
        perror("msgctl IPC_STAT");
        exit(EXIT_FAILURE);
    }

    buf.msg_qbytes = 4; // Permitir 1 msg de int na fila

    if (msgctl(msqid, IPC_SET, &buf) == -1) {
        perror("msgctl IPC_SET");
        exit(EXIT_FAILURE);
    }

    sender = fork();
    if (sender < 0) {
        perror("Error creating sender process");
        exit(EXIT_FAILURE);
    }
    if (sender == 0) {
        for (int i = 1; i <= 128; i++) {
            aux.mtype = 1;
            aux.mtext = i;
            if (msgsnd(msqid, &aux, sizeof(aux.mtext), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            printf("Enviou msg %d\n", aux.mtext);
            if(msgrcv(msqid, &aux2, sizeof(aux2.mtext),2,0)==-1){ // recebe mensagem de receiver de confirmação
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
        }
        exit(0);
    }

    receiver = fork();
    if (receiver < 0) {
        perror("Error creating receiver process");
        exit(EXIT_FAILURE);
    }
    if (receiver == 0) {
        for (int i = 1; i <= 128; i++) {
            if (msgrcv(msqid, &aux, sizeof(aux.mtext), 1, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
            printf("Recebeu message %d\n", aux.mtext);
            aux2.mtype = 2; // manda mensagem do tipo 2, de confirmação, para sender.
            aux2.mtext = i;
            if (msgsnd(msqid, &aux2, sizeof(aux.mtext), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        }
        exit(0);
    }

    for (int i = 0; i < 2; i++) {
        wait(&status);
    }

    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID");
        exit(EXIT_FAILURE);
    }

    return 0;
}