#define _XOPEN_SOURCE 700
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


int main(int argc, char** argv) {

    //==================================
    //parameters we don't need to modify
    //==================================
    char buffer[BUFSIZ];
    enum CONSTEXPR { MAX_REQUEST_LEN = 1024};
    char request[MAX_REQUEST_LEN];
    char request_template[] = "GET / HTTP/1.1\r\nHost: %s\r\n\r\n";
    struct protoent *protoent;
    char hostname[30] ;
    in_addr_t in_addr;
    int request_len;
    int socket_file_descriptor;
    ssize_t nbytes_total, nbytes_last;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;
    unsigned short server_port = 80;


    fprintf(stderr, "Enter the hostname: ");
    scanf("%s",hostname);
    request_len = snprintf(request, MAX_REQUEST_LEN, request_template, hostname);
    if (request_len >= MAX_REQUEST_LEN) {
        fprintf(stderr, "request length large: %d\n", request_len);
        exit(EXIT_FAILURE);
    }

    /* Build the socket. */
    protoent = getprotobyname("tcp");
    if (protoent == NULL) {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }

    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (socket_file_descriptor == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Build the address. */
    hostent = gethostbyname(hostname);
    if (hostent == NULL) {
        fprintf(stderr, "error: gethostbyname(\"%s\")\n", hostname);
        exit(EXIT_FAILURE);
    }
    in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
    if (in_addr == (in_addr_t)-1) {
        fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
        exit(EXIT_FAILURE);
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(server_port);

    /* Actually connect. */
    if (connect(socket_file_descriptor, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    /* Send HTTP request. */
    fprintf(stderr, "socket: Start send HTTP request\n");
    nbytes_total = 0;
    while (nbytes_total < request_len) {
        nbytes_last = write(socket_file_descriptor, request + nbytes_total, request_len - nbytes_total);
        if (nbytes_last == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        nbytes_total += nbytes_last;
    }

    /* Read the response. */
    fprintf(stderr, "socket: Start read the response\n");
    while ((nbytes_total = read(socket_file_descriptor, buffer, BUFSIZ)) > 0) 
    {
        //==============================================
        // uncomment next line to get response on screen
        // write(STDOUT_FILENO, buffer, nbytes_total);
        //==============================================
    }
    fprintf(stderr, "socket: Finish read to buffer\n");
    if (nbytes_total == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    close(socket_file_descriptor);

    
    printf("======== Hyperlinks ========\n");
    //===========================
    // spider the "<a href" label
    //===========================
    char *ret1,*ret2;
    
    ret1 = strstr(buffer,"<a href")+9;
    ret2 = strstr(buffer,"</a>");
        for(;*ret1!='\"';ret1++)
            printf("%c",*ret1);
    printf("\n");
    while(ret1 = strstr(ret1,"<a href"))
    {
        ret1+=9;
        for(;*ret1!='\"';ret1++)
            printf("%c",*ret1);
        printf("\n");
    }
    exit(EXIT_SUCCESS);
}