// Eric Goulart - 2110878
// João Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "stdlib.h"

#define TRUE 0
#define FALSE 1


  
int main(int argc, char *argv[]){
  FILE * arquivos[argc-1];
  char ch;

  for(int i = 1;i<argc;i++){
    arquivos[i] = fopen(argv[i],"r");
    if (arquivos[i] == NULL)
    {
      printf("Problemas na leitura do arquivo\n");
      return 0;
    }
    while((ch = (getc(arquivos[i]))) != EOF){
      printf("%c", ch);
    }
    fclose(arquivos[i]);
    
  }
  printf("\n");



  return 0;
}