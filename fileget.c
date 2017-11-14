#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <stdbool.h>

#define SERVER_PORT 62341
#define MAX_BUF_SIZE 1024

int main(int argc, char const *argv[])
{
	/* code */
	struct sockaddr_in sockaddr;
	int sockfd;
	char buf[MAX_BUF_SIZE] = {0};
	const char *ip, *filename;
	char *d = "/";

	if (argc !=2) {
		printf("use like this ./fileget 192.168.0.188/filename\n");
		return 0;
	}

	printf("%s\n", argv[1]);
	ip = strtok(argv[1], d);
	printf("ip is %s\n", ip);
	filename= strtok(NULL, d);
	printf("filename is %s\n", filename);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Create ot server socket error: %s\n",strerror(errno));
		return -1;
	}

	bzero(&sockaddr, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(ip);
	sockaddr.sin_port = htons(SERVER_PORT);

	if (connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
		printf("Connect to server error: %s:%d\n", ip, SERVER_PORT);
		return -1;
	}

	send(sockfd, filename, strlen(filename), 0);
	printf("wait for fetch filename %s,\n", filename);

	FILE *fp = fopen(filename, "wb");

	int recv_len = 0;
	int total = 0;
	int write_len = 0;
	while (1) {
		recv_len = recv(sockfd, buf, sizeof(buf), 0);
		//printf("recv_len is %d\n", recv_len);
		if (recv_len <= 0) {
			break;
		}
		total += recv_len;
		write_len += fwrite(buf, 1, recv_len, fp);
	}

	printf("total recv size is %d, write_len is %d\n", total, write_len);

	fclose(fp);
	close(sockfd);
	return 0;
}
