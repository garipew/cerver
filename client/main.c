#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include "../conexao.h"
#include "client.h"


int main(int argc, char* argv[]){
	if(argc < 2){
		printf("missing.\n");
		return 1;
	}
	char host[100] = { 0 };
	char path[100] = { 0 };
	char msg[1000] = { 0 };
	int serverfd;
	extrair_host(argv[1], host, path);
	serverfd = encontrar_conexao(host, "80");

	construir_get(msg, path, host);
	
	send(serverfd, msg, sizeof(msg), 0);
	while(recv(serverfd, (char*)msg, sizeof(msg), 0)){
		printf("%s\n", msg);
		memset(msg, 0, sizeof(msg));
	}
	close(serverfd);
	return 0;
}
