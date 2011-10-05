/* mod_test.c
 * copyright (c) 2011
 * jeff nettleton
 *     jeffdn@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>

#include <srv/mod.h>

#define HEAD                                                    \
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""\
    "    http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"  \
    "<html>"                                                    \
    " <head>"                                                   \
    "  <title>mod_test.so -- srv 0.1.1</title>"                 \
    "  <style>"                                                 \
    "   body{font-family:courier new;font-size:12;}"            \
    "  </style>"                                                \
    " </head>"                                                  \
    " <body>"                                                   \
    "  <p>"

#define TAIL                                                    \
    "</p>"                                                      \
    " </body>"                                                  \
    "</html>"

char *handle_mre(char *path, struct srv_mod_trans *mt,
                 struct srv_req_param *params, unsigned int cnt)
{
    char *data, buf[16384];
    unsigned int i, len;
    int got, fd;

    memset(buf, '\0', sizeof buf);

    if (!(fd = open(path, O_RDONLY)))
        return NULL;

    got = read(fd, buf, sizeof buf);
    close(fd);

    if (buf[0] == '\0')
        return NULL;

    len = strlen(HEAD) + got + strlen(TAIL) + 14;

    for (i = 0; i < cnt; i++) {
        /* print out params */
        len += strlen(params[i].key) + strlen(params[i].val) + 9;
    }

    data = calloc(1, len);

    if (NULL == data)
        return NULL;

    snprintf(data, len, "%s<h3>%s</h3>", HEAD, buf);

    for (i = 0; i < cnt; i++) {
        strcat(data, params[i].key);
        strcat(data, ":");
        strcat(data, params[i].val);
        strcat(data, "<br/>");
    }

    strcat(data, "\r\n\r\n");

    mt->ftype = 9;
    mt->status = SRV_MOD_SUCCESS;
    mt->len = len;

    return data;
}
