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
#include <netdb.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/stat.h>

#define SERVER_PORT 62341
#define MAX_EVENTS 10
#define MAX_BUF_SIZE 1024

int epoll_fd;
int gRun = 1;
struct epoll_event ev;

char ip[32]={0};

static int server_init(void)
{
	struct sockaddr_in serveraddr;
	int listenfd;
	int ret = -1, on = 1;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		printf("Create server socket error: %s\n",strerror(errno));
		return -1;
	}

	if ((ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
				  (char *) &on, sizeof(on))) < 0) {
		printf("setsockopt(SO_REUSEADDR): %m");
		goto OUT;
	}

	if (ioctl(listenfd, FIONBIO, (char *)&on) < 0) {
		printf("ioctl FIONBIO failed: %m");
		goto OUT;
	}


	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	serveraddr.sin_addr.s_addr = inet_addr(ip);
	if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		printf("Socket bind port (%d) error: %s\n",SERVER_PORT, strerror(errno));
		goto OUT;
	}

	if (listen(listenfd, MAX_EVENTS) == -1) {
		perror("listen");
		goto OUT;
	}

	return listenfd;
OUT:
	close(listenfd);
	return -1;
}

int handle_recv(int fd)
{
	char buf[MAX_BUF_SIZE + 1] = {0};
	int len =  recv(fd, buf, MAX_BUF_SIZE, 0);

	if (len == 0) {
		close(fd);
		return -1;
	}
	printf("fd is %d, filename is %s\n", fd, buf);

	FILE *fp = fopen(buf, "rb");
	if (fp == NULL) {
		printf("file is not exsit\n");
		return -1;
	}

	struct stat file_stat;
	stat(buf, &file_stat);
	printf("filesize is %d\n", (int)file_stat.st_size);

	int read_len = 0;
	int send_len = 0;
	int total = 0;

	while (1) {
		read_len = fread(buf, 1, MAX_BUF_SIZE, fp);
		if (read_len == 0)
			break;
		//printf("read_len is %d\n", read_len);
		send_len = send(fd, buf, read_len, 0);
		total += send_len;
	}
	printf("total send is %d\n", total);
	close(fd);
	fclose(fp);
	return 0;
}

int handle_error(int fd)
{
	printf("fd %d occurs error\n", fd);
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
	close(fd);
	return 0;
}

int getLocalIP(const char *nic){
	int inet_sock;
	struct ifreq ifr;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, nic);
	ioctl(inet_sock, SIOCGIFADDR, &ifr);
	strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	printf("ip is %s\n", ip);
	return 0;
}

int main(int argc, char const *argv[])
{
	/* code */
	int server_fd,new_fd;
	struct sockaddr_storage recv_addr;
	socklen_t sin_size = sizeof(struct sockaddr_storage);
	char buf[100];

	if (argc != 2) {
		printf("use like this : ./fileserver eth0\n");
		return 0;
	}
	getLocalIP(argv[1]);

	server_fd = server_init();
	if (server_fd < 0) {
		return -1;
	}

	epoll_fd = epoll_create(MAX_EVENTS);
	ev.data.fd = server_fd;
	ev.events = EPOLLIN;
	epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&ev);

	int i = 0, nfds = 0;
	struct epoll_event recv_ev[MAX_EVENTS];
	while (gRun) {
		nfds = epoll_wait(epoll_fd, recv_ev, MAX_EVENTS, -1);
		for (i = 0; i < nfds; i++) {
			printf("ev is %d, fd is %d\n",recv_ev[i].events, recv_ev[i].data.fd);
			if (recv_ev[i].data.fd == server_fd) {
				new_fd = accept(server_fd, (struct sockaddr *)&recv_addr, &sin_size);
				if (new_fd < 0) {
					perror("accept");
					break;
				}
				//int flags = fcntl(new_fd, F_GETFL, 0);//获取文件的flags值
				//fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式；
				ev.events = EPOLLIN;
				ev.data.fd = new_fd;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &ev);
				printf("new accept %d\n", new_fd);
			} else if(recv_ev[i].events&EPOLLIN) {
				handle_recv(recv_ev[i].data.fd);
			} else {
				handle_error(ev.data.fd);
			}
		}
	}

	return 0;
}
