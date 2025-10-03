#include "rio.h"
#include "utils.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "8080"
#define BACKLOG 5

int main() {
  int status;
  int sockfd;
  int connfd;
  struct addrinfo hints;
  struct addrinfo *server_info;
  struct addrinfo *p;
  struct sockaddr *addr;
  struct sockaddr_storage client_addr;        // connector's address info
  socklen_t client_len = sizeof(client_addr); // connector's storage size
  char hostname[MAXLINE], port [MAXLINE];

  // get addrinfo
  // reset hints memory space
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;     // ipv4 or ipv6
  hints.ai_socktype = SOCK_STREAM; // use TCP
  hints.ai_flags = AI_PASSIVE;     // use local IP

  // don't pass a hostname, the server_info will have that for us?
  if ((status = getaddrinfo(NULL, PORT, &hints, &server_info) != 0)) {
    printf("get address info error: %s", gai_strerror(status));
    exit(1);
  };

  // loop through addinfo list and
  for (p = server_info; p != NULL; p = p->ai_next) {
    //  a. try to create socket
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    //  b. try to bind to socket
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(server_info);

  // We couldn't create or bind to a socket in all of the available addrinfo
  // structs
  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  // once bound, listen on socket
  if (listen(sockfd, BACKLOG) == -1) {
    perror("server: listen");
    exit(1);
  }

  printf("Server listening on port %s\n", PORT);

  // accept connections
  while (1) {
    connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    getnameinfo((struct sockaddr *) &client_addr, client_len, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    /* deal with the request */
    doit(connfd);
    close(connfd);
  }
  return 0;
}
