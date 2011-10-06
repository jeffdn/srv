/* resp.h
 * Copyright (c) 2007
 * Jeff Nettleton
 */

#ifndef SRV_RESP_H
#define SRV_RESP_H

#include <time.h>

#include <util/hash.h>

#include <srv/req.h>
#include <srv/mod.h>

#define RESP_HTTP_200     2
#define RESP_HTTP_403    19
#define RESP_HTTP_404    20

#define RESP_403_HTML                                           \
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""\
    "    http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"  \
    "<html>"                                                    \
    " <head>"                                                   \
    "  <title>error 403: forbidden</title>"                     \
    "  <style>"                                                 \
    "   body{font-family:courier new;font-size:12;}"            \
    "   p{font-family:courier;font-size:12;}"                   \
    "  </style>"                                                \
    " </head>"                                                  \
    " <body>"                                                   \
    "  <h2>http error 403</h2>"                                 \
    "  <p>access is forbidden to the requested file!</p>"       \
    "   <tr>"                                                   \
    "    <th colspan=\"5\"><hr></th>"                           \
    "   </tr>"                                                  \
    "  <address>server powered by srv 0.1.1</address>"          \
    " </body>"                                                  \
    "</html>"

#define RESP_404_HTML                                           \
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""\
    "    http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"  \
    "<html>"                                                    \
    " <head>"                                                   \
    "  <title>error 404: not found</title>"                     \
    "  <style>"                                                 \
    "   body{font-family:courier new;font-size:12;}"            \
    "  </style>"                                                \
    " </head>"                                                  \
    " <body>"                                                   \
    "  <h2>http error 404</h2>"                                 \
    "  <p>the requested file was not found!</p>"                \
    "   <tr>"                                                   \
    "    <th colspan=\"5\"><hr></th>"                           \
    "   </tr>"                                                  \
    "  <address>server powered by srv 0.1.1</address>"          \
    " </body>"                                                  \
    "</html>"

typedef void *dlptr_t;
typedef char *(*_srv_modfunc_t) (char *,
								 struct srv_mod_trans *, struct req_param *,
								 unsigned int);

struct _modfunc {
	dlptr_t mod;
	_srv_modfunc_t func;
	char path[256];
};

typedef struct _resp_t {
	/* how much have we sent */
	size_t pos;

	unsigned int code;
	char *file;
	unsigned int type;
	char header[256];
	size_t headlen;
	size_t senthead;
	size_t len;

	/* should we cache? */
	unsigned int cache;
	time_t mtime;

	/* if we pregenerate/cache content */
	unsigned int pregen;
	char *data;
} resp_t;

/* pointer to a resp_t */
struct _respptr {
	resp_t *r;
};

/* pregenerate a 404 */
void srv_resp_403(resp_t *);
/* generate/update a resp_t for a cached object */
int srv_resp_cache(resp_t *, const char *);
/* generate a response from a request */
int srv_resp_generate(resp_t *, const char *, const char *,
					  const char *, struct req_param *, unsigned int,
					  hash_t *, hash_t *);
#endif
