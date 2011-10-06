/* conn.c
 * Copyright (c) 2007
 * Jeff Nettleton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <util/util.h>

#include <srv/conn.h>

/**
 * initialize a connection
 */
int srv_conn_init(conn_t * conn, unsigned short port)
{
	int y = 1;

#ifdef DEBUG
	assert(NULL != conn);
	assert(port > 0);
#endif

	conn->sock = -1;
	conn->state = CONN_STATE_NEW;
	conn->locked = 0;

	if ((conn->sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		/* problems creating the socket */
		ERRF(__FILE__, __LINE__, "creating socket: %s!\n", strerror(errno));
		return 0;
	}

	if (setsockopt(conn->sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof y) == -1) {
		/* couldn't clear up a lingering socket */
		ERRF(__FILE__, __LINE__, "clearing socket: %s!\n", strerror(errno));
		close(conn->sock);
		return 0;
	}

	/* these aren't portable, so check if they are around 
	 * before we start dicking around with setting them.
	 */

#ifdef TCP_DEFER_ACCEPT
	/* keep listener sleeping until when data arrives */
	if (setsockopt(conn->sock, IPPROTO_TCP, TCP_DEFER_ACCEPT,
				   &y, sizeof y) == -1) {
		/* couldn't set TCP_DEFER_ACCEPT */
		ERRF(__FILE__, __LINE__, "defer accept: %s!\n", strerror(errno));
		close(conn->sock);
		return 0;
	}
#endif

#ifdef TCP_NODELAY
	/* since all this does is accept connections */
	if (setsockopt(conn->sock, IPPROTO_TCP, TCP_NODELAY, &y, sizeof y) == -1) {
		/* couldn't set TCP_NODELAY */
		ERRF(__FILE__, __LINE__, "nodelay: %s!\n", strerror(errno));
		close(conn->sock);
		return 0;
	}
#endif

	/* make the socket non-blocking */
	if (fcntl(conn->sock, F_SETFL, O_NONBLOCK) == -1) {
		/* failure */
		ERRF(__FILE__, __LINE__, "non blocking: %s!\n", strerror(errno));
		close(conn->sock);
		return 0;
	}

	conn->addr.sin_family = AF_INET;
	conn->addr.sin_port = htons(port);
	conn->addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(conn->addr.sin_zero), '\0', 8);

	/* lets see if we can bind to the port we want */
	if (bind(conn->sock, (struct sockaddr *)&conn->addr,
			 sizeof(struct sockaddr)) == -1) {
		ERRF(__FILE__, __LINE__, "bind: %s!\n", strerror(errno));
		close(conn->sock);
		return 0;
	}

	/* lets see if we can listen on this port now */
	if (listen(conn->sock, SOMAXCONN) == -1) {
		ERRF(__FILE__, __LINE__, "listen: %s!\n", strerror(errno));
		close(conn->sock);
		return 0;
	}

	return 1;
}

/**
 * close a socket
 */
void srv_conn_cleanup(conn_t * conn)
{
#ifdef DEBUG
	assert(NULL != conn);
#endif

	if (-1 == conn->sock) {
		/* do nothing. */
		return;
	}

	if (conn->state != CONN_STATE_DESTROY)
		DEBUGF(__FILE__, __LINE__,
			   "(sock:%d) preemptive close of connection\n", conn->sock);

	/* nicer way to close instead of straight disconnect */
	if (shutdown(conn->sock, SHUT_WR)) {
		if (errno != ENOTCONN) {
			ERRF(__FILE__, __LINE__,
				 "shutting down socket(%d)! %s\n", conn->sock, strerror(errno));
			return;
		}
	}

	/* if the shutdown wasn't respected... */
	if (-1 != conn->sock) {
		if (close(conn->sock)) {
			ERRF(__FILE__, __LINE__, "closing socket! %s\n", strerror(errno));
			return;
		}
	}

	if (conn->fd) {
		/* our filehandle still open */
		close(conn->fd);
	}

	conn->fd = 0;
	conn->sock = -1;
	conn->state = CONN_STATE_NEW;
	conn->locked = 0;

	memset(&conn->addr, 0, sizeof &conn->addr);
}
