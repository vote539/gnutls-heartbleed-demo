/* This example code is placed in the public domain. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

/* tcp.c */
int tcp_connect(char* server, char* port);
void tcp_close(int sd);

/* Connects to the peer and returns a socket
 * descriptor.
 */
extern int tcp_connect(char* server, char* port)
{
        int err, sd;
        struct sockaddr_in sa;

        /* connects to server
         */
        sd = socket(AF_INET, SOCK_STREAM, 0);

        memset(&sa, '\0', sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(atoi(port));
        inet_pton(AF_INET, server, &sa.sin_addr);

        printf("Connecting to %s:%s\n", server, port);

        err = connect(sd, (struct sockaddr *) &sa, sizeof(sa));
        if (err < 0) {
                fprintf(stderr, "Connect error %d\n", err);
                exit(1);
        }

        return sd;
}

/* closes the given socket descriptor.
 */
extern void tcp_close(int sd)
{
        shutdown(sd, SHUT_RDWR);        /* no more receptions */
        close(sd);
}
