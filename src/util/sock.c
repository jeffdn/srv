/* sock.c
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "util.h"
#include "sock.h"

/**
 * create a new listening socket
 * @param port the port to listen on
 */
sock_t *sock_new(unsigned int proto, unsigned int type, const char *host,
                 unsigned short port)
{
    sock_t *sock;

#ifdef DEBUG
    assert(proto != 0);
#endif

    sock = calloc(1, sizeof *sock);
    if (NULL == sock) {
        ERRF(__FILE__, __LINE__, "error allocating memory for new sock!\n");
        return NULL;
    }

    memset(sock, 0, sizeof *sock);
    if (!sock_init(sock, proto, type, host, port)) {
        ERRF(__FILE__, __LINE__, "error creating socket, aborting...\n");
        exit(1);
    }

    return sock;
}

/**
 * initialize an allocated socket
 * @param sock the socket to initialize
 * @param port the port to listen on
 */
int sock_init(sock_t * sock, unsigned int proto, unsigned int type,
              const char *host, unsigned short port)
{
#ifdef DEBUG
    assert(NULL != sock);
    assert(proto != 0);
    assert(NULL != host);
    assert(0 != port);
#endif

#ifdef WIN32
    WSADATA data;

    if (SOCKET_ERROR == WSAStartup(0x0202, &data)) {
        ERRF(__FILE__, __LINE__, "error starting up Windows sockets!\n");
        return 0;
    }
#endif

    sock->sock = INVALID_SOCKET;
    sock->port = port;
    sock->host = strdup(host);
    memset(sock->ip, 0, sizeof sock->ip);
    sock->proto = proto;
    sock->connected = 0;

    /* clear sockaddr structures */
    memset(&sock->addr, 0, sizeof sock->addr);

    return 1;
}

/**
 * create a socket
 * @param sock the socket to create
 * @param type the type of socket to use;
 * @param host the hostname to use or contact
 * @param port the port to listen on or connect to
 */
int sock_create(sock_t * sock)
{
    int y = 1;

#ifdef DEBUG
    assert(NULL != sock);
#endif

    sock_disconnect(sock);

    /* create the socket */
    if ((sock->sock = socket(PF_INET, sock->proto, 0)) == -1) {
        ERRF(__FILE__, __LINE__, "creating socket: %s!\n", strerror(errno));
        return 0;
    }

    /* get rid of any sockets that are sticking around if we are
     * encroaching on their port */
#ifdef WIN32
    if (setsockopt
        (sock->sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&y,
         sizeof(int)) == -1)
#else
    if (setsockopt(sock->sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int))
        == -1)
#endif
    {
        ERRF(__FILE__, __LINE__,
             "removing any lingering sockets: %s!\n", strerror(errno));
        return 0;
    }

    return 1;
}

/**
 * the bind call
 * @param sock the socket to bind to
 */
int sock_bind(sock_t * sock)
{
#ifdef DEBUG
    assert(NULL != sock);
#endif

    sock_create(sock);

    sock->addr.sin_family = AF_INET;
    sock->addr.sin_port = htons(sock->port);
    sock->addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(sock->addr.sin_zero), '\0', 8);

    /* bind to the port we want */
    if (bind(sock->sock, (struct sockaddr *)&sock->addr,
             sizeof(struct sockaddr)) == -1) {
        ERRF(__FILE__, __LINE__, "binding socket to port %u: %s!\n",
             sock->port, strerror(errno));
        return 0;
    }

    return 1;
}

/**
 * wrap the listen call
 * @param sock the sock to listen on
 */
int sock_listen(sock_t * sock)
{
#ifdef DEBUG
    assert(NULL != sock);
#endif

    /* begin listening on the desired port */
    if (listen(sock->sock, 512) == -1) {
        ERRF(__FILE__, __LINE__, "listening on socket: %s!\n", strerror(errno));
        return 0;
    }

    return 1;
}

/**
 * accept a new connection
 * @param sock the socket to connect on
 */
sock_t *sock_accept(sock_t * sock)
{
    sock_t *client;
    SOCKET tmp_sock;
    struct sockaddr tmp_addr;
    socklen_t sin_size = sizeof sock->addr;

#ifdef DEBUG
    assert(NULL != sock);
#endif

    if (!sock_has_data(sock)) {
        /* don't bother... */
        return NULL;
    }

    if ((tmp_sock =
         accept(sock->sock, (struct sockaddr *)&tmp_addr, &sin_size)) == -1) {
        /* couldn't accept. */
        return NULL;
    }

    client = sock_new(sock->proto, sock->type, sock->host, sock->port);
    sock_destroy(client);

    client->sock = tmp_sock;
    memcpy(&client->addr, &tmp_addr, sizeof client->addr);

    snprintf(client->ip, sizeof client->ip,
             "%s", inet_ntoa(client->addr.sin_addr));

    client->port = ntohs(client->addr.sin_port);

    return client;
}

/**
 * connect to the host
 * @param sock the sock object to use when connecting
 */
int sock_connect(sock_t * sock)
{
    struct hostent *he;

#ifdef DEBUG
    assert(NULL != sock);
    assert(NULL != sock->host);
    assert(0 != sock->port);
#endif

    sock_create(sock);

    if (NULL == (he = gethostbyname(sock->host))) {
        ERRF(__FILE__, __LINE__, "gethostbyname: %s!\n", strerror(errno));
        return 0;
    }

    sock->addr.sin_family = AF_INET;
    sock->addr.sin_port = htons(sock->port);
    sock->addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(sock->addr.sin_zero), '\0', 8);

    if (connect(sock->sock, (struct sockaddr *)&(sock->addr),
                sizeof(struct sockaddr)) == -1) {
        ERRF(__FILE__, __LINE__, "connecting to our host: %s!\n",
             strerror(errno));
        return 0;
    }

    sock->connected = 1;

    return 1;
}

/**
 * check if the socket has data waiting to be read
 * @param sock the socket to check
 */
int sock_has_data(sock_t * sock)
{
    fd_set fds;
    struct timeval tv;

#ifdef DEBUG
    assert(NULL != sock);
#endif

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(sock->sock, &fds);

    if (select((int)(sock->sock + 1), &fds, NULL, NULL, &tv) >= 1) {
        if (!FD_ISSET(sock->sock, &fds)) {
            /* weird, there is data to be read, but not
             * on the only file descriptor.
             */
            return 0;
        }
    } else {
        /* no waiting data. */
        return 0;
    }

    return 1;
}

/**
 * send data to the sock
 * @param sock the sock structure object
 * @param buf the buffer to send
 * @param size the size of the data to send
 */
size_t sock_send(sock_t * sock, const void *buf, size_t size)
{
    int nleft = (signed)size;
    int wrote = 0;

#ifdef DEBUG
    assert(NULL != sock);
    assert(NULL != buf);
    assert(size != 0);
#endif

    while (nleft > 0) {
        if ((wrote = send(sock->sock, (char *)buf + wrote, nleft, 0)) <= 0) {
            if (errno == EINTR) {
                wrote = 0;
            } else {
                ERRF(__FILE__, __LINE__, "sending data! %s\n", strerror(errno));
                return 0;
            }
        }

        nleft -= wrote;
    }

    return size;
}

/**
 * recv some data from the sock!
 * @param sock the sock structure object
 * @param buf a void pointer to a buffer to fille
 * @param size the number of bytes to read into buf
 */
size_t sock_recv(sock_t * sock, void *buf, size_t size)
{
    int got;

#ifdef DEBUG
    assert(NULL != sock);
    assert(size != 0);
#endif

    got = recv(sock->sock, (char *)buf, (int)size, 0);
    if (!got) {
        return 0;
    } else if (got == -1) {
        ERRF(__FILE__, __LINE__, "recieving data! %s\n", strerror(errno));
        return 0;
    }

    return (size_t) got;
}

/**
 * close a socket
 * @param sock the socket to destroy
 */
void sock_disconnect(sock_t * sock)
{
#ifdef DEBUG
    assert(NULL != sock);
#endif

    if (INVALID_SOCKET == sock->sock) {
        /* do nothing. */
        return;
    }

    if (shutdown(sock->sock, SHUT_WR)) {
        if (errno != WSAENOTCONN) {
            ERRF(__FILE__, __LINE__,
                 "shutting down socket(%d)! %s\n", sock->sock, strerror(errno));
            return;
        }
    }

    if (INVALID_SOCKET != sock->sock) {
        if (closesocket(sock->sock)) {
            ERRF(__FILE__, __LINE__, "closing socket! %s\n", strerror(errno));
            return;
        }
    }

    sock->sock = INVALID_SOCKET;
    sock->connected = 0;
}

/**
 * destroy a socket
 * @param sock the socket to destroy
 */
void sock_destroy(sock_t * sock)
{
#ifdef DEBUG
    assert(NULL != sock);
#endif

    sock_disconnect(sock);

#ifdef WIN32
    WSACleanup();
#endif

    sock->sock = INVALID_SOCKET;
    sock->connected = 0;
    sock->type = -1;
    sock->proto = 0;
    sock->port = 0;

    memset(&sock->addr, 0, sizeof sock->addr);
    memset(sock->ip, 0, sizeof sock->ip);
}

/**
 * free a socket
 * @param sock the socket to free
 */
void sock_free(sock_t * sock)
{
#ifdef DEBUG
    assert(NULL != sock);
#endif

    sock_destroy(sock);
    free(sock);
}

/**
 * duplicate a socket
 * @param sock the socket to duplicate
 */
sock_t *sock_dup(sock_t * sock)
{
    sock_t *copy;

#ifdef DEBUG
    assert(NULL != sock);
    assert(0 != sock->port);
    assert(NULL != sock->host);
    assert(!sock->type || sock->type == 1);
#endif

    copy = sock_new(sock->proto, sock->type, sock->host, sock->port);

    return copy;
}
