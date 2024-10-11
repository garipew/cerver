#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>


int criar_servidor(){
	int sfd;
	struct addrinfo hint, *p, *servinfo;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;

	if(getaddrinfo(NULL, "80", &hint, &servinfo) != 0){
		printf("1\n");
		return -1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next){
		sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(sfd == -1){
			continue;
		}

		if(bind(sfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sfd);
			continue;
		}
		break;
	}
		
	if(p == NULL){
		printf("2\n");
		return -1;
	}
	freeaddrinfo(servinfo);
	return sfd;
}


int iniciar_servidor(int sfd){
	if(listen(sfd, 10) == -1){
		printf("3\n");
		return 1;
	}
	return 0;
}


int aceitar_conexao(int sfd){
	struct sockaddr_storage client_addr;
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	int cfd;
	sin_size = sizeof(cfd);
	cfd = accept(sfd, (struct sockaddr*)&client_addr, &sin_size);
	inet_ntop(client_addr.ss_family, &(((struct sockaddr_in*)&client_addr)->sin_addr), s, sizeof(s));
	return cfd;
}


int get_method(int cfd, char* msg, char* absolute_path){
	int fsize;
	FILE* fp = fopen(absolute_path, "r");
	char *answ;
	size_t answ_l;
	if(!fp){
		send(cfd, "HTTP/1.1 404 Not Found\r\n", 24, 0);
	} else{
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		rewind(fp);
		memset(msg, 0, sizeof(msg));
		sprintf(msg, "HTTP/1.1 200 OK\r\nDate: \r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", fsize);
		answ_l = strlen(msg) + fsize;
		answ = malloc(answ_l);
		memset(answ, 0, answ_l);
		strcat(answ, msg);	
		memset(msg, 0, sizeof(msg));
		while(fgets(msg, 100, fp)){
			if(strlen(answ) + strlen(msg) <= answ_l){
				strcat(answ, msg);
			}
			memset(msg, 0, sizeof(msg));
		}	
		send(cfd, answ, answ_l, 0);
		fclose(fp);
		free(answ);
	}
	return 0;
}


int main(int argc, char* argv[]){
	int sfd = criar_servidor();
	if(sfd == -1){
		return 1;
	}
	if(iniciar_servidor(sfd)){
		return 1;
	}
	int cfd = aceitar_conexao(sfd);
	char msg[1000];
	char path[70];
	char absolute_path[75];
	char method[5];

	memset(msg, 0, sizeof(msg));
	memset(path, 0, sizeof(path));
	memset(method, 0, sizeof(method));

	if(!recv(cfd, (char*)msg, sizeof(msg), 0)){
		return 1;
	}

	printf("%s", msg);
	sscanf(msg, "%5s %80s ", method, path);
	sprintf(absolute_path, "src%s", path);
	if(!strcmp(method, "GET")){
		get_method(cfd, msg, absolute_path);
	}

	close(sfd);
	close(cfd);
	return 0;
}
