/* req.h
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

#ifndef SRV_REQ_H
#define SRV_REQ_H

#define HTTP_MTHD_GET       0

/* unsupported */
#define HTTP_MTHD_PUT       1
#define HTTP_MTHD_HEAD      2
#define HTTP_MTHD_POST      3
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
