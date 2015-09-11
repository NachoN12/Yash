#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //    execv
#include <stdbool.h>
#include <sys/types.h>  //kill
#include <signal.h>		//kill
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
int parse(char *input, char *aux[], char **puntero_aux, int *flag_puntero);
void ejecutar(char **aux, int flag, char **puntero_aux, char *input);

int parse(char *input, char *aux[], char **puntero_aux, int *flag_puntero){

  int cont = 0, terminate = 0;
  char *puntero = input;
  while(*puntero != '\0' && terminate == 0){
     *aux = puntero;
      cont++;
      while(*puntero != ' ' && *puntero != '\t' && *puntero != '\0' && *puntero != '\n' && terminate == 0){
        if(*puntero=='&'){
            *flag_puntero = 4;
          }
        if(*puntero=='>'){ //redieccion salida
          *flag_puntero = 1;
          *aux = '\0';
          puntero++;
          if(*puntero == '>') {//redireccion salida (>>)        
            *flag_puntero = 5;
            puntero++;
          }
          while(*puntero == ' ' || *puntero == '\t')
            puntero++;
            *puntero_aux = puntero;
            char *borrar=*puntero_aux;
           while(*borrar != ' ' && *borrar != '\t' && *borrar != '\n'){
                  borrar++;
                }
            *borrar = '\0';
            terminate = 1;
        }
        if(*puntero=='<'){ // //redieccion entrada
            *flag_puntero = 2;
            *aux = '\0';
            puntero++;
            while(*puntero == ' ' || *puntero == '\t')
              puntero++;
            *puntero_aux = puntero;
            char *borrar=*puntero_aux;
            while(*borrar != ' ' && *borrar != '\t' && *borrar != '\n'){
                  borrar++;
                }
            *borrar = '\0';
            terminate = 1;
          }
        if(*puntero=='|') { 
          *flag_puntero = 3;
          *aux = '\0';
          puntero++;
          while(*puntero == ' ' || *puntero == '\t')
            puntero++;
          *puntero_aux = puntero;
          terminate = 1;
         }     
      puntero++;

    }
    while((*puntero == ' ' || *puntero == '\t' || *puntero == '\n') && terminate == 0){
      *puntero = '\0';
      puntero++;
    }
    aux++;
  }
  *aux = '\0';
  return cont;
}
void ejecutar(char **aux, int flag, char **puntero_aux, char *input){
    pid_t pid, pid2;
    FILE *fp;
    int flag2 = 0, status1;
    char *aux2[30], *puntero_aux2 = NULL;
    int myPipe[2];
    if(flag == 3){
       if(pipe(myPipe)) {         //create pipe
          fprintf(stderr, "Error pipe!");
          exit(-1);
     }
      parse(*puntero_aux, aux2, &puntero_aux2, &flag2);
    }
    pid = fork();          //se crea un child process
    if( pid < 0) {          //si fork retorna <0, es porque ocurrió un error
      printf("Error ");
      exit(-1);
    }
    else if(pid == 0) {  //pregunta si es el child process  
       if(flag==1) {    //pregunta si es una redirección de salida (crea un archivo )
          fp = fopen(*puntero_aux, "w+");
           dup2(fileno(fp), 1);
      }
      if(flag==5){     //pregunta si es una redirección de salida (al final del archivo)
          fp = fopen(*puntero_aux, "a");
          dup2(fileno(fp), 1);
       }
      if(flag==2){
        fp = fopen(*puntero_aux, "r");
        dup2(fileno(fp), 0);
       }
      if(flag==3){
        close(myPipe[0]);   //close input of pipe
        dup2(myPipe[1], fileno(stdout));
        close(myPipe[1]);
      }
      if (execvp(*aux, aux) < 0) {     // ejecuta el comando y si retorna < 0, es porque es un comando inválido //
                    printf("%s :no se encontró la orden\n", input);
               exit(1);
          }
      }
    else  {
    if(flag == 4)
     ; 
    else if(flag == 3)  {
      waitpid(pid, &status1, 0);    //espera por el proceso 1 termine
      pid2 = fork();
      if(pid2 < 0)   {
        printf("error in forking");
        exit(-1);
      }
      else if(pid2 == 0)
      {
        close(myPipe[1]);   //close output to pipe
        dup2(myPipe[0], fileno(stdin));
        close(myPipe[0]);
        execvp(*aux2, aux2);
      }
      else   {
        ;//wait(NULL);
        close(myPipe[0]);
        close(myPipe[1]);
      }
    }
    else{
      waitpid(pid, &status1, 0);
  }
  }
}

int main(int argc, char const *argv[]){
	//pid_t cpid;   // se obtiene el id del proceso
  int flag;
	while (1){
    flag=0;  //bandera de estados
//		wait(cpid); // Espera que termine el proceso hijo
		printf("yash:>");
		char input[56];  //arreglo de caracteres que guarda lo que se ingresa por teclado
		char *aux2[30];
		char *aux[30]; //arreglo de arreglos de char, se guarda en forma parseada 	
		fgets(input,56,stdin); //espera la entrada por teclado
		input[strlen(input) - 1] = '\0';
		//parse(input,aux,aux2); //se parsea input, y se guarda en aux
		
             if (strcmp(input, "exit")==0){
        // kill(cpid, SIGTERM); //termina el programa
         return 0;
      }
    parse(input, aux, aux2, &flag);    //se parsea input, y se guarda en aux
      if(*aux!='\0'){
       if(strcmp(aux[0], "cd") == 0){
        chdir(aux[1]);
      }
    
      else 
      ejecutar(aux, flag, aux2,input);
  }
	}	
}
