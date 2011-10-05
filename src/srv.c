/* srv.c
 * Copyright (c) 2007
 * Jeff Nettleton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <regex.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <event.h>

#include <util/util.h>
#include <util/hash.h>
#include <util/stack.h>
#include <util/thread.h>

#include <srv/conn.h>
#include <srv/conf.h>
#include <srv/resp.h>
#include <srv/req.h>

#define SRV_VHOST_MAX 128
#define SRV_TPOOL_MAX 16
#define SRV_CONN_MAX  128

struct _workunit {
    int fd;
};

#if 0
#define DEBUG
#endif

/* declare our globals */

/* our global config */
static conf_t conf;
/* the pool of connections */
static conn_t pool[SRV_CONN_MAX];
/* thread pool */
static tpool_t tp;
/* list of paths and the module to execute with */
static hash_t mps;
/* modules */
static struct _modfunc mods[SRV_MODULE_MAX];

/* declare our handler functions */

/* accept a new connection */
void srv_accept_new_conn(int, short, void *);
/* notify th/ead pool of activity */
void srv_conn_handle_activity(int, short, void *);
/* threadpool handler function */
void *srv_threadpool_handler(void *);
/* the client has sent their request */
int srv_conn_req_ready(conn_t *);
/* we've successfully generated our response */
int srv_conn_resp_ready(conn_t *);
/* send some pregenerated data */
int srv_conn_send_pregen(conn_t *);
/* send data from a file, not pregen'd */
int srv_conn_send_file(conn_t *);

/* end declarations */

void *srv_modhash_alloc(const void *arg)
{
    struct _modfunc *mf = (struct _modfunc *)arg;

    return (void *)mf;
}

void srv_modhash_free(void *a)
{
    struct _modfunc *mf = (struct _modfunc *)a;

    mf->mod = NULL;
    mf->func = NULL;
    memset(mf->path, '\0', sizeof mf->path);
}

int srv_modhash_valcmp(const void *a, const void *b)
{
    struct _modfunc *mfa = (struct _modfunc *)a;
    struct _modfunc *mfb = (struct _modfunc *)b;

    return (mfa->func == mfb->func) ? 0 : 1;
}

void *srv_threadpool_alloc(void *arg)
{
    struct _workunit *wu = calloc(1, sizeof *wu);

    if (NULL == wu) {
        DEBUGF(__FILE__, __LINE__, "allocation error\n");
        return NULL;
    }

    /* either set value, or empty */
    if (NULL != arg)
        wu->fd = (int)arg;
    else
        wu->fd = 0;

    return (void *)wu;
}

/**
 * accept a new connection
 */
void srv_accept_new_conn(int fd, short ev, void *arg)
{
    int tmp_sock, y = 1;
    struct sockaddr_in tmp_addr;
    socklen_t socklen;
    conn_t *clnt;

    /* accept the connection */
    socklen = sizeof tmp_addr;
    if ((tmp_sock = accept(fd, (struct sockaddr *)&tmp_addr, &socklen)) == -1) {
        ERRF(__FILE__, __LINE__, "accepting conn: %s, %d!\n",
             strerror(errno), fd);
        return;
    }

    if (tmp_sock >= (signed)sizeof pool) {
        /* too many concurrent connections */
        ERRF(__FILE__, __LINE__, "accepting conn: too many!\n");
        close(tmp_sock);
        return;
    }

    /* make the socket non-blocking */
    if (fcntl(tmp_sock, F_SETFL, O_NONBLOCK) == -1) {
        /* failure */
        ERRF(__FILE__, __LINE__, "non blocking: %s!\n", strerror(errno));
        close(tmp_sock);
        return;
    }

    /* unset TCP_NODELAY, because that makes shit slower for sends */
#ifdef TCP_CORK
    if (setsockopt(tmp_sock, IPPROTO_TCP, TCP_CORK, &y, sizeof y) == -1) {
        /* couldn't set TCP_CORK */
        ERRF(__FILE__, __LINE__, "cork: %s!\n", strerror(errno));
        close(tmp_sock);
        return;
    }
#endif

    /* set up the clnt shit */
    clnt = &pool[tmp_sock];
    clnt->sock = tmp_sock;
    memcpy(&clnt->addr, &tmp_addr, sizeof clnt->addr);

    clnt->state = CONN_STATE_REQ;

    event_set(&clnt->ev, clnt->sock,
              EV_READ | EV_PERSIST, srv_conn_handle_activity, NULL);
    event_add(&clnt->ev, NULL);
}

/**
 * handle activity on a socket
 */
void srv_conn_handle_activity(int fd, short ev, void *arg)
{
    event_del(&pool[fd].ev);
    tpool_add_work(&tp, (void *)fd);
}

/* threadpool controller */
void *srv_threadpool_controller(void *arg)
{
    unsigned int i;
    struct _worker_t *w;
    struct _workunit *wu;

    w = &tp.boss;

    for (;;) {
        wu = NULL;

        /* block until there is work */
        pthread_mutex_lock(&w->mt);

        while (NULL != (wu = tpool_get_work(&tp))) {
            /* there is a pending job */
            for (i = 0; i < tp.cnt; i++) {
                /* find an available thread */
                if (!tp.pool[i].busy) {
                    /* got one */
                    DEBUGF(__FILE__, __LINE__,
                           "(thread:%d) taking a job...\n", tp.pool[i].id);

                    /* assign work, unlock mutex */
                    tp.pool[i].job = wu->fd;
                    free(wu);
                    wu = NULL;

                    /* pull the trigger */
                    pthread_mutex_unlock(&tp.pool[i].mt);
                    break;
                }
            }

            if (tp.cnt == i) {
                /* there were no available threads, throw back on stack */
                tpool_add_work(&tp, (void *)wu->fd);
                free(wu);
                wu = NULL;
            }
        }
    }

    return NULL;
}

/* threadpool handler thread for all threads in the pool */
void *srv_threadpool_handler(void *arg)
{
    struct _worker_t *w;
    conn_t *clnt;

    /* who am i? */
    w = &tp.pool[(int)arg];

    /* infinite loop */
    for (;;) {
        /* block until we have work */
        pthread_mutex_lock(&w->mt);

        /* let's do this */
        w->busy = 1;

        if (w->job) {
            clnt = &pool[w->job];

            /* figure out what we're supposed to do */
            if (CONN_STATE_REQ == clnt->state) {
                /* ready to read the client's request */
                if (!srv_conn_req_ready(clnt)) {
                    DEBUGF(__FILE__, __LINE__,
                           "(sock:%d) problem with parsing request!\n",
                           clnt->sock);
                } else {
                    /* prepare socket for write-ready signal */
                    event_set(&clnt->ev, clnt->sock,
                              EV_WRITE | EV_PERSIST,
                              srv_conn_handle_activity, NULL);
                    event_add(&clnt->ev, NULL);
                }
            } else if (CONN_STATE_RESP == clnt->state) {
                /* ready to send the HTTP response to the client */
                if (!srv_conn_resp_ready(clnt)) {
                    DEBUGF(__FILE__, __LINE__,
                           "(sock:%d) problem with sending response!\n",
                           clnt->sock);
                    srv_conn_cleanup(clnt);
                } else {
                    /* prepare socket for write-ready signal */
                    event_set(&clnt->ev, clnt->sock,
                              EV_WRITE | EV_PERSIST,
                              srv_conn_handle_activity, NULL);
                    event_add(&clnt->ev, NULL);
                }
            } else if (CONN_STATE_SEND == clnt->state) {
                /* ready to send our data to the client */
                if (!clnt->resp.pregen) {
                    /* sending a file */
                    if (!srv_conn_send_file(clnt)) {
                        DEBUGF(__FILE__, __LINE__,
                               "(sock:%d) problem sending file!\n", clnt->sock);
                    }
                } else {
                    /* sending pregenerated data, i.e. dir listing */
                    if (!srv_conn_send_pregen(clnt)) {
                        DEBUGF(__FILE__, __LINE__,
                               "(sock:%d) problem sending pregen!\n",
                               clnt->sock);
                    }
                }

                /* and we're done! */
                srv_conn_cleanup(clnt);
            } else {
                /* and we're done! */
                srv_conn_cleanup(clnt);
            }
        }

        w->job = 0;
        w->busy = 0;
        clnt = NULL;
    }

    return NULL;
}

/**
 * read a clnt's request
 */
int srv_conn_req_ready(conn_t * clnt)
{
    ssize_t got;

#ifdef DEBUG
    assert(NULL != clnt);
#endif

    /* get some more shit */
    got = recv(clnt->sock, clnt->req.buf, sizeof clnt->req.buf, 0);

    if (got == -1) {
        /* erreur! */
        ERRF(__FILE__, __LINE__, "receiving data: %s!\n", strerror(errno));
        return 0;
    }

    if (sizeof clnt->req.buf == got) {
        /* request the maximum size? fuck you */
        ERRF(__FILE__, __LINE__,
             "buffer overflow attempt? killing connection.\n");
        return 0;
    }

    /* got rid of allocation */
    if (!srv_req_parse(&clnt->req)) {
        /* bad request, disconnect */
        ERRF(__FILE__, __LINE__, "bad request.\n");
        return 0;
    }

    /* got rid of allocation */
    if (!srv_resp_generate(&clnt->resp, conf.docroot,
                           clnt->req.path, conf.index, clnt->req.params,
                           clnt->req.param_cnt, &mps)) {
        /* couldn't build the response? */
        ERRF(__FILE__, __LINE__, "error generating response.\n");
        return 0;
    }

    clnt->resp.headlen = strlen(clnt->resp.header);
    clnt->state = CONN_STATE_RESP;

    return 1;
}

/**
 * send the header
 */
int srv_conn_resp_ready(conn_t * clnt)
{
    ssize_t sent;

#ifdef DEBUG
    assert(NULL != clnt);
#endif

    if (!(sent = send(clnt->sock, clnt->resp.header, clnt->resp.headlen, 0))) {
        ERRF(__FILE__, __LINE__, "(sock:%d) sending error...\n", clnt->sock);
        return 0;
    } else if (sent == -1) {
        /* errno is set */
        switch (errno) {
        case EAGAIN:
        case EINTR:
        case EPIPE:
        default:
            /* problem */
            ERRF(__FILE__, __LINE__,
                 "(sock:%d) unrecoverable send error", clnt->sock);
            return 0;
        }
    }

    /* now lets send the data! */
    if (!clnt->resp.pregen) {
        /* we're sending a file */
        if (!(clnt->fd = open(clnt->resp.file, O_RDONLY))) {
            ERRF(__FILE__, __LINE__, "opening file for sending!\n");
            return 0;
        }
    }

    clnt->state = CONN_STATE_SEND;

    return 1;
}

/**
 * send the pregenerated data (403/404/dir listing)
 */
int srv_conn_send_pregen(conn_t * clnt)
{
    ssize_t sent;

#ifdef DEBUG
    assert(NULL != clnt);
#endif

    /* send some of the data */
    while (clnt->resp.pos < clnt->resp.len) {
        if (!(sent = send(clnt->sock,
                          &clnt->resp.data[clnt->resp.pos],
                          clnt->resp.len - clnt->resp.pos, 0))) {
            /* failure :'( */
            ERRF(__FILE__, __LINE__, "send: %s!\n", strerror(errno));
            return 0;
        } else if (sent == -1) {
            /* errno is set */
            switch (errno) {
            case EAGAIN:
            case EINTR:
                /* we'll wait and try again */
                DEBUGF(__FILE__, __LINE__,
                       "(sock:%d) minor send issue, retrying\n", clnt->sock);
                continue;

            case EPIPE:
            default:
                /* problem */
                ERRF(__FILE__, __LINE__, "send: %s!\n", strerror(errno));
                return 0;
            }
        }

        /* advance our shit */
        clnt->resp.pos += sent;
    }

    clnt->state = CONN_STATE_DESTROY;

    return 1;
}

/**
 * send a file
 */
int srv_conn_send_file(conn_t * clnt)
{
    ssize_t sent = 0;
    ssize_t total = 0;
    ssize_t bytes = 0;
    ssize_t count = 0;
    ssize_t toget = 0;
    char buf[4096];

#ifdef DEBUG
    assert(NULL != clnt);
#endif

    toget = sizeof buf;
    memset(buf, '\0', sizeof buf);

    while ((bytes = read(clnt->fd, buf, toget))) {
        while (bytes) {
            if (!(sent = send(clnt->sock, &buf[count], bytes, 0))) {
                /* failure :'( */
                ERRF(__FILE__, __LINE__, "send: failed!\n");
                return 0;
            } else if (sent == -1) {
                /* errno is set */
                switch (errno) {
                case EAGAIN:
                case EINTR:
                    /* back that ass up */
                    continue;

                case EPIPE:
                default:
                    /* problem */
                    ERRF(__FILE__, __LINE__, "send: %s!\n", strerror(errno));
                    return 0;
                }
            }

            /* subtract the amt we sent */
            count += sent;
            bytes -= sent;
            total += sent;

            toget =
                ((clnt->resp.len - total) >
                 sizeof buf) ? sizeof buf : clnt->resp.len - total;
        }

        /* reset bytes to 0 */
        memset(buf, '\0', sizeof buf);
        bytes = count = sent = 0;
    }

    clnt->state = CONN_STATE_DESTROY;
    DEBUGF(__FILE__, __LINE__, "(sock:%d) sent %db, made it!\n",
           clnt->sock, total);

    return 1;
}

/**
 * lets do this
 */
int main(int argc, char *argv[])
{
    unsigned int i, j;
    struct passwd *user;
    struct group *group;

    /* lets set our shit up */
    if (argc > 1) {
        /* they specified a config */
        if (!srv_conf_parse(&conf, argv[1])) {
            /* invalid */
            ERRF(__FILE__, __LINE__, "invalid config: %s!\n", argv[1]);
            return 1;
        }
    } else {
        if (!srv_conf_parse(&conf, "srv.conf")) {
            /* invalid */
            ERRF(__FILE__, __LINE__, "invalid config: srv.conf!\n");
            return 1;
        }
    }

    /* dump a bunch of startup info */
    printf("srv 0.1.1\n");
    printf("copyright (c) 2007, 2011\n");
    printf("jeff nettleton\n");
    printf("jeffdn@gmail.com\n\n");
    printf("  port(s):  ");

    for (i = 0; i < conf.port_cnt; i++) {
        /* all the ports we're on */
        printf(" %u", conf.ports[i]);
    }

    printf("\n");
    printf("  docroot:   %s\n", conf.docroot);
    printf("  index:     %s\n", conf.index);
    printf("  hostname:  %s\n", conf.hostname);
    printf("  chroot:    %s\n", (conf.chroot) ? "yes" : "no");

    /* if we're running as root... */
    if (conf.chroot) {
        printf("  user:      %s\n", conf.user);
        printf("  group:     %s\n", conf.group);
    }

    printf("  module(s): ");

    for (i = 0; i < conf.mod_cnt; i++)
        printf("%s ", conf.mods[i].name);

    printf("%s\n", (conf.mod_cnt) ? " " : "none");

    /* initialize libevent */
    event_init();

    /* set up all the ports while we have permission */
    for (i = 0; i < conf.port_cnt; i++) {
        /* set up our host */
        if (!srv_conn_init(&pool[i], conf.ports[i])) {
            /* something got fucked up */
            ERRF(__FILE__, __LINE__,
                 "error creating socket on %u!\n", conf.ports[i]);
            return 1;
        }
    }

    /* set up our hashtable */
    hash_init(&mps, SRV_MODULE_MAX);

    /* key functions... basic string type */
    hash_set_keycmp(&mps, hash_default_keycmp);
    hash_set_keycpy(&mps, hash_default_keycpy);
    hash_set_free_key(&mps, hash_default_free_key);

    /* val functions... pointer into module array */
    hash_set_valcmp(&mps, srv_modhash_valcmp);
    hash_set_valcpy(&mps, srv_modhash_alloc);
    hash_set_free_val(&mps, srv_modhash_free);

    /* set up our modules, insert paths into hashtable */
    for (i = 0; i < conf.mod_cnt; ++i) {
        /* get ready for it */
        memset(&mods[i], '\0', sizeof mods[i]);

        /* get filename together */
        snprintf(mods[i].path, sizeof mods[i].path, "%s/%s.so",
                 conf.mods[i].path, conf.mods[i].name);

        /* load module */
        DEBUGF(__FILE__, __LINE__, "loading module %s from %s...\n",
               conf.mods[i].name, mods[i].path);
        mods[i].mod = dlopen(mods[i].path, RTLD_LAZY);

        if (NULL == mods[i].mod) {
            ERRF(__FILE__, __LINE__, "couldn't load module %s!\n");
            continue;
        }

        /* get our function symbol */
        DEBUGF(__FILE__, __LINE__, "loading symbol %s from %s...\n",
               conf.mods[i].func, mods[i].path);
        mods[i].func = (_srv_modfunc_t) dlsym(mods[i].mod, conf.mods[i].func);

        if (NULL == mods[i].func) {
            ERRF(__FILE__, __LINE__, "couldn't get function %s\n",
                 conf.mods[i].func);
            continue;
        }

        for (j = 0; j < conf.mods[i].hnd_cnt; j++) {
            /* insert into hash */
            if (conf.mods[i].hnd[j].type == SRV_HANDLER_FILE) {
                /* add this path to handler hash */
                DEBUGF(__FILE__, __LINE__,
                       "request path %s to be handled with %s...\n",
                       conf.mods[i].hnd[j].data, conf.mods[i].func);
                hash_insert(&mps, conf.mods[i].hnd[j].data, &mods[i]);
            }
        }
    }

    /* everything else we don't need to do as root */
    if (!geteuid()) {
        if (NULL != conf.user && NULL != conf.group) {
            /* get the user info */
            user = getpwnam(conf.user);

            if (NULL == user) {
                ERRF(__FILE__, __LINE__, "invalid user %s!\n", conf.user);
                return 1;
            }

            /* get the group info */
            group = getgrnam(conf.group);

            if (NULL == group) {
                ERRF(__FILE__, __LINE__, "invalid group %s!\n", conf.group);
                return 1;
            }

            /* successfully got the user/group data */

            /* do they want to chroot? we need to do this before
             * switching our user and group.
             */
            if (conf.chroot) {
                if (chdir(conf.docroot) == -1) {
                    /* fail */
                    ERRF(__FILE__, __LINE__,
                         "could not chdir to %s: %s!\n",
                         conf.docroot, strerror(errno));
                    return 1;
                }

                if (chroot(conf.docroot) == -1) {
                    /* fail */
                    ERRF(__FILE__, __LINE__,
                         "could not chroot to %s: %s!\n",
                         conf.docroot, strerror(errno));
                    return 1;
                }

                /* we successfully chrooted.  now we want to
                 * make our path to files / instead of whatever
                 * the docroot was previously
                 */
                free(conf.docroot);
                conf.docroot = strdup("/");
            }

            /* now become someone non-root, for security reasons */

            /* try to join the group */
            if (setgid(group->gr_gid) == -1) {
                ERRF(__FILE__, __LINE__,
                     "could not join group %s: %s!\n",
                     conf.group, strerror(errno));
                return 1;
            }

            /* try to assume the new identity */
            if (setuid(user->pw_uid) == -1) {
                ERRF(__FILE__, __LINE__,
                     "could not become user %s: %s!\n",
                     conf.user, strerror(errno));
                return 1;
            }
        } else {
            fprintf(stderr,
                    "i'm running as root, but you haven't provided a\n"
                    "non-root user and group to switch to.  this is\n"
                    "insecure, so set the variables 'user' and 'group'\n"
                    "in the config file. thanks!\n");
            return 1;
        }
    }

    /* set up the thread pool handler */
    tpool_init(&tp, SRV_TPOOL_MAX, srv_threadpool_controller,
               srv_threadpool_handler);
    stack_set_data_alloc(&tp.work, srv_threadpool_alloc);
    stack_set_data_free(&tp.work, free);

    /* now set up handlers for when we have incoming connections! */
    for (i = 0; i < conf.port_cnt; i++) {
        /* set up the accept event */
        event_set(&pool[i].ev, pool[i].sock,
                  EV_READ | EV_PERSIST, srv_accept_new_conn, NULL);
        event_add(&pool[i].ev, NULL);
    }

    /* begin our main loop */
    event_dispatch();

    /* this is kinda useless but fuck it */
    for (i = 0; i < conf.port_cnt; i++) {
        srv_conn_cleanup(&pool[i]);
    }

    return 0;
}
