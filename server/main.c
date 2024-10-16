#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include "server.h"
#include "../conexao.h"


static int keepalive;


void sigint_handler(int signum){
	keepalive = 0;
}


int main(int argc, char* argv[]){
	signal(SIGINT, sigint_handler);

	int sfd = encontrar_conexao(NULL, "80");
	if(sfd == -1){
		return 1;
	}
	
	unsigned char msg[1000];
	char absolute_path[100];
	char method[5];
	char *connection;
	int cfd = -1;
	keepalive = 1;

	while(keepalive){
		cfd = cfd == -1 ? aceitar_conexao(sfd) : cfd;
		memset(msg, 0, sizeof(msg));
		memset(method, 0, sizeof(method));
		memset(absolute_path, 0, sizeof(absolute_path));

		if(!recv(cfd, (char*)msg, sizeof(msg), 0)){
			close(cfd);
			cfd = -1;
			continue;
		}

		extrair_requisicao(msg, method, absolute_path);
		enviar_resposta(cfd, msg, sizeof(msg), method, absolute_path);
		printf("%s %s\n", method, absolute_path);
		close(cfd);
		cfd = -1;
	}

	close(sfd);
	return 0;
}
