/* conn.h
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
