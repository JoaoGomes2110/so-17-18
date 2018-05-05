
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
	int status, i, fd_aux, fd;
	int comandos = 0;
	int nivel = 0;

	fd_aux = open("teste.txt", O_RDONLY);

	if(fd_aux!=-1){

		while((linha_aux = lerLinha(0))!=NULL){
			if(linha_aux[0] == '$'){
				comandos++;
			}
		}
		close(fd_aux);
	}

	int pipes[comandos][2];

	fd = open("teste.txt", O_RDONLY);

	if(fd!=-1){
		while((linha = lerLinha(fd))!=NULL){
			if (linha[0] == '$' && linha[1] == '|'){
				printf("$%s\n", linha + 1);
				
				for(token = strtok(linha+1, " "), i = 0; token; token = strtok(NULL, " "), i++){
					args[i] = token;
				}

				args[i] = NULL;
				printf(">>>\n");
				pipe(pipes[nivel]);

				if(fork()==0){
					dup2(0,pipes[nivel-1][0]);
					execvp(args[0], args);
					dup2(pipes[nivel][1],1);
					close(pipes[nivel][1]);
					_exit(0);
				}

				close(pipes[nivel][1]);
				wait(&status);

				while((linha = lerLinha(pipes[nivel][0]))){
					printf("%s\n", linha);
				}
							
				close(pipes[nivel][0]);
				nivel++;
				printf("<<<\n");
			}
			else if (linha[0] == '$'){

					printf("$%s\n", linha + 1);
					for(token = strtok(linha+1, " "), i = 0; token; token = strtok(NULL, " "), i++){
						args[i] = token;
					}

					args[i] = NULL;
					printf(">>>\n");
					pipe(pipes[nivel]);

					if(fork() == 0){
						execvp(args[0], args);
						dup2(pipes[nivel][1],1);
						close(pipes[nivel][1]);
						_exit(0);
					}

					close(pipes[nivel][1]);
					wait(&status);

					while((linha = lerLinha(pipes[nivel][0]))){
						printf("%s\n", linha);
					}
							
					close(pipes[nivel][0]);
					nivel++;
					printf("<<<\n");
			}
			else{
				printf("%s\n", linha);
			}
		}
	}else{
		perror("ERRO AO LER O FICHEIRO");
	}
}

	
	
