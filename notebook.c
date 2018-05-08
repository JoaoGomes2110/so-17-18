
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_CHAR_LINE 1024
#define MAX_ARGS 100

char *lerLinha(int fd){
	char *buffer = (char*) malloc(MAX_CHAR_LINE);
	int res, i = -1;

	while( (res=read(fd, buffer + ++i, 1)) > 0 && buffer[i] != '\n' && i < MAX_CHAR_LINE - 1);

	if (res == 0 && i <= 0) return NULL;

	buffer[i] = '\0';

	return buffer;
}

int main(){
	
	char *linha, *linha_aux, *args[MAX_ARGS], *token;
	int status, i, fd;
	int comandos = 0;
	int file, nivel = 0;

	int p[2];

	fd = open("teste.txt", O_RDONLY);


	if(fd!=-1){
		while((linha = lerLinha(fd))!=NULL){
			if (linha[0] == '$' && linha[1] == '|'){	
				
				printf("$|%s\n", linha + 2);

				for(token = strtok(linha+2, " "), i = 0; token; token = strtok(NULL, " "), i++){
					args[i] = token;
				}
					
				args[i] = NULL;
				printf(">>>\n");

				pipe(p);

				if(fork()==0){
					close(p[0]);
					file = open("tmp", O_CREAT|O_RDONLY);
					dup2(p[1], 1);
					dup2(file, 0);
					execvp(args[0], args);
					close(p[1]);
					close(file);
				}


				wait(&status);
				close(p[1]);
				file = open("tmp",O_CREAT| O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);

				while((linha = lerLinha(p[0]))){
					write(1, linha, strlen(linha));
					write(file, linha, strlen(linha));
					write(file,"\n",1);
					write(1,"\n",1);
				}	

				close(p[0]);
				close(file);
				printf("<<<\n");
			}	
			else if (linha[0] == '$'){

				printf("$%s\n", linha + 1);

				for(token = strtok(linha+1, " "), i = 0; token; token = strtok(NULL, " "), i++){
					args[i] = token;
				}
					
				args[i] = NULL;
				printf(">>>\n");

				pipe(p);

				if(fork()==0){
					close(p[0]);
					dup2(p[1], 1);
					execvp(args[0], args);
					close(p[1]);
				}

				wait(&status);
				close(p[1]);

				file = open("tmp",O_CREAT| O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);

				while((linha = lerLinha(p[0]))){
					write(1, linha, strlen(linha));
					write(file, linha, strlen(linha));
					write(file,"\n",1);
					write(1,"\n",1);
				}	

				close(p[0]);
				close(file);
				printf("<<<\n");
					
			}
			else{
				printf("%s\n", linha);
			}
		}
	}
	else {
		perror("ERRO AO LER O FICHEIRO");
	}

	return 0;
}

	
	
