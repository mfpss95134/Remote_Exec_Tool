#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
//------------new
#include <time.h>
//--------------
#define MAXLINE  64

void str_echo(int fd);
void sig_chld(int signo);
//-----------new
void time_cycle(int fd);
//--------------
int main(int argc, char **argv)
{
    if (argc < 2) {
       fprintf(stderr,"usage %s port\n", argv[0]);
       exit(0);
    }
    int portno = atoi(argv[1]);
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;

    struct sockaddr_in cliaddr, servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portno);
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, 5);
    if (signal(SIGCHLD, sig_chld) == SIG_ERR){
        fprintf(stderr, "can't catch SIGCHLD \n");
        exit(0);
    }
    for ( ; ; ){
        clilen = sizeof(cliaddr);
        if ( (connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0){
            if (errno == EINTR) 
                continue; // back to for()
            else{
                fprintf(stderr, "accept error\n");
                exit(0);
            }
        }

        if ((childpid = fork()) == 0){ // child
            close(listenfd); // close listening socket
//----new
			time_cycle(connfd);
			//----
            str_echo(connfd);// process the request
			
            exit(0);
        }
        close(connfd); // parent closes connected socket
    }
    return 0;
}

void str_echo(int sockfd)
{
    int n;
    char buf[MAXLINE];
again:
    while ( (n = read(sockfd, buf, MAXLINE)))
        write(sockfd, buf, n);
    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        printf("str_echo: read error");
}
//----------new
void time_cycle(int sockfd)
{
time_t now;
    
    int n;
    while(1)
	{
time(&now);
sleep(1);
		n = write(sockfd, ctime(&now), strlen(ctime(&now)));
        if (n < 0) 
         error("ERROR writing to socket");

	}
}
//------------------
void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);// printf() isn't suitable for use here
    return;
}
