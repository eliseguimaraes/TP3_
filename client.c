#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>

void deserialize (unsigned char *pkt, unsigned int *pmin, unsigned int *pmax, unsigned int *bmin, unsigned int *bmax) {
    unsigned int i, a;
    i = 0;
    *pmin = 0;
    *pmax = 0;
    *bmin = 0;
    *bmax = 0;
    a = sizeof(unsigned int);
    for (i=0; i<a; i++) {
        *pmin += (pkt[i]-1)*pow(256,i);
        *pmax += (pkt[i+a]-1)*pow(256,i);
        *bmin += (pkt[i+2*a]-1)*pow(256,i);
        *bmax += (pkt[i+3*a]-1)*pow(256,i);
    }
}

struct set {
    unsigned int A, B, C, x, y, z;
};

void serialize (struct set result, char *pkt) {
    unsigned int i, a;
    i = 0;
    a = sizeof(unsigned int);
    for (i=0; i<a; i++) { //serializa os quatro inteiros
        pkt[i] = (result.A) >> 8*i;
        pkt[i]+=1; //evitando que se tenha NULL
        pkt[i+a] = (result.B) >> 8*i;
        pkt[i+a]+=1;
        pkt[i+2*a] = (result.C) >> 8*i;
        pkt[i+2*a]+=1;
        pkt[i+3*a] = (result.x) >> 8*i;
        pkt[i+3*a]+=1;
        pkt[i+4*a] = (result.y) >> 8*i;
        pkt[i+4*a]+=1;
        pkt[i+5*a] = (result.z) >> 8*i;
        pkt[i+5*a]+=1;
    }
    i+=5*a;
    pkt[i] = '\0';
}

unsigned int commonFactor (unsigned int a, unsigned int b) {
    if (a>b) {
        if (a%b==0) return b;
        else return (commonFactor(a%b,b));
    }
    else {
        if(b%a==0) return a;
        else return (commonFactor(b%a,a));
    }
}

unsigned int commonFactor3 (unsigned int a, unsigned int b, unsigned int c) {
    return (commonFactor(commonFactor(a,b),c));
}

int searchResult (unsigned int bmin, unsigned int bmax, unsigned int pmin, unsigned int pmax, struct set *result) {
    unsigned int A,B,C,x,y,z,comum,r;
    long long int aux;
    int i = 0;
    for (A=bmin; A<=bmax; A++) {
        for (x=pmin; x<=pmax; x++) {
            for (B=bmin; B<=bmax; B++) {
                for (y=pmin; y<=pmax; y++) {
                    aux = pow(A,x) + pow(B,y);
                    for (z = pmin; z<=pmax; z++) {
                        C = powf(aux, ((float)1/(float)z));
                        r = pow(C,z);
                            if (r==aux && z>2) {
                                comum = commonFactor3(A,B,C);
                                if (comum!=1) {
                                    //printf("\n%d^%d + %d^%d = %d^%d, com fator primo %d\n",A,x,B,y,C,z,comum);
                                }
                                else {
                                    result[i].A = A;
                                    result[i].B = B;
                                    result[i].C = C;
                                    result[i].x = x;
                                    result[i].y = y;
                                    result[i].z = z;
                                    i++;
                                }
                            }
                        }
                    }
                }
            }
        }
        return i; //retorna o numero de resultados encontrados
}

int main(int argc , char *argv[]) {
    unsigned int bmin, bmax, pmin,pmax, num,i;
    struct set result[100];
    int sock, n, tam_pkt, tam_pkt2;
    long long unsigned int aux;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000], *pkt;

    if(argc!=4) {
        puts("Argumentos esperados: host_servidor, porto_servidor, id_cliente");
        return 1;
    }

    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        printf("Erro ao criar o socket.");
    }
    puts("Socket criado.");

    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));

    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Erro na conexão");
        return 1;
    }

    puts("Conectado ao server.\n");
    tam_pkt = 4*sizeof(unsigned int);
    tam_pkt2 = sizeof(struct set);
    pkt = (char*)malloc(tam_pkt2);
    //Comunicação com o server
        strcpy(message, argv[3]);

        //Envia o identificador
        if(send(sock , message , strlen(message) , 0) < 0) {
            puts("Falha no envio.");
            return 1;
        }

        //Recebe os intervalos a serem analisados do servidor
        if((n = recv(sock, server_reply, tam_pkt, 0)) < 0) {
            puts("Falha no recebimento.");
            return 1;
        }
        server_reply[n] = 0;
        deserialize(server_reply, &pmin, &pmax, &bmin, &bmax);
        printf ("\nAnalisando intervalos de bases %u a %u e potencias %u a %u.\n",bmin, bmax, pmin, pmax);
        num = searchResult(bmin, bmax, pmin, pmax, result);
        sprintf(message,"%03d",num); //envia o numero de resultados encontrados (provavelmente nenhum)
        if(send(sock , message , strlen(message) , 0) < 0) {
            puts("Falha no envio.");
            return 1;
        }
        for (i=0; i<num; i++) { //envia os resultados encontrados um a um
                serialize(result[i],pkt);
                if (send(sock, pkt, tam_pkt2, 0)<0) {
                        puts("Falha no envio.");
                        return 1;
                }
        }
    free(pkt);
    close(sock);
    return 0;
}
