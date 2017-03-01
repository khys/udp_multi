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
    socklen_t fromlen;
    in_port_t myport;
    struct msg_echo echo;
    struct sockaddr_in myskt, from;

    if (argc != 2) {
		fprintf(stderr, "Usage: ./udpsrv <port number>\n");
		exit(1);
	} else if ((myport = strtol(argv[1], NULL, 10)) == 0) {
        fprintf(stderr, "Error: bad port number\n");
        exit(1);
    }
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    memset(&myskt, 0, sizeof myskt);
    myskt.sin_family = AF_INET;
    myskt.sin_port = htons(myport);
    myskt.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&myskt, sizeof myskt) < 0){
        perror("bind");
        exit(1);
    }
    for (;;) {
        fromlen = sizeof from;
        if ((cnt = recvfrom(s, &echo, sizeof echo, 0,
                            (struct sockaddr *)&from, &fromlen)) < 0) {
            perror("recvfrom");
            exit(1);
        }
        echo.msg[cnt - sizeof(unsigned short) * 2] = '\0';
        msglen = strlen(echo.msg) + sizeof(unsigned short) * 2;

        printf("%d bytes recved: IP %s, port %d, seq %d, msg %s\n",
               cnt, inet_ntoa(from.sin_addr),
               ntohs(from.sin_port), echo.seq, echo.msg);
        
        echo.seq++;
        
        if ((cnt = sendto(s, &echo, msglen, 0,
                          (struct sockaddr *)&from, fromlen)) < 0) {
            perror("sendto");
            exit(1);
        }
        printf("%d bytes sent\n", cnt);
    }
    close(s);
}
