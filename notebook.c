
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>


#define MAX_CHAR_LINE 1024
#define MAX_ARGS 100
int controlo;


void handler(){
	controlo = 1;
}

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
	int file, fd_result;
	signal(SIGUSR1,handler);

	int p[2];

	fd = open("teste.txt", O_RDONLY);
	fd_result = open("result", O_WRONLY|O_RDONLY|O_CREAT|O_TRUNC,S_IRWXU | S_IRWXG | S_IRWXO);


	if(fd!=-1){
		while((linha = lerLinha(fd))!=NULL){
			
			if (linha[0] == '$' && linha[1] == '|'){	
				write(fd_result,linha,strlen(linha));
				write(fd_result,"\n",1);

				for(token = strtok(linha+2, " "), i = 0; token; token = strtok(NULL, " "), i++)
					args[i] = token;
					
				args[i] = NULL;
				write(fd_result,">>>\n",4);

				pipe(p);

				if(fork()==0){
					close(p[0]);

					file = open("tmp", O_RDONLY);

					dup2(p[1], 1);
					dup2(file, 0);
					execvp(args[0], args);
					kill(getppid(),SIGUSR1);
				}
			
				wait(&status);
				close(p[1]);

				if(controlo == 0){
					file = open("tmp",O_TRUNC| O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);

					while((linha = lerLinha(p[0]))){
						write(fd_result, linha, strlen(linha));
						write(file, linha, strlen(linha));
						write(file,"\n",1);
						write(fd_result,"\n",1);
					}	

					close(p[0]);
					close(file);
					write(fd_result,"<<<\n",4);
				}else{
					perror("COMANDO IMPOSSIVEL DE EXECUTAR");
					_exit(0);
				}	
			}	
			else if(linha[0] == '$'){
				write(fd_result,linha,strlen(linha));
				write(fd_result,"\n",1);

				for(token = strtok(linha+1, " "), i = 0; token; token = strtok(NULL, " "), i++){
					args[i] = token;
				}
					
				args[i] = NULL;
				write(fd_result,">>>\n",4);

		  		pipe(p);

				if(fork()==0){
					close(p[0]);
					dup2(p[1], 1);
					execvp(args[0], args);
					kill(getppid(),SIGUSR1);
				}

				wait(&status);
				close(p[1]);

				if(controlo == 0){

					file = open("tmp",O_TRUNC | O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);

					while((linha = lerLinha(p[0]))){
						write(fd_result, linha, strlen(linha));
						write(file, linha, strlen(linha));
						write(file,"\n",1);
						write(fd_result,"\n",1);
					}	

					close(p[0]);
					close(file);
					write(fd_result,"<<<\n",4);
				}else{
					perror("COMANDO IMPOSSIVEL DE EXECUTAR");
					_exit(0);
				}
			}
		}
		close(fd_result);
		remove("teste.txt");
		rename("result","teste.txt");
	}else{
		perror("ERRO AO LER O FICHEIRO");
	} 
	return 0;
}

	
	
