#include <sys/socket.h>
#include <string.h>
#include "client.h"


/* extrair_host(char*, char*, char*)
 * Separa o hostname do path em uma url
 */
void extrair_host(char* url, char* hostname, char* path){
	char* divisao;
	if((divisao = strchr(url, '/'))){
		strncpy(hostname, url, divisao - url);
		strncpy(path, divisao, strlen(divisao));
		path[0] = '/';
		return;
	}
	strncpy(hostname, url, strlen(url));
	path[0] = '/';
}


/*void construir_get(char* request, char* path, char* host)
 * Constrói uma requisição get com o caminho em path e
 * armazena em request.
 */
void construir_get(char* request, char* path, char* host){
	strcat(request, "GET ");
	strcat(request, path);
	strcat(request, " HTTP/1.1\r\n");
	strcat(request, "Host: ");
	strcat(request, host);
	strcat(request, "\r\n");
	strcat(request, "Connection: close\r\n\r\n");
}
