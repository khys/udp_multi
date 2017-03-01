#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

// #define SRVPORT 49152

struct msg_echo {
	unsigned short seq;
	unsigned short reserve;
	char msg[32];
};

int main(int argc, char *argv[])
{
    int s, msglen, cnt;
    socklen_t srvlen, fromlen;
    in_port_t port;
    fd_set rfds;
    struct msg_echo echo;
    struct sockaddr_in myskt, srvskt, from;

    if (argc != 3) {
		fprintf(stderr, "Usage: ./udpsrv <IP address> <port number>\n");
		exit(1);
	} else if ((port = strtol(argv[2], NULL, 10)) == 0) {
        fprintf(stderr, "Error: bad port number\n");
        exit(1);
    }
    
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    memset(&myskt, 0, sizeof myskt);
    myskt.sin_family = AF_INET;
    myskt.sin_port = htons(port);
    myskt.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&myskt, sizeof myskt) < 0){
        perror("bind");
        exit(1);
    }

    memset(&srvskt, 0, sizeof srvskt);
    srvskt.sin_family = AF_INET;
    srvskt.sin_port = htons(port);
    if (inet_aton(argv[1], &srvskt.sin_addr) < 0) {
        perror("inet_aton");
        exit(1);
    }
    
    for (;;) {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(s, &rfds);

        if (select(s+1, &rfds, NULL, NULL, NULL) < 0) {
            fprintf(stderr, "select: error\n");
            exit(1);
        }
        if (FD_ISSET(0, &rfds)) {
            if (fgets(echo.msg, sizeof echo.msg, stdin) == NULL) {
                break;
            }
            echo.msg[strlen(echo.msg) - 1] = '\0';
            msglen = strlen(echo.msg) + sizeof(unsigned short) * 2;
            
            if (!strcmp(echo.msg, "FIN")) {
                srvlen = sizeof srvskt;
                if ((cnt = sendto(s, &echo, msglen, 0,
                                  (struct sockaddr *)&srvskt,
                                  srvlen)) < 0) {
                    perror("sendto");
                    exit(1);
                }
                printf("-- FIN sent --\n");
                break;
            }

            srvlen = sizeof srvskt;
            if ((cnt = sendto(s, &echo, msglen, 0,
                              (struct sockaddr *)&srvskt, srvlen)) < 0) {
                perror("sendto");
                exit(1);
            }
            printf("-- %d bytes sent --\n", cnt);
        }
        if (FD_ISSET(s, &rfds)) {
            fromlen = sizeof from;
            if ((cnt = recvfrom(s, &echo, sizeof echo, 0,
                                (struct sockaddr *)&from, &fromlen)) < 0) {
                perror("recvfrom");
                exit(1);
            }
            echo.msg[cnt - sizeof(unsigned short) * 2] = '\0';
            msglen = strlen(echo.msg) + sizeof(unsigned short) * 2;

            if (!strcmp(echo.msg, "FIN")) {
                printf("-- FIN received --\n");
                break;
            }
            
            printf("-- %d bytes recved: IP %s, port %d --\n> %s\n",
                   cnt, inet_ntoa(from.sin_addr),
                   ntohs(from.sin_port), echo.msg);
        }
    }
    close(s);
}
