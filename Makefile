srv: server.o conexao.o server/main.c
	gcc server/main.c server.o conexao.o -o srv

clt: client.o conexao.o client/main.c
	gcc client/main.c client.o conexao.o -o clt

server.o: server/server.c server/server.h
	gcc -c server/server.c

client.o: client/client.c client/client.h
	gcc -c client/client.c

conexao.o: conexao.c conexao.h
	gcc -c conexao.c 

clean:
	rm -rf *.o srv clt
