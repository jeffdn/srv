/* req.c
 * Copyright (c) 2007
 * Jeff Nettleton
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <srv/req.h>

#include <util/util.h>
#include <util/utstring.h>

#define REQ_LINES        15
#define HTTP_ESC_CNT     22
#define MAX_REPLACE_ITER 128

/* HTTP escape codes */
static const char *http_esc_codes[][2] = {
    {"%20", " "},
    {"%3C", "<"},
    {"%3E", ">"},
    {"%23", "#"},
    {"%25", "%"},
    {"%7B", "{"},
    {"%7D", "}"},
    {"%7C", "|"},
    {"%5C", "\\"},
    {"%5E", "^"},
    {"%7E", "~"},
    {"%5B", "["},
    {"%5D", "]"},
    {"%60", "`"},
    {"%3B", ";"},
    {"%2F", "/"},
    {"%3F", "?"},
    {"%3A", ":"},
    {"%40", "@"},
    {"%3D", "="},
    {"%26", "&"},
    {"%24", "$"}
};

/**
 * parse a request
 */
unsigned int srv_req_parse(req_t * req)
{
    char *str, *copy, *tmp, *pm, *pmcopy;
    char *pa, *pb;
    unsigned int i;

#ifdef DEBUG
    assert(NULL != req);
#endif

    if (NULL != req->path)
        free(req->path);

    req->param_cnt = 0;
    req->port = 0;
    req->pos = 0;

    copy = strdup((char *)req->buf);

    /* lets take care of the first line */
    str = strsep(&copy, "\r\n");

    if (NULL == str) {
        /* wtf? */
        ERRF(__FILE__, __LINE__, "empty request!\n");

        if (NULL != copy)
            free(copy);

        return 0;
    }

    /* first parse the GET line */
    if (!strncmp(str, "GET", 3)) {
        req->meth = HTTP_MTHD_GET;
        str += 4;
#if 0
    } else if (!strncmp(str, "PUT", 3)) {
        req->meth = HTTP_MTHD_PUT;
        str += 4;
    } else if (!strncmp(str, "POST", 4)) {
        req->meth = HTTP_MTHD_POST;
        str += 5;
    } else if (!strncmp(str, "HEAD", 4)) {
        req->meth = HTTP_MTHD_HEAD;
        str += 5;
#endif
    } else {
        /* unsupported */
        return 0;
    }

    /* which http/x.x is this? */
    if (NULL == (tmp = strchr(str, '.')))
        return 0;

    req->type = (*tmp == '1') ? 1 : 0;

    /* lose the HTTP/x.x directive */
    str[strlen(str) - 9] = '\0';

    /* check to see if there are any escapes */
    if (NULL != strchr(str, '%')) {
        /* make a copy */
        pa = strdup(str);

        for (i = 0; i < HTTP_ESC_CNT; i++) {
            /* fix up the path */
            pb = strreplace(pa, http_esc_codes[i][0],
                            http_esc_codes[i][1], MAX_REPLACE_ITER);

            /* clean it up */
            free(pa);
            pa = pb;
        }

        /* copy back */
        str = strdup(pa);
    }

    if (NULL != (pmcopy = strchr(str, '?'))) {
        /* remove the shit from the path */
        str[strlen(str) - (strlen(pmcopy))] = '\0';
        req->path = strdup(str);
        ++pmcopy;

        if (NULL != pmcopy) {
            while (NULL != (pm = strsep(&pmcopy, "&")) &&
                   req->param_cnt < SRV_REQ_PARAM_MAX) {
                if (!strlen(pm))
                    continue;

                if (NULL == (tmp = strchr(pm, '=')))
                    continue;

                req->params[req->param_cnt].val = strdup(tmp + 1);

                /* slightly tricksy */
                req->params[req->param_cnt].key =
                    calloc(1, strlen(pm) + 1 - strlen(tmp));
                snprintf(req->params[req->param_cnt].key,
                         (int)((strlen(pm) + 1) - strlen(tmp)),
                         "%.*s", (int)(strlen(pm) - strlen(tmp)), pm);

                /* increment the count */
                req->param_cnt++;
            }
        }
    } else {
        /* no parameters */
        req->path = strdup(str);
    }

    while (NULL != (str = strsep(&copy, "\r\n"))) {
        if (!strlen(str))
            continue;

        if (!strncmp(str, "User-Agent", 10)) {
            /* user agent directive */
            tmp = strchr(str, ' ');
            strncpy(req->ua, ++tmp, sizeof req->ua);
        } else if (!strncmp(str, "Referer", 7)) {
            /* the referer */
            tmp = strchr(str, ' ');
            strncpy(req->ref, ++tmp, sizeof req->ref);
        } else if (!strncmp(str, "From", 4)) {
            /* who sent that shit son */
            tmp = strchr(str, ' ');
            strncpy(req->from, ++tmp, sizeof req->from);
        } else if (!strncmp(str, "Connection", 10)) {
            /* close when we're done? */
            tmp = strchr(str, ' ');

            if (!strncmp(++tmp, "close", 5))
                req->close = 1;
            else
                req->close = 0;
        } else if (!strncmp(str, "Host", 4)) {
            /* host/port comboooo */
            str += 5;

            while (*str == ' ')
                str++;

            /* the :port */
            tmp = strchr(str, ':');
            if (NULL != tmp) {
                req->port = strtol(++tmp, NULL, 0);

                /* the host: (-port len + -1 for the ':') */
                str[strlen(str) - (strlen(tmp) + 1)] = '\0';
                strncpy(req->host, str, sizeof req->host);
            } else {
                /* there was no port specified */
                req->port = 80;
                strncpy(req->host, str, sizeof req->host);
            }
        } else {
            /* we don't support it yet */
            continue;
        }
    }

    free(copy);

    return 1;
}
