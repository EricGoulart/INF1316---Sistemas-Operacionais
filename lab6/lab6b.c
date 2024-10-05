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

    msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if (msgctl(msqid, IPC_STAT, &buf) == -1) {
        perror("msgctl IPC_STAT");
        exit(EXIT_FAILURE);
    }
    buf.msg_qnum = 0;
    buf.msg_qbytes = 32; // Permitir 32 bytes na fila (8 mensagens de 4 bytes cada)

    if (msgctl(msqid, IPC_SET, &buf) == -1) {
        perror("msgctl IPC_SET");
        exit(EXIT_FAILURE);
    }

    sender = fork();
    if (sender < 0) {
        perror("Erro ao criar processo de envio");
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
            printf("Enviou mensagem %d\n", aux.mtext);
        }
        exit(0);
    }

    receiver = fork();
    if (receiver < 0) {
        perror("Erro ao criar processo receptor");
        exit(EXIT_FAILURE);
    }
    if (receiver == 0) {
        for (int i = 1; i <= 128; i++) {
            if (msgrcv(msqid, &aux, sizeof(aux.mtext), 0, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
            printf("Recebeu mensagem %d\n", aux.mtext);
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
