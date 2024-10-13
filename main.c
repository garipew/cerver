#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "server.h"


int main(int argc, char* argv[]){
	int sfd = criar_servidor();
	if(sfd == -1){
		return 1;
	}
	
	unsigned char msg[1000];
	char path[70];
	char absolute_path[75];
	char method[5];
	char *connection;
	int cfd = -1;
	int keepalive = 0;

	while(1){
		cfd = cfd == -1 ? aceitar_conexao(sfd) : cfd;
		memset(msg, 0, sizeof(msg));
		memset(path, 0, sizeof(path));
		memset(method, 0, sizeof(method));
		memset(absolute_path, 0, sizeof(absolute_path));

		if(!recv(cfd, (char*)msg, sizeof(msg), 0)){
			close(cfd);
			cfd = -1;
			continue;
		}

		extrair_requisicao(msg, method, path, absolute_path);
		enviar_resposta(cfd, msg, sizeof(msg), method, absolute_path, sizeof(absolute_path));
		printf("%s %s\n", method, path);
		close(cfd);
		cfd = -1;
	}

	close(sfd);
	return 0;
}
