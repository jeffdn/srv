/* req.h
 * Copyright (c) 2007
 * Jeff Nettleton
 */

#ifndef SRV_REQ_H
#define SRV_REQ_H

#define HTTP_MTHD_GET       0

/* unsupported */
#define HTTP_MTHD_PUT       1
#define HTTP_MTHD_HEAD      2
/* unsupported */

#define SRV_REQ_PARAM_MAX   64
#define SRV_REQ_MAX_LEN     1024

struct req_param {
	char *key;
	char *val;
};

typedef struct _req_t {
	/* where were we? */
	char buf[1024];
	size_t pos;

	/* requested file/dir */
	char *path;
	/* only GET for now */
	unsigned int meth;
	/* http/1.[01] */
	unsigned int type;
	/* list of ?key=val params */
	struct req_param params[SRV_REQ_PARAM_MAX];
	unsigned int param_cnt;

	/* header directives */
	unsigned int close;
	char from[64];
	char ref[128];
	char ua[128];

	/* http/1.1 only */
	unsigned short port;
	char host[64];
} req_t;

/* parse a request */
unsigned int srv_req_parse(req_t *);

#endif
