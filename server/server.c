#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <time.h>


/* void extrair_tipo(char*, char*, size_t)
 * Extrai o tipo de arquivo requisitado por GET ou HEAD
 * Armazena o MIME equivalente no segundo argumento.
 * absolute_path -> caminho descrito na requisição.
 * type -> endereço em que será armazenado o tipo equivalente.
 * type_l -> tamanho de type.
 */
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


/* char* encontrar_data()
 * Retorna uma string contendo a data.
 */
char* encontrar_data(){
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char* date = asctime(tm);
	date[strlen(date) - 1] = 0;
	return date;
}


/* int escrever_head(FILE*, unsigned char*, size_t, char*)
 * Escreve a header da resposta para GET ou HEAD.
 * fp -> fp do arquivo descrito na requisição. NULL caso arquivo não exista.
 * msg -> armazena todo o conteúdo da requisição.
 * msg_l -> tamanho de msg.
 * absolute_path -> caminho descrito na requisição.
 */
int escrever_head(FILE* fp, unsigned char* msg, size_t msg_l, char* absolute_path){
	char type[100];
	extrair_tipo(absolute_path, type, sizeof(type));
	int fsize = 0;
	if(!fp){
		memset(msg, 0, msg_l);
		sprintf(msg, "HTTP/1.0 404 Not Found\r\nDate: %s\r\n\r\n", encontrar_data());
	} else{
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		rewind(fp);
		memset(msg, 0, msg_l);
		sprintf(msg, "HTTP/1.0 200 OK\r\nDate: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n", encontrar_data(), fsize, type);
	}
	return fsize;
}


/*int get_method(int, unsigned char*, size_t, char*)
 * Processa requisições GET e envia resposta.
 * Retorna 0 em sucesso e 1 em fracasso.
 * cfd -> fd da requisição.
 * msg -> armazena todo o conteúdo da requisição.
 * msg_l -> tamanho de msg.
 * absolute_path -> caminho descrito na requisição.
 */
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
		strcat(answ, msg);	
		while(fread(answ+strlen(answ), sizeof(unsigned char), answ_l-strlen(answ), fp)){
		}	
		send(cfd, answ, answ_l, 0);
		fclose(fp);
		free(answ);
	}
	return 0;
}


/* int head_method(int cfd, unsigned char* msg, size_t msg_l, char* absolute_path)
 * Processa requisição HEAD e envia resposta.
 * Retorna 0 em sucesso.
 * cfd -> fd da requisição.
 * msg -> armazena todo o conteúdo da requisição.
 * msg_l -> tamanho de msg.
 * absolute_path -> caminho descrito na requisição.
 */
int head_method(int cfd, unsigned char* msg, size_t msg_l, char* absolute_path){
	FILE *fp = fopen(absolute_path, "r");
	escrever_head(fp, msg, msg_l, absolute_path);
	send(cfd, msg, msg_l, 0);
	if(fp){
		fclose(fp);
	}
	return 0;
}


/* char* filtrar_str(char* filtered, char* filter)
 * Filtra todas as ocorrencias de filter em filtered.
 * Retorna filtered.
 * filtered -> string original.
 * filter -> filtro a ser removido.
 */
char* filtrar_str(char* filtered, char* filter){
	char *path_until, *remaining_path = filtered;
	char fixed_path[80] = { 0 };
	path_until = strstr(remaining_path, filter);
	while(path_until){
		remaining_path = path_until + strlen(filter);
		path_until = strstr(remaining_path, filter);
		if(!path_until){
			strncat(fixed_path, remaining_path, strlen(remaining_path));
			break;
		}
		strncat(fixed_path, remaining_path, path_until - remaining_path);
	}
	if(filtered != remaining_path){
		strncpy(filtered, fixed_path, sizeof(fixed_path));
	}
	return filtered;
}


/* void extrair_requisicao(unsigned char* msg, char* method, char* path, char* absolute_path)
 * Extrai o método e o caminho de uma requisição feita.
 * msg -> armazena todo o conteúdo da requisição.
 * method -> endereço onde será armazenado o método extraido.
 * absolute_path -> endereço onde será armazenado o caminho extraido.
 */
void extrair_requisicao(unsigned char* msg, char* method, char* absolute_path){
	char path[80] = { 0 };
	sscanf(msg, "%5s %80s ", method, path);
	filtrar_str(path, "/..");
	path[0] = '/';
	sprintf(absolute_path, "root%s", path);
	if(absolute_path[strlen(absolute_path) -1] == '/'){
		strcat(absolute_path, "index.html");
	}
}


/* void enviar_resposta(int cfd, unsigned char* msg, size_t msg_l, char* method, char* absolute_path){
 * Envia uma resposta apropriada para a requisição em method.
 * cfd -> fd da requisição.
 * msg -> armazena todo o conteúdo da requisição.
 * msg_l -> tamanho de msg.
 * method -> método da requisição.
 * absolute_path -> caminho descrito na requisição.
 */
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
