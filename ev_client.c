#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define PORT 9999
#define BUFFER_SIZE 1024

int main(int argc,char const *argv[])
{
	int sd;
	struct sockaddr_in addr;


            int port = atoi(argv[1]);
            printf("port is %d\n", port);
	//int addr_len=sizeof(addr);  
	char buf[BUFFER_SIZE] = "";
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
		return -1;
	}
	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect error");
		return -1;
	}
            printf("connect suceess\n");
/*	while (strcmp(buf, "q") != 0) {
		scanf("%s", buf);
		send(sd, buf, strlen(buf), 0);
		//recv(sd, buf, BUFFER_SIZE, 0);
		printf("message:%s\n", buf);
	}*/

           int i = 0, str_len = 0;
            while (1) {
                    str_len = snprintf(buf, sizeof(buf), "0x%dhello", i);
                    printf("str is %s, len is %d\n", buf, str_len);
                     int send_len = send(sd, buf, str_len, 0);
                     printf("send_len is %d\n", send_len);
                    i++;
                     i = i % 100;
        //if (i == 100) {
        //  break;
        //}
            }
	return 0;
}
