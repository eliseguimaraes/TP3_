#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#define BMIN 3
#define BMAX 1000
#define PMIN 3
#define PMAX 15
#define BINT 20
#define PINT 3

struct set {
    unsigned int A, B, C, x, y, z, comum;
};

struct argument {
    int pmin_at, pmax_at, bmin_at, bmax_at;
    int new_sock;
};

void *connection_handler(void *);

void serialize (unsigned int numA, unsigned int numB, unsigned int numC, unsigned int numD, char *pkt) {
    unsigned int i, a;
    i = 0;
    a = sizeof(unsigned int);
    for (i=0; i<a; i++) { //serializa os quatro inteiros
        pkt[i] = (numA) >> 8*i;
        pkt[i]+=1; //evitando que se tenha NULL
        pkt[i+a] = (numB) >> 8*i;
        pkt[i+a]+=1;
        pkt[i+2*a] = (numC) >> 8*i;
        pkt[i+2*a]+=1;
        pkt[i+3*a] = (numD) >> 8*i;
        pkt[i+3*a]+=1;
    }
    i+=3*a;
    pkt[i] = '\0';
}

void deserialize (unsigned char *pkt, struct set *result) {
    unsigned int i, a;
    i = 0;
    result->A = 0;
    result->B = 0;
    result->C = 0;
    result->x = 0;
    result->y = 0;
    result->z = 0;
    result->comum = 0;
    a = sizeof(unsigned int);
    for (i=0; i<a; i++) {
        result->A += (pkt[i]-1)*pow(256,i);
        result->B += (pkt[i+a]-1)*pow(256,i);
        result->C += (pkt[i+2*a]-1)*pow(256,i);
        result->x += (pkt[i+3*a]-1)*pow(256,i);
        result->y += (pkt[i+4*a]-1)*pow(256,i);
        result->z += (pkt[i+5*a]-1)*pow(256,i);
        result->comum += (pkt[i+6*a]-1)*pow(256,i);
    }
}

int main(int argc , char *argv[]) {
    int s, conn[10], c, *new_sock, flag1, flag2;
    struct sockaddr_in  server, client;
    unsigned int pmin_at, pmax_at, bmin_at, bmax_at;
    struct argument args;

    if (argc != 2) {
        puts("Argumento esperado: porto");
        return 1;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        printf("Erro ao criar o socket.");
        return 1;
    }
    puts("socket criado.\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    //Bind
    if (bind(s,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Falha no bind.");
        return 1;
    }
    puts("bind\n");

    if (listen(s,10) == -1) {
        printf("\nErro ao aguardar conexão: %s", strerror(errno));
        exit(1);
    }

    printf("\nminimo: %d\n",BMIN);
    puts("Aguardando conexoes...");
    c = sizeof(struct sockaddr_in);

    //inicializa as variáveis globais


    int i = 0;
    for (pmin_at = PMIN, pmax_at = PMIN+PINT - 1; flag1!=1;pmin_at += PINT, pmax_at = (pmax_at + PINT) >= PMAX ? PMAX:pmax_at + PINT) {
        flag1 = (pmax_at == PMAX);
        for (bmin_at = BMIN, bmax_at = BMIN+BINT-1;flag2 != 1;bmin_at += BINT, bmax_at = (bmax_at + BINT) >= BMAX ? BMAX:bmax_at + BINT) {
            flag2 = (bmax_at == BMAX);
            if((conn[i] = accept(s, (struct sockaddr *)&client, (socklen_t*)&c))) {
                puts("Conectado a um cliente.");

                pthread_t sniffer_thread;
                args.new_sock = conn[i];
                args.pmin_at = pmin_at;
                args.pmax_at = pmax_at;
                args.bmin_at = bmin_at;
                args.bmax_at = bmax_at;
                if(pthread_create(&sniffer_thread, NULL, connection_handler, (void*)&args) < 0) {
                    perror("Erro ao criar a thread.");
                    return 1;
                }

                //pthread_join( sniffer_thread , NULL);
                puts("Handler assigned");
                i = (i+1)%10;
            }
            if (conn[i] < 0) {
                perror("Erro ao aceitar conexão.");
                return 1;
            }
        }
    }

    return 0;
}


void *connection_handler(void *args) {
    //Get the socket descriptor
    struct argument *arg;
    arg = args;
    int sock = arg->new_sock;
    unsigned int pmin_at, pmax_at, bmin_at, bmax_at;
    pmin_at = arg->pmin_at;
    pmax_at = arg->pmax_at;
    bmin_at = arg->bmin_at;
    bmax_at = arg->bmax_at;
    unsigned int num,i;
    unsigned int read_size;
    char *message , client_message[2000], identificador[2000];
    struct set *result;

    if ((read_size = recv(sock , client_message , 2000 , 0)) > 0) {
            strcpy(identificador, client_message);
            printf("\n%s processando intervalo de bases %u a %u e potencias %u a %u\n", identificador, bmin_at, bmax_at,pmin_at,pmax_at);
    }

    message = (char*)malloc(4*sizeof(unsigned int));
    serialize(pmin_at,pmax_at,bmin_at,bmax_at,message);
    write(sock , message , strlen(message));

    if((read_size = recv(sock , client_message , 3 , 0)) < 0) {
        puts("Error.");
        exit(1);
    }
    else if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
        return 0;
    }
    client_message[read_size] = 0;
    num = atoi(client_message);
    result = (struct set*)malloc(num*sizeof(struct set));
    for (i=0; i<num; i++) {
        if((read_size = recv(sock , client_message , 3 , 0)) < 0) {
            puts("Error.");
            exit(1);
        }
        else if(read_size == 0) {
            puts("Client disconnected");
            fflush(stdout);
            return;
        }
        puts(client_message);
    }
    if(!num) printf("\nNada encontrado por %s.",identificador);
    free(message);
    free(result);
    return;
}
