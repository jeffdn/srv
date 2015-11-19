/* conf.h
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

#ifndef SRV_CONF_H
#define SRV_CONF_H

#include <util/hash.h>

#define SRV_PORT_MAX     64
#define SRV_MODULE_MAX   16
#define SRV_HANDLER_MAX  128
#define SRV_CACHE_MAX     512

#define SRV_HANDLER_FILE  0
#define SRV_HANDLER_DIR   1
#define SRV_HANDLER_EXT   2

struct _srvhndlr_conf_t {
    short type;
    char *data;
};

struct _srvmod_conf_t {
    char *name;
    char *path;
    char *func;

    struct _srvhndlr_conf_t hnd[SRV_HANDLER_MAX];
    unsigned int hnd_cnt;
};

/* config def */
typedef struct {
    /* can run on SRV_HOSTS_MAX ports */
    unsigned short ports[SRV_PORT_MAX];
    unsigned int port_cnt;

    /* chroot boolean */
    unsigned int chroot;

    unsigned int hide_cnt;
    char *hide[SRV_CACHE_MAX];
    char *hostname;
    char *docroot;
    char *index;

    /* when running as root */
    char *group;
    char *user;

    /* set up modules */
    struct _srvmod_conf_t mods[SRV_MODULE_MAX];
    unsigned int mod_cnt;

#if 0                            /* not implemented */
    /* number of connections */
    unsigned int max_conn;

    /* kill after... */
    unsigned int conn_time;
#endif
} conf_t;

/* the only public function */
int srv_conf_parse(conf_t *, const char *);

#endif
