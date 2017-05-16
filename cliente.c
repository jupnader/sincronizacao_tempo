#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSG_LEN (64 *1024)

struct sockaddr_in server_addr(int port, char *addr) {
	struct sockaddr_in saddr;
	saddr.sin_port = htons(port);
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(addr);
	return saddr;
}

int main(int argc, char **argv) {
	if (argc != 5){
		printf("uso: %s <ip_servidor> <porta_servidor> <arquivo_servidor> <arquivo_destino>\n", argv[0]);
		return 0;
	}

	int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sfd < 0) {
		perror("socket()");
		return -1;
	}

	struct sockaddr_in saddr = server_addr(atoi(argv[2]), argv[1]);

	if (connect(sfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
		perror("connect()");
		return -1;
	}

	int ns, nr;
	ns = send(sfd, argv[3], strlen(argv[3]), 0);
	if (ns < 0) {
		perror("send(<arquivo_servidor>)");
		close(sfd);
		return -1;
	}

	int cod_resp;
	nr = recv(sfd, &cod_resp, sizeof(int), 0);
	if (nr < 0) {
		perror("recv(cod_resp)");
		close(sfd);
		return -1;
	}

	if (cod_resp < 0) {
		printf("sevidor: %s\n", strerror(cod_resp*-1));
		close (sfd);
		return -1;
	}

	int fd;
	fd = open(argv[4], O_CREAT | O_RDWR | O_APPEND, 0644);
	if (fd < 0) {
		perror("open()");
		close (sfd);
		return -1;
	}

	void *buff = calloc(1, MSG_LEN);
	if (!buff) {
		perror("calloc()");
		close (sfd);
		return -1;
	}
	int nw;
	do {
		bzero(buff, MSG_LEN);
		nr = recv(sfd, buff, MSG_LEN, 0);
		if (nr > 0) {
			nw = write(fd, buff, nr);
			if (nw < 0) {
				perror("write(<buff>)");
				close(sfd);
				close(fd);
				return -1;
			}
		} else {
			perror("recv(<buff>)");
			close(sfd);
			close(fd);
			return -1;
		}
	} while (nr > 0);
	close(sfd);
	close(fd);
	return 0;
}
