#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "conexao.h"


#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); }while(0)


/* conectar_primeiro(struct addrinfo*)
 * Conecta a primeira conexao possivel
 * Retorna o fd
 */
int conectar_primeiro(struct addrinfo *servinfo){
	struct addrinfo *p;
	int serverfd = -1;

	for(p = servinfo; p != NULL; p = p->ai_next){
		serverfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(serverfd == -1){
			continue;
		}

		if(connect(serverfd, p->ai_addr, p->ai_addrlen) == -1){
			close(serverfd);
			continue;
		}
		break;
	}
		
	if(p == NULL){
		handle_error("addr");
	}
	return serverfd;
}


/* bind_primeiro(struct addrinfo*)
 * Da um bind na primeira conexao possivel
 * Retorna o fd
 */
int bind_primeiro(struct addrinfo *servinfo){
	struct addrinfo *p;
	int serverfd;

	for(p = servinfo; p != NULL; p = p->ai_next){
		serverfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(serverfd == -1){
			continue;
		}

		if(bind(serverfd, p->ai_addr, p->ai_addrlen) == -1){
			close(serverfd);
			continue;
		}
		break;
	}
		
	if(p == NULL){
		handle_error("addr");
	}
	return serverfd;
}


/* iniciar_servidor(int)
 * Começa a escutar requisições de conexões no fd passado como argumento.
 * Retorna 0 em caso de sucesso e 1 em caso de fracasso.
 * sfd -> fd do servidor.
 */
int iniciar_servidor(int sfd){
	if(listen(sfd, 10) == -1){
		printf("listen\n");
		return 1;
	}
	return 0;
}


/* encontrar_conexao(char*, char*)
 * Recebe duas strings como argumento, ip e port,
 * e busca uma conexao, retorna o fd da conexao
*/
int encontrar_conexao(char* ip, char* port){
	int serverfd;
	struct addrinfo hint, *servinfo; 

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;

	if(ip == NULL){
		hint.ai_flags = AI_PASSIVE;
	}

	if(getaddrinfo(ip, port, &hint, &servinfo) != 0){
		handle_error("addrinfo");
	}

	if(ip == NULL){
		serverfd = bind_primeiro(servinfo);
		freeaddrinfo(servinfo);
		if(serverfd != -1){
			if(iniciar_servidor(serverfd)){
				close(serverfd);
				serverfd = -1;
				handle_error("listen");
			}
			printf("Server is running and listening on port %s\n", port);
		}
		return serverfd;
	}
		
	serverfd = conectar_primeiro(servinfo);
	freeaddrinfo(servinfo);
	return serverfd;
}


/* aceitar_conexao(int sfd)
 * Aceita a primeira conexão possível em sfd.
 * Retorna o fd da conexão.
 * sfd -> fd do servidor.
 */
int aceitar_conexao(int sfd){
	struct sockaddr_storage client_addr;
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	int cfd = -1;
	sin_size = sizeof(cfd);
	cfd = accept(sfd, (struct sockaddr*)&client_addr, &sin_size);
	inet_ntop(client_addr.ss_family, &(((struct sockaddr_in*)&client_addr)->sin_addr), s, sizeof(s));
	return cfd;
}
