/*
 * Echo client example for testing itrace
 *
 * All clients who connect to the server will receive messages sent by all other
 * clients
 *
 * Compile with make echoclient
 * */
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <string>

int main(int argc, char **argv) {
	// Make a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("error making socket");
		:q
		exit(1);
	}

	struct sockaddr_in addr;
	addr.sin_family      = AF_INET;
	struct hostent *host = gethostbyname("127.0.0.1");
	if (host == NULL) {
		perror("error gethostbyname");
		exit(1);
	}
	memcpy(&addr.sin_addr, host->h_addr, host->h_length);
	addr.sin_port = htons(34900);  // server port

	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("error connecting");
		exit(1);
	}

	std::string message {};
	if (argc == 1) {
		message = "echo client is sending default message hi";
	} else {
		message = std::string(argv[1]);
	}

	srand(time(NULL));
	while (1) {
		struct timespec ts;
		ts.tv_sec  = 0;
		ts.tv_nsec = 1000000;  // 1ms
		nanosleep(&ts, NULL);

		ssize_t sent = 0;
		do {
			ssize_t ret = send(sockfd, message.data() + sent, message.size() - sent, 0);
			if (ret == -1) {
				perror("send");
				continue;
			}
			sent += ret;
		} while (sent < (ssize_t)message.size());

		char buf[1024];
		ssize_t ret = 0;
		do {
			ret = recv(sockfd, buf, sizeof(buf), 0);
			if (ret == -1) {
				perror("recv");
				break;
			}
			printf("%s\n", std::string(buf, ret).c_str());
		} while (ret <= 0);
	}
}
