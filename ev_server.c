#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ev.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>


#define PORT1 9989
#define PORT2 9988
#define BUFFER_SIZE 1024
int total_clients1 = 0;
int total_clients2 = 0;

void accept_cb1(struct ev_loop *loop, struct ev_io *watcher, int revents);
void read_cb1(struct ev_loop *loop, struct ev_io *watcher, int revents);

void accept_cb2(struct ev_loop *loop, struct ev_io *watcher, int revents);
void read_cb2(struct ev_loop *loop, struct ev_io *watcher, int revents);

int sock_init(int port)
{
	int sd;
	struct sockaddr_in addr;
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket error");
		return -1;
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		printf("bind error");
	}
	if (listen(sd, 0) < 0) {
		printf("listen error");
		return -1;
	}
	return sd;
}

static int server_init(int port)
{
	struct sockaddr_in serveraddr;
	int listenfd;
	int ret = -1, on = 1;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		printf("Create server socket error: %s\n", strerror(errno));
		return -1;
	}

	if ((ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))) < 0) {
		printf("setsockopt(SO_REUSEADDR): %m");
		goto OUT;
	}

	if (ioctl(listenfd, FIONBIO, (char *)&on) < 0) {
		printf("ioctl FIONBIO failed: %m");
		goto OUT;
	}

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) != 0) {
		printf("Socket bind port (%d) error: %s\n", port, strerror(errno));
		goto OUT;
	}

	if (listen(listenfd, 15) == -1) {
		perror("listen");
		goto OUT;
	}

	return listenfd;
      OUT:
	close(listenfd);
	return -1;
}

int main()
{
	//struct ev_loop *loop = ev_default_loop(0);
            struct ev_loop *loop = EV_DEFAULT;
	int sd1, sd2;

	//int addr_len=sizeof(addr);  
	struct ev_io socket1_accept;
	struct ev_io socket2_accept;

	sd1 = server_init(PORT1);
	sd2 = server_init(PORT2);

	if (sd1 < 0 || sd2 < 0) {
		printf("server socket create error\n");
	}
            printf("server fd is [%d, %d] \n", sd1, sd2);
	ev_io_init(&socket1_accept, accept_cb1, sd1, EV_READ);
	ev_io_init(&socket2_accept, accept_cb2, sd2, EV_READ);

	ev_io_start(loop, &socket1_accept);
	ev_io_start(loop, &socket2_accept);
            
            printf("start loop \n");   
            ev_run (loop, 0);
/*	while (1) {
		ev_loop(loop, 0);
	}*/
	return 0;
}

void accept_cb1(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	//struct sockaddr_in client_addr;  
	int client_sd;
	struct ev_io *w_client = (struct ev_io *)malloc(sizeof(struct ev_io));
	printf("---%s()---\n", __func__);
	if (EV_ERROR & revents) {
		printf("error event in accept");
		return;
	}
	//client_sd=accept(watcher->fd,(struct sockaddr *)&client_addr,&client_len);  
	client_sd = accept(watcher->fd, NULL, NULL);
	if (client_sd < 0) {
		printf("accept error");
		return;
	}
	total_clients1++;
	printf("successfully connected with client.\n");
	printf("%d client connected .\n", total_clients1);
	ev_io_init(w_client, read_cb1, client_sd, EV_READ);
	ev_io_start(loop, w_client);
}

void accept_cb2(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	//struct sockaddr_in client_addr;  
	int client_sd;
	struct ev_io *w_client = (struct ev_io *)malloc(sizeof(struct ev_io));
	printf("---%s()---\n", __func__);
	if (EV_ERROR & revents) {
		printf("error event in accept");
		return;
	}
	//client_sd=accept(watcher->fd,(struct sockaddr *)&client_addr,&client_len);  
	client_sd = accept(watcher->fd, NULL, NULL);
	if (client_sd < 0) {
		printf("accept error");
		return;
	}
	total_clients2++;
	printf("successfully connected with client.\n");
	printf("%d client connected .\n", total_clients2);
	ev_io_init(w_client, read_cb2, client_sd, EV_READ);
	ev_io_start(loop, w_client);
}

void read_cb1(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	char buffer[BUFFER_SIZE];
	int read;
	printf("---%s()---\n", __func__);
	if (EV_ERROR & revents) {
		printf("error event in read");
		return;
	}
	read = recv(watcher->fd, buffer, BUFFER_SIZE, 0);
	if (read == 0) {
		ev_io_stop(loop, watcher);
		free(watcher);
		perror("peer might closing");
		total_clients1--;
		printf("%d client connected .\n", total_clients1);
		return;
	} else if (read < 0) {
		printf("read len is %d\n", read);
	} else {
		buffer[read] = '\0';
		printf("get the message: %s\n", buffer);
	}

	//send(watcher->fd, buffer, read, 0);
	bzero(buffer, read);
}

void read_cb2(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	char buffer[BUFFER_SIZE];
	int read;
	printf("---%s()---\n", __func__);
	if (EV_ERROR & revents) {
		printf("error event in read");
		return;
	}
	read = recv(watcher->fd, buffer, BUFFER_SIZE, 0);
	if (read == 0) {
		ev_io_stop(loop, watcher);
                        close(watcher->fd);
		free(watcher);
		perror("peer might closing");
		total_clients2--;
		printf("%d client connected .\n", total_clients2);
		return;
	} else if (read < 0) {
		printf("read len is %d\n", read);
	} else {
		buffer[read] = '\0';
		printf("get the message: %s\n", buffer);
	}

	// send(watcher->fd, buffer, read, 0);
	bzero(buffer, read);
}
