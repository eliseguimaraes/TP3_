
/* Sample TCP server */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
int i;
int main(int argc, char**argv) {
   int listenfd,connfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t clilen;
   pid_t childpid;
   char mesg[1000], string[100];
    i = 0;
   listenfd=socket(AF_INET,SOCK_STREAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(30000);
   bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   listen(listenfd,10);
      while (1) {
        clilen=sizeof(cliaddr);
        connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
        if (connfd == -1) {
            perror ("Erro no accept");
            exit(1);
        }
        if ((childpid = fork()) == 0) {
            close (listenfd);
            n = recv(connfd,mesg,1000,0);
            printf("-------------------------------------------------------\n");
            mesg[n] = 0;
            printf("Received the following:\n");
            printf("%s",mesg);
            printf("-------------------------------------------------------\n");
            strcpy(mesg,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<HTML><HEAD><TITLE>Teste</TITLE></HEAD><BODY>");
            sprintf(string,"<center>Hello World! %d </center>", i);
            strcat(mesg, string);
            strcat(mesg, "</BODY>");
            strcat(mesg, "</HTML>");
            puts(mesg);
            send(connfd,mesg,strlen(mesg),0);
        }
        i++;
        close(connfd);
      }
}
