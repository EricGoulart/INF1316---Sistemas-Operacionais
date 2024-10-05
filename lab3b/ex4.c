//Eric Goulart - 2110878
//João Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

void handler(int signum);

int main(void) {

  int a,b;
  float x,y;
  int soma_int, sub_int, mult_int, div_int;
  float soma_float, sub_float, mult_float, div_float;

  void (*p)(int);

  p = signal(SIGFPE,handler);

  printf("Digite o primeiro número real aqui:\n");
  scanf("%f",&x);
  printf("Digite o segundo número real aqui:\n");
  scanf("%f",&y);

  soma_float = x+y;
  sub_float = x-y;
  mult_float = x*y;
  div_float = x/y;
  printf("Soma: %f\nSubtração: %f\nMultiplicação: %f\nDivisão: %f\n",
    soma_float, 
    sub_float,
    mult_float, 
    div_float
  );

  printf("Digite o primeiro número inteiro aqui:\n");
  scanf("%d",&a);
  printf("Digite o segundo número inteiro aqui:\n");
  scanf("%d",&b);

  soma_int = a+b;
  sub_int = a-b;
  mult_int = a*b;
  div_int = a/b;

  printf("Soma: %d\nSubtração: %d\nMultiplicação: %d\nDivisão: %d\n",
    soma_int, 
    sub_int,
    mult_int, 
    div_int
  );
  
  
  return 0;
}

void handler(int signum){
  printf("Você tentou dividir por 0, saíndo do programa...\n");
  exit(1);
}