#ifndef SERVER
#define SERVER

#include <stddef.h>

int criar_servidor();
int aceitar_conexao(int);
void extrair_requisicao(unsigned char*, char*, char*, char*);
void enviar_resposta(int, unsigned char*, size_t, char*, char*);
#endif
