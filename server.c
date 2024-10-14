#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <time.h>


int criar_conexao(){
	int sfd;
	struct addrinfo hint, *p, *servinfo;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;

	if(getaddrinfo(NULL, "80", &hint, &servinfo) != 0){
		printf("addrinfo\n");
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
		freeaddrinfo(servinfo);
		printf("bind\n");
		return -1;
	}
	freeaddrinfo(servinfo);
	return sfd;
}


int iniciar_servidor(int sfd){
	if(listen(sfd, 10) == -1){
		printf("listen\n");
		return 1;
	}
	return 0;
}


int criar_servidor(){
	int sfd = criar_conexao();
	if(sfd == -1){
		return sfd;
	}
	int listen = iniciar_servidor(sfd);
	if(listen){
		close(sfd);
		return -1;
	}
	return sfd;
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


unsigned char* uncat(unsigned char* dst, unsigned char* src){
	unsigned char *p;
	p = memcpy(dst+strlen(dst), src, strlen(src));	
	p = '\0';
	return dst;
}


void extrair_tipo(char* absolute_path, char* type, size_t type_l){
	char *suffix;
	suffix = strchr(absolute_path, '.');
	if(!suffix){
		strcat(absolute_path, ".html");
		suffix = strchr(absolute_path, '.');
	} 
	memset(type, 0, type_l);
	if(!strncmp(suffix, ".html", 5) ){
		sprintf(type, "text/html; charset=UTF-8");
	} else if(!strncmp(suffix, ".css", 4)){
		sprintf(type, "text/css; charset=UTF-8");
	} else if(!strncmp(suffix, ".png", 4)){
		sprintf(type, "image/png");
	} else if(!strncmp(suffix, ".jp", 3)){
		sprintf(type, "image/jpeg");
	} else if(!strncmp(suffix, ".svg", 4)){
		sprintf(type, "image/svg+xml");
	} else if(!strncmp(suffix, ".json", 5)){
		sprintf(type, "application/json");
	} else if(!strncmp(suffix, ".js", 3)){
		sprintf(type, "text/javascript");
	} else if(!strncmp(suffix, ".ttf", 4)){
		sprintf(type, "font/ttf");
	} else if(!strncmp(suffix, ".webp", 5)){
		sprintf(type, "image/webp");
	} else if(!strncmp(suffix, ".ico", 4)){
		sprintf(type, "image/vnd.microsoft.icon");
	}
	suffix = NULL;
}


char* encontrar_data(){
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char* date = asctime(tm);
	date[strlen(date) - 1] = 0;
	return date;
}


int escrever_head(FILE* fp, unsigned char* msg, size_t msg_l, char* absolute_path){
	char type[100];
	extrair_tipo(absolute_path, type, sizeof(type));
	int fsize = 0;
	if(!fp){
		memset(msg, 0, msg_l);
		sprintf(msg, "HTTP/1.1 404 Not Found\r\nDate: %s\r\n\r\n", encontrar_data());
	} else{
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		rewind(fp);
		memset(msg, 0, msg_l);
		sprintf(msg, "HTTP/1.1 200 OK\r\nDate: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n", encontrar_data(), fsize, type);
	}
	return fsize;
}


int get_method(int cfd, unsigned char* msg, size_t msg_l, char* absolute_path){
	unsigned char* answ;
	size_t answ_l;
	FILE* fp = fopen(absolute_path, "r");
	int fsize = escrever_head(fp, msg, msg_l, absolute_path);
	if(!fp){
		send(cfd, msg, msg_l, 0);
	} else{
		answ_l = strlen(msg) + fsize + 3;
		answ = malloc(answ_l);
		if(!answ){
			printf("heap\n");
			return 1;
		}
		memset(answ, 0, answ_l);
		uncat(answ, msg);	
		while(fread(answ+strlen(answ), sizeof(unsigned char), answ_l-strlen(answ), fp)){
		}	
		send(cfd, answ, answ_l, 0);
		fclose(fp);
		free(answ);
	}
	return 0;
}


int head_method(int cfd, unsigned char* msg, size_t msg_l, char* absolute_path){
	FILE *fp = fopen(absolute_path, "r");
	escrever_head(fp, msg, msg_l, absolute_path);
	send(cfd, msg, msg_l, 0);
	if(fp){
		fclose(fp);
	}
	return 0;
}


char* filtrar_str(char* dst, char* src){
	char *path_until, *remaining_path = dst;
	char fixed_path[80] = { 0 };
	path_until = strstr(remaining_path, src);
	while(path_until){
		remaining_path = path_until + strlen(src);
		path_until = strstr(remaining_path, src);
		if(!path_until){
			strncat(fixed_path, remaining_path, strlen(remaining_path));
			break;
		}
		strncat(fixed_path, remaining_path, path_until - remaining_path);
	}
	if(dst != remaining_path){
		strncpy(dst, fixed_path, sizeof(fixed_path));
	}
	return dst;
}


void extrair_requisicao(unsigned char* msg, char* method, char* path, char* absolute_path){
	sscanf(msg, "%5s %80s ", method, path);
	filtrar_str(path, "/..");
	path[0] = '/';
	sprintf(absolute_path, "src%s", path);
	if(absolute_path[strlen(absolute_path) -1] == '/'){
		strcat(absolute_path, "index.html");
	}
}


void enviar_resposta(int cfd, unsigned char* msg, size_t msg_l, char* method, char* absolute_path){
	if(!strcmp(method, "GET")){
		get_method(cfd, msg, msg_l, absolute_path);
		return;
	}
	if(!strcmp(method, "HEAD")){
		head_method(cfd, msg, msg_l, absolute_path);
		return;
	}
	if(!strcmp(method, "POST")){
	
	}
}
