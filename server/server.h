#ifndef SERVER
#define SERVER

#include <stddef.h>

void extrair_requisicao(unsigned char*, char*, char*);
void enviar_resposta(int, unsigned char*, size_t, char*, char*);
#endif
