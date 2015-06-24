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
#define PMAX 20
#define BINT 300 //número de bases dadas a cada cliente para análise
#define PINT 7 //número de expoentes dados a cada cliente para análise
#define MAX_CLIENTS 100

struct set {
    unsigned int A, B, C, x, y, z;
};

struct statusN{
    int type;
    char statusMsg[256];
};

struct statusN status[MAX_CLIENTS];
int clients_had;

struct argument {
    int client_no;
    unsigned int pmin_at, pmax_at, bmin_at, bmax_at;
    int new_sock;
};

void *connection_handler(void *);
void *http_handler(void *);

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
    a = sizeof(unsigned int);
    for (i=0; i<a; i++) {
        result->A += (pkt[i]-1)*pow(256,i);
        result->B += (pkt[i+a]-1)*pow(256,i);
        result->C += (pkt[i+2*a]-1)*pow(256,i);
        result->x += (pkt[i+3*a]-1)*pow(256,i);
        result->y += (pkt[i+4*a]-1)*pow(256,i);
        result->z += (pkt[i+5*a]-1)*pow(256,i);
    }
}

int main(int argc , char *argv[]) {
    int s, conn[10], c, *new_sock, flag1, flag2;
    int client_no = 0;
    struct sockaddr_in  server, client;
    unsigned int pmin_at, pmax_at, bmin_at, bmax_at;
    struct argument args;

    //variáveis do http server
    int listenfd;
    struct sockaddr_in servaddr;
    //fim das variáveis do http server

    if (argc != 3) {
        puts("Argumento esperado: porto e porto_http");
        return 1;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        printf("Erro ao criar o socket.");
        return 1;
    }
    puts("socket criado.\n");

    //http
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1) {
        printf("Erro ao criar o socket para http.");
        return 1;
    }
    puts("socket http criado.\n");
    ///////////////////////

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    //http
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(atoi(argv[2]));
    if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0) {
        perror("Falha no bind");
        return 1;
    }
    puts("bind na porta http");
    //////////////////////////////


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
    //http
    if (listen(listenfd,10)==-1) {
        perror("Erro ao ouvir a porta http");
        exit(1);
    }
    //////////////////////////////////

    puts("Aguardando conexoes...");
    c = sizeof(struct sockaddr_in);


    int i = 0;
    pthread_t thread_id;
    pthread_create(&thread_id,NULL,http_handler,(void*)&listenfd); //o servidor de http rodara em paralelo



    //distribui os intervalos a serem analisados entre os clientes
    for (pmin_at = PMIN, pmax_at = PMIN+PINT - 1; flag1!=1;pmin_at += PINT, pmax_at = (pmax_at + PINT) >= PMAX ? PMAX:pmax_at + PINT) {
        flag1 = (pmax_at == PMAX);
        for (bmin_at = BMIN, bmax_at = BMIN+BINT-1;flag2 != 1;bmin_at += BINT, bmax_at = (bmax_at + BINT) >= BMAX ? BMAX:bmax_at + BINT) {
            flag2 = (bmax_at == BMAX);
            if (client_no > MAX_CLIENTS) {
                    puts("Maximo de clientes excedido.");
                    exit(0);
            }
            if((conn[i] = accept(s, (struct sockaddr *)&client, (socklen_t*)&c))) {
                puts("Conectado a um cliente.");
                pthread_t sniffer_thread;
                args.new_sock = conn[i];
                args.pmin_at = pmin_at;
                args.pmax_at = pmax_at;
                args.bmin_at = bmin_at;
                args.bmax_at = bmax_at;
                args.client_no = client_no;
                if(pthread_create(&sniffer_thread, NULL, connection_handler, (void*)&args) < 0) {
                    perror("Erro ao criar a thread.");
                    return 1;
                }

                //pthread_join( sniffer_thread , NULL);
                puts("Handler assigned");
                client_no ++; //incrementa a flag de clientes
                clients_had = client_no;
                i = (i+1)%10;
            }
            if (conn[i] < 0) {
                perror("Erro ao aceitar conexão.");
                return 1;
            }
        }
    }
    //mesmo depois de terminado o processamento, o servidor de http deve se manter funcionando
    while(1){}
    return 0;
}

void *http_handler (void *socketd) { //lida com tudo do server de http
        struct sockaddr_in cliaddr;
        socklen_t clilen;
        pid_t childpid;
        int connfd,n,i;
        char mesg[10000], string[100];
        int listenfd = *(int*)socketd;
        while(1) {
            clilen=sizeof(cliaddr);
            connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
            if (connfd == -1) {
                perror ("Erro no accept");
                exit(1);
            }
            if ((childpid = fork()) == 0) {
                close (listenfd);
                n = recv(connfd,mesg,1000,0); //recebe mensagem de requisição do cliente
                mesg[n] = 0;
                strcpy(mesg,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<HTML><HEAD><TITLE>Teste</TITLE><style>table, th, td {border: 1px solid black;border-collapse: collapse;}th, td {padding: 5px;}</style></HEAD><BODY>");
                sprintf(string,"<center><h1>Resultados da busca por contra-exemplos da conjectura de Beal</h1></center><br>");
                strcat(mesg,string);
                sprintf(string,"<p>Intervalo total analisado:<br>Bases: %d a %d<br>Expoentes: %d a %d</p><br>",BMIN,BMAX,PMIN,PMAX);
                strcat(mesg,string);
                sprintf(string,"<p>%d clientes ja se conectaram</p><br>", clients_had);
                strcat(mesg, string);
                sprintf(string, "<table style=\"width:100%%\">");
                strcat(mesg,string);
                sprintf(string,"<tr><th>Id do cliente</th><th>Bases</th><th>Expoentes</th><th>Resultado</th></tr>");
                strcat(mesg,string);
                send(connfd,mesg,strlen(mesg),0);
                strcpy(mesg,"");
                for (i=0; i<clients_had; i++) {
                    if (status[i].type == 1) {
                        strcat(mesg,status[i].statusMsg);
                    }
                }
                for (i=0; i<clients_had; i++) {
                    if (status[i].type == 2) {
                        strcat(mesg,status[i].statusMsg);
                    }
                }
                for (i=0; i<clients_had; i++) {
                    if (status[i].type == 3) {
                        strcat(mesg,status[i].statusMsg);
                    }
                }
                strcat(mesg,"</table>");
                strcat(mesg, "</BODY>");
                strcat(mesg, "</HTML>");
                send(connfd,mesg,strlen(mesg),0);
            }
            close(connfd);
        }
}
void *connection_handler(void *args) { //lida com as conexões do clientes de teste
    //Get the socket descriptor
    struct argument *arg;
    arg = args;
    int client_no = arg->client_no;
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
            //printf("\n%s processando intervalo de bases %u a %u e potencias %u a %u\n", identificador, bmin_at, bmax_at,pmin_at,pmax_at);
        status[client_no].type = 1; //status de processando
        sprintf(status[client_no].statusMsg,"<tr><td>%s</td><td>%u a %u</td><td>%u a %u</td><td>Processando</td></tr>", identificador, bmin_at, bmax_at,pmin_at,pmax_at);
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
        puts("Cliente desconectou");
        fflush(stdout);
        return 0;
    }
    client_message[read_size] = 0;
    num = atoi(client_message);
    result = (struct set*)malloc(num*sizeof(struct set));
    for (i=0; i<num; i++) {
        if((read_size = recv(sock, client_message, sizeof(struct set), 0)) < 0) {
            puts("Error.");
            exit(1);
        }
        else if(read_size == 0) {
            puts("Cliente desconectou");
            fflush(stdout);
            return;
        }
        deserialize(client_message,result+i);
        status[client_no].type = 2; //status de encontrado
        sprintf(status[client_no].statusMsg,"<tr><td>%s</td><td>%u a %u</td><td>%u a %u</td><td>%u^%u + %u^%u = %u^%u, sem fator primo comum</td></tr>", identificador, bmin_at, bmax_at,pmin_at,pmax_at, result[i].A, result[i].x,result[i].B,result[i].y,result[i].C,result[i].z);
    }
    if(!num) {
        status[client_no].type = 3; //status de não encontrado
        sprintf(status[client_no].statusMsg,"<tr><td>%s</td><td>%u a %u</td><td>%u a %u</td><td>Nenhum contra-exemplo encontrado</td></tr>",identificador,bmin_at, bmax_at,pmin_at,pmax_at);
    }
    free(message);
    free(result);
    return;
}
