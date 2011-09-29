/* sock.h
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#ifndef UTIL_SOCK_H
#define UTIL_SOCK_H

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#define SHUT_WR SD_SEND
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define INVALID_SOCKET  (-1)
#define closesocket(s)  close(s)
#define WSAENOTCONN     ENOTCONN

typedef int SOCKET;
#endif

/* the purpose of this header file is to abstract
 * out the combined ugliness of socket code and the
 * non-cross platform aspect of it. *shudder*
 */

#define SOCK_TYPE_HOST 		0
#define SOCK_TYPE_CLIENT 	1

#define sock_connected(s)   ((s)->connected)
#define sock_host(s)        ((s)->host)
#define sock_type(s)        ((s)->type)
#define sock_port(s)        ((s)->port)
#define sock_sock(s)        ((s)->sock)
#define sock_ip(s)          ((s)->ip)

typedef struct _sock_t {
	unsigned int type;
	unsigned int proto;

	/* local vars */
	unsigned short port;
	char *host;
	char ip[16];
	struct sockaddr_in addr;
	SOCKET sock;

	unsigned int connected;
} sock_t;

/* create a new listening socket */
sock_t *sock_new(unsigned int, unsigned int, const char *, unsigned short);
/* initialise an allocated sock */
int sock_init(sock_t *, unsigned int, unsigned int, const char *,
	      unsigned short);
/* choose what kind of socket we're having */
int sock_create(sock_t *);
/* accept an incoming connection */
sock_t *sock_accept(sock_t *);
/* bind to a port */
int sock_bind(sock_t *);
/* listen on a socket */
int sock_listen(sock_t *);
/* connect to the host */
int sock_connect(sock_t *);
/* disconnect from the host */
void sock_disconnect(sock_t *);
/* check if the socket has data waiting */
int sock_has_data(sock_t *);
/* send to a host */
size_t sock_send(sock_t *, const void *, size_t);
/* receive from a host */
size_t sock_recv(sock_t *, void *, size_t);
/* destroy a sock */
void sock_destroy(sock_t *);
/* free a sock */
void sock_free(sock_t *);

/* duplicate a socket */
sock_t *sock_dup(sock_t *);

#endif
