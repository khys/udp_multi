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
    in_port_t srvport;
    struct msg_echo echo;
    struct sockaddr_in srvskt, from;

    if (argc != 3) {
		fprintf(stderr, "Usage: ./udpcli <IP address> <port number>\n");
		exit(1);
    } else if ((srvport = strtol(argv[2], NULL, 10)) == 0) {
        fprintf(stderr, "Error: bad port number\n");
        exit(1);
    }
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    memset(&srvskt, 0, sizeof srvskt);
    srvskt.sin_family = AF_INET;
    srvskt.sin_port = htons(srvport);
    if (inet_aton(argv[1], &srvskt.sin_addr) < 0) {
        perror("inet_aton");
        exit(1);
    }
    
    echo.seq = 0;
    
    for (;;) {
        printf("message to be sent (FIN to exit): ");
        if (fgets(echo.msg, sizeof echo.msg, stdin) == NULL) {
            break;
        }
        echo.msg[strlen(echo.msg) - 1] = '\0';
        msglen = strlen(echo.msg) + sizeof(unsigned short) * 2;

        if (!strcmp(echo.msg, "FIN")) {
            printf("FIN received from client\n");
            break;
        } else if (echo.seq == 10) {
            printf("Finished: seq = 10\n");
            break;
        }
        
        srvlen = sizeof srvskt;
        if ((cnt = sendto(s, &echo, msglen, 0,
                          (struct sockaddr *)&srvskt, srvlen)) < 0) {
            perror("sendto");
            exit(1);
        }
        printf("%d bytes sent\n", cnt);

        fromlen = sizeof from;
        if ((cnt = recvfrom(s, &echo, sizeof echo, 0,
                            (struct sockaddr *)&from, &fromlen)) < 0) {
            perror("recvfrom");
            exit(1);
        }
        echo.msg[cnt - sizeof(unsigned short) * 2] = '\0';
        printf("%d bytes recved: IP %s, port %d, seq %d, msg %s\n",
               cnt, inet_ntoa(from.sin_addr),
               ntohs(from.sin_port), echo.seq, echo.msg);
    }
    close(s);
}
