/* test.c
 * copyright (c) 2011
 * jeff nettleton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <util/sock.h>
#include <util/util.h>

#define SRV_TEST_LEVEL 8
#define SRV_TEST_ITERS 1024

void *srv_test_handler(void *arg)
{
    char *request;
    char buf[1024];
    unsigned int i;
    sock_t *s = (sock_t *)arg;

    request = strdup("GET / HTTP/1.0\r\n\r\n");
    printf("request: %s", request);

    for (i = 0; i < SRV_TEST_ITERS; i++) {
        memset(buf, '\0', sizeof buf);
        
        if (!sock_connect(s))
            ERRF(__FILE__, __LINE__, "connecting to host!\n");

        if (!sock_send(s, request, strlen(request)))
            ERRF(__FILE__, __LINE__, "sending request!\n");

        usleep(500);

        if (!sock_recv(s, buf, sizeof buf))
            ERRF(__FILE__, __LINE__, "getting response!\n");

        sock_disconnect(s);

        printf("%s\n", buf);
    }

    free(request);

    return NULL;
}

int main(int argc, char *argv[])
{
    char *host;
    unsigned short port, i;

    sock_t sock[SRV_TEST_LEVEL];
    pthread_t p[SRV_TEST_LEVEL];
    
    if (argc != 3) {
        ERRF(__FILE__, __LINE__, "./srvtest hostname port\n");
        return 1;
    }

    host = strdup(argv[1]);
    port = (unsigned short) strtol(argv[2], NULL, 0);

    for (i = 0; i < SRV_TEST_LEVEL; i++) {
        /* get these threads rollin' */
        sock_init(&sock[i], SOCK_STREAM, SOCK_TYPE_CLIENT, host, port);
        pthread_create(&p[i], NULL, srv_test_handler, (void *)&sock[i]);
        printf("pew!\n");
    }

    free(host);
    usleep(1000000);

    return 0;
}
