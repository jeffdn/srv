/* mod_test.c
 * copyright (c) 2011
 * jeff nettleton
 *     jeffdn@gmail.com
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
