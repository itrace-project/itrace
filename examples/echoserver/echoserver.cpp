/*
 * Echo server example for testing itrace
 *
 * All clients who connect to the server will receive messages sent by all other
 * clients
 *
 * Compile with make echoserver
 * */
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

// C-style linkage to avoid symbol mangling by C++ compiler
extern "C" std::string get_client_message(int sockfd) {
	char buf[1024];
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_size = sizeof(peer_addr);
	ssize_t rcvd;

	if ((rcvd = recvfrom(
	         sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&peer_addr,
	         &peer_addr_size
	     )) == -1) {
		perror("recvfrom");
		close(sockfd);
		exit(1);
	}

	return std::string(buf, rcvd);
}

extern "C" void echo(
    std::string message, const std::vector<std::pair<int, std::string>>& clients
) {
	for (const auto& client : clients) {
		int sockfd   = client.first;
		ssize_t sent = 0;
		do {
			ssize_t ret =
			    send(sockfd, message.data() + sent, message.size() - sent, 0);
			if (ret == -1) {
				perror("send");
				return;
			}
			sent += ret;
		} while (sent < (ssize_t)message.size());
	}
}

int main() {
	struct addrinfo* res;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));  // Make sure to do this
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;
	if (getaddrinfo(NULL, "34900", &hints, &res) != 0) {
		perror("getaddrinfo");
		exit(1);
	}

	int listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (listenfd == -1) {
		perror("socket");
		exit(1);
	}

	int yes = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) ==
	        -1 ||
	    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes))) {
		perror("setsockopt");
		exit(1);
	}

	if (bind(listenfd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("bind");
		close(listenfd);
		exit(1);
	}

	if (listen(listenfd, 10) == -1) {
		perror("listen");
		close(listenfd);
		exit(1);
	}

	std::vector<std::pair<int, std::string>> clients {};
	while (1) {
		fd_set readfds;
		FD_ZERO(&readfds);

		int maxfd = listenfd;
		FD_SET(listenfd, &readfds);
		for (const auto& client : clients) {
			FD_SET(client.first, &readfds);
			maxfd = std::max(maxfd, client.first);
		}

		if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1) {
			perror("select");
			continue;
		}

		if (FD_ISSET(listenfd, &readfds)) {
			struct sockaddr_storage addr;
			socklen_t size = sizeof(addr);
			int connfd     = accept(listenfd, (struct sockaddr*)&addr, &size);

			char addr_str[INET6_ADDRSTRLEN];
			if (addr.ss_family == AF_INET) {
				struct sockaddr_in* in_addr = (struct sockaddr_in*)&addr;
				inet_ntop(
				    in_addr->sin_family, &in_addr, addr_str, sizeof(addr_str)
				);
				printf(
				    "new connection from IPv4 %s on fd %d\n", addr_str, connfd
				);
			} else {
				struct sockaddr_in6* in6_addr = (struct sockaddr_in6*)&addr;
				inet_ntop(
				    in6_addr->sin6_family, &in6_addr, addr_str, sizeof(addr_str)
				);
				printf(
				    "new connection from IPv6 %s on fd %d\n", addr_str, connfd
				);
			}
			clients.push_back({connfd, addr_str});
		}

		for (const auto& client : clients) {
			int sockfd = client.first;
			if (FD_ISSET(sockfd, &readfds)) {
				std::string message = get_client_message(sockfd);
				std::string prefix  = client.second + " says: ";
				message.insert(0, prefix);
				std::cout << message << std::endl;
				echo(message, clients);
			}
		}
	}
}
