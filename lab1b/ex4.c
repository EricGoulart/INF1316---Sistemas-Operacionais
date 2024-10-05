//Eric Goulart - 2110878
//João Pedro Biscaia Fernandes - 2110361

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "stdlib.h"
#include <string.h>


#define TRUE 1
#define FALSE 0

#define MAX_COMMAND_SIZE 80

void type_prompt() {
    printf("$");
}

char*  read_command(void);
char ** split_commands(char*line);
void run_command(char **args);

int main(void) {
  int status;
  char *line =(char*) malloc(MAX_COMMAND_SIZE*sizeof(char));
  char **args;

  int n = 3;
  int i;
  pid_t pid;
  while(TRUE){
    type_prompt();
    line = read_command();
    args = split_commands(line);
    
    if((pid = fork())!=0){
      // Parent code!
      waitpid(-1,&status,0);
    }else{
      // Child code
      if(strcmp(args[0], "meuecho")==0 || strcmp(args[0], "meucat")==0){
        run_command(args);
      }else if (execvp(args[0], args) == -1) {
        perror("lsh");
        exit(EXIT_FAILURE);
      }
    
    }
    free(line);
    free(args);
  }
  return 0;
}

char* read_command(void){
  char * buffer = (char*)malloc(sizeof(char)*MAX_COMMAND_SIZE);
  int position=0;
  int c;

  while (1) {
    c = getchar();

    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
    
  }
}

char ** split_commands(char* line){
  char ** params = (char**) malloc(10*sizeof(char*));
  if(!params){
    printf("Erro ao alocar memoria");
    exit(1);
  }
  char *word;
  int pos = 0;

  word = strtok(line, " ");
  while(word != NULL){
    params[pos] = malloc(20*sizeof(char));
    strcpy(params[pos], word);
    pos++;

    word = strtok(NULL, " ");
  }
  params[pos] = NULL;
  return params;
}

void run_command(char **args){
  if(strcmp(args[0], "meuecho")==0){
    int pos = 1;
    while(args[pos] != NULL){
      printf("%s ", args[pos]);
      pos++;
    }
    printf("\n");
    return;
  }else{
    FILE * arquivos[10];
    char ch;

    for(int i = 1;args[i]!=NULL;i++){
      arquivos[i] = fopen(args[i],"r");
      if (arquivos[i] == NULL)
      {
        printf("Problemas na leitura do arquivo\n");
        exit(1);
      }
      while((ch = (getc(arquivos[i]))) != EOF){
        printf("%c", ch);
      }
      fclose(arquivos[i]);

    }
    printf("\n");
  }
    
}