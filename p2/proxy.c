/*
 * proxy.c - A Simple Sequential Web proxy
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 2
 * Student Name: Ben Wiggins
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"

/*
 * Function prototypes
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
    int listenfd, valread, connfd, webservfd;
    unsigned int len;
    struct sockaddr_in client_addr, proxy_addr;
    char buffer[MAXBUF], get[MAXLINE], uri[MAXLINE], http_ver[MAXLINE], logstring[MAXLINE];

    //For URI parsing, may move
    int port = -1;
    //char protocol[MAXLINE], host[MAXLINE], path[MAXLINE];
    char host[MAXLINE];

    //For server response
    char buffer2[MAXLINE];
    rio_t rio;
    ssize_t riosize, filesize;

    /* Check arguments */
    if (argc != 2) {
	    fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	    exit(0);
    }

    //Create socket
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Failed to create socket");
        exit(1);
    }

    //Populate socket address struct
    bzero(&proxy_addr, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    proxy_addr.sin_port = htons(atoi(argv[1]));

    //Bind socket to address
    if(bind(listenfd, (struct sockaddr*) &proxy_addr, sizeof(proxy_addr)) < 0){
        perror("Failed to bind socket");
        exit(1);
    }

    //printf("Listening on IP: %s, Port: %u", inet_ntoa(proxy_addr.sin_addr), ntohs(proxy_addr.sin_port));

    listen(listenfd, LISTENQ);

    //Loop infinitely to continually handle requests.
    while(1){
        //Accept connection request from client
        len = sizeof(client_addr);
        if((connfd = accept(listenfd, (struct sockaddr*) &client_addr, &len)) < 0){
            perror("Failed to accept");
            exit(1);
        }

        //Read from client socket into buffer
        valread = read(connfd, buffer, MAXBUF);

        //printf("%s", buffer);

        //Split GET, uri, and HTTP version into three separate variables
        if(sscanf(buffer, "%s %s %s", get, uri, http_ver) < 3){perror("Error parsing request"); exit(1);};

        //printf("%s\n", uri);
        //Check for http
        /*char prefix[] = "http://";
        if (strncmp(uri, prefix, 7) == 0) {
            printf("The URI starts with 'http://'\n");
        } else {
            printf("The URI does not start with 'http://'\n");
        }*/

        /*Since "http://" is 7 characters, I have to start parsing the uri
        7 characters in to resolve hostname and port. If the hostname doesn't
        have ':' or '/' or ' ' or '\0' at the end, then just read in the rest of
        the uri to host.*/
        if(strpbrk(uri + 7, " :/\0")){
            strncpy(host, uri + 7, strpbrk(uri + 7, " :/\0") - (uri + 7));
            if(strpbrk(uri + 7, ":")){
                sscanf(strpbrk(uri + 7, ":") + 1, "%d", &port);
            }
        }else{
            strncpy(host, uri + 7, strlen(uri) - 7);
        }

        //printf("%s %d\n", host, port);

        //Open a connection to the web server using the host and port
        if(port > -1){
            if((webservfd = Open_clientfd(host, port)) < 0){
                perror("Failed to connect to webserver");
                exit(1);
            };
        } else {
            if((webservfd = Open_clientfd(host, 80)) < 0){
                perror("Failed to connect to webserver");
                exit(1);
            };
        }

        //Initialize the robust I/O using the header file's rio helper functions
        Rio_readinitb(&rio, webservfd);
        
        //Write the request to the webserver
        Rio_writen(webservfd, buffer, strlen(buffer));
 

        /*Read the response a chunk at a time and write it back to the client socket.
        Increase the recorded response size each time for logging purposes.*/
        filesize = 0;

        while((riosize = Rio_readnb(&rio, buffer2, MAXLINE)) > 0){
            Rio_writen(connfd, buffer2, riosize);
            filesize += riosize;
            memset(buffer2, 0, sizeof(buffer2));
        }


        //Open log file and log using the provided helper function
        FILE *file = fopen("proxy.log", "a");
        format_log_entry(logstring, &client_addr, uri, filesize);
        fprintf(file, "%s\n", logstring);
        fclose(file);
        close(connfd);
    }


    exit(0);
}


/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}


