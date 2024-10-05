// Eric Goulart - 2110878
// João Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "stdlib.h"

#define TRUE 1
#define FALSE 0


  
int main(int argc, char *argv[]){
  FILE * arquivos[argc-1];
  FILE * cat_file;
  char ch;
  int copy = FALSE;
  int index = 0;
  
  for(int i = 0;i<argc;i++){
    if(argv[i][0]=='>'){
      copy = TRUE;
      index = i+1;
      break;
    }
  }

  if(copy == FALSE){
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
  }else{
    for(int i = 1;i<argc-2;i++){
      arquivos[i] = fopen(argv[i],"r");
      cat_file = fopen(argv[index],"w");
      if (arquivos[i] == NULL)
      {
        printf("Problemas na leitura do arquivo\n");
        return 0;
      }
      while((ch = (getc(arquivos[i]))) != EOF){
        putc(ch,cat_file);
      }
      fclose(arquivos[i]);
    }
    fclose(cat_file);
  }
  printf("\n");



  return 0;
}