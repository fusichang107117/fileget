#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 62341

int main(int argc, char const *argv[])
{
	/* code */
	struct sockaddr_in sockaddr;
	int sockfd;
	char buf[100];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Create ot server socket error: %s\n",strerror(errno));
		return -1;
	}

	bzero(&sockaddr, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	sockaddr.sin_port = htons(SERVER_PORT);

	if (connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		printf("Connect to server error: %s:%d\n", SERVER_IP, SERVER_PORT);
		return -1;
	}
	int i = 0, str_len = 0;
	while (1) {
		str_len = snprintf(buf, sizeof(buf), "0x%dhello", i);
		printf("str is %s, len is %d\n", buf, str_len);
		int send_len = send(sockfd, buf, str_len, 0);
		printf("send_len is %d\n", send_len);
		i++;
		i = i % 100;
		//if (i == 100) {
		//	break;
		//}
	}
	sleep(10);
	//close(sockfd);
	return 0;
}
