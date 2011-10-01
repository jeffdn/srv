/* conn.h
 * Copyright (c) 2007
 * Jeff Nettleton
 */

#ifndef SRV_CONN_H
#define SRV_CONN_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <event.h>

#include <srv/resp.h>
#include <srv/req.h>

#define CONN_STATE_NEW      1
#define CONN_STATE_REQ      2
#define CONN_STATE_RESP     3
#define CONN_STATE_SEND     4
#define CONN_STATE_DESTROY  0

typedef struct _conn_t {
	int sock;
	struct sockaddr_in addr;
	unsigned int locked;
	unsigned int state;

	/* our file */
	int fd;

	/* client only */
	struct event ev;
	req_t req;
	resp_t resp;
} conn_t;

/* intialize a connection */
int srv_conn_init(conn_t *, unsigned short);
/* disconnect */
void srv_conn_cleanup(conn_t *);

#endif
