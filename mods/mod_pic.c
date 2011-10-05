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

char *handle_pic(char *name, struct srv_mod_trans *mt,
                 struct srv_req_param *params, unsigned int cnt)
{
    char path[256];
    char *data, buf[1638400];
    unsigned int i, len;
    int got, fd = 0;

    memset(buf, '\0', sizeof buf);
    memset(path, '\0', sizeof path);

    for (i = 0; i < cnt; ++i) {
        if (!strncmp(params[i].key, "name", 4)) {
            snprintf(path, sizeof path,
                     "/home/jeff/srv/site/pics/%s.jpeg", params[i].val);
        } else {
            mt->status = SRV_MOD_FAILURE;
            return NULL;
        }
    }

    if (!(fd = open(path, O_RDONLY))) {
        mt->status = SRV_MOD_FAILURE;
        return NULL;
    }

    got = read(fd, buf, sizeof buf);
    close(fd);

    if (buf[0] == '\0') {
        mt->status = SRV_MOD_FAILURE;
        return NULL;
    }

    data = calloc(1, got);
    memcpy(data, buf, got);

    if (NULL == data) {
        mt->status = SRV_MOD_FAILURE;
        return NULL;
    }

    mt->ftype = 12;
    mt->status = SRV_MOD_SUCCESS;
    mt->len = got;

    return data;
}
