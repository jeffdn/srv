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
