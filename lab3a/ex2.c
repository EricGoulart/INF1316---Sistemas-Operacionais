//Eric Goulart da Cunha - 2110878
//Joâo Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define EVER ;;

void killHandler(int sinal);

int main (void) {
  void (*p)(int);// ponteiro para função que recebe int como parâmetro
  pid_t pid = getpid();
  printf("pid é === %d\n",pid);
  p = signal(SIGKILL, killHandler);
  printf("Endereco do manipulador anterior %p\n", p);
  for(EVER);
}

void killHandler(int sinal) {
  printf ("Você tentou dar sigkill (%d)\n", sinal);
}
