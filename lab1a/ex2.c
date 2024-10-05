// Eric Goulart - 2110878
// João Pedro Biscaia Fernandes - 2110361
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "stdlib.h"

#define TRUE 0
#define FALSE 1

int main(int argc, char *argv[]){

  for(int i = 1;i<argc;i++){
    printf("%s ",argv[i]);
  }
  printf("\n");
  
  
  return 0;
}