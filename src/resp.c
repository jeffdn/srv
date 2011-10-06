/* resp.c
 * Copyright (c) 2007
 * Jeff Nettleton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <time.h>

#include <util/hash.h>
#include <util/util.h>

#include <srv/mod.h>
#include <srv/resp.h>

#define MIME_TYPE_CNT 31

/* http response codes */
static const char *resp_status[][2] = {
	{"100", "Continue"},
	{"101", "Switching Protocols"},
	{"200", "OK"},
	{"201", "Created"},
	{"202", "Accepted"},
	{"203", "Non-Authoritative Information"},
	{"204", "No Content"},
	{"205", "Reset Content"},
	{"206", "Partial Content"},
	{"300", "Multiple Choices"},
	{"301", "Moved Permanently"},
	{"302", "Found"},
	{"303", "See Other"},
	{"304", "Not Modified"},
	{"305", "Use Proxy"},
	{"307", "Temporary Redirect"},
	{"400", "Bad Request"},
	{"401", "Unauthorized"},
	{"402", "Payment Required"},
	{"403", "Forbidden"},
	{"404", "Not Found"},
	{"405", "Method Not Allowed"},
	{"406", "Not Acceptable"},
	{"407", "Proxy Authentication Required"},
	{"408", "Request Time Out"},
	{"409", "Conflict"},
	{"410", "Gone"},
	{"411", "Length Required"},
	{"412", "Precondition Failed"},
	{"413", "Request Entity Too Large"},
	{"414", "Request-URI Too Large"},
	{"415", "Unsupported Media Type"},
	{"416", "Requested Range Not Satisfiable"},
	{"417", "Expectation Failed"},
	{"500", "Internal Server Error"},
	{"501", "Not Implemented"},
	{"502", "Bad Gateway"},
	{"503", "Service Unavailable"},
	{"504", "Gateway Time-out"},
	{"505", "HTTP Version Not Supported"}
};

/* the mime types we support */
static const char *mime_types[][2] = {
	{"", "application/octet-stream"},
	{"avi", "video/x-msvideo"},
	{"c", "text/plain"},
	{"conf", "text/plain"},
	{"cpp", "text/plain"},
	{"css", "text/css"},
	{"gif", "image/gif"},
	{"gz", "application/x-gzip"},
	{"h", "text/plain"},
	{"html", "text/html"},
	{"htm", "text/html"},
	{"jpeg", "image/jpeg"},
	{"jpg", "image/jpeg"},
	{"js", "application/javascript"},
	{"m4a", "audio/mp4"},
	{"midi", "audio/midi"},
	{"mp3", "audio/mpeg"},
	{"mpeg", "video/mpeg"},
	{"mpg", "video/mpeg"},
	{"ogg", "application/ogg"},
	{"pdf", "application/pdf"},
	{"php", "text/plain"},		/* for now */
	{"pl", "text/plain"},		/* for now */
	{"png", "image/png"},
	{"sd", "text/plain"},
	{"swf", "application/x-shockwave-flash"},
	{"tiff", "image/tiff"},
	{"txt", "text/plain"},
	{"wav", "audio/x-wav"},
	{"wmv", "video/x-ms-wmv"},
	{"xml", "text/xml"}
};

/* file structure */
typedef struct _file_t {
	char *name;
	char mod[20];
	size_t size;
	int type;
} file_t;

/* list a dir homie */
char *srv_build_dir_index(const char *, file_t *, unsigned int);

/* srv responses */
void srv_resp_403(resp_t * resp)
{
#ifdef DEBUG
	assert(NULL != resp);
#endif

	resp->code = RESP_HTTP_403;
	resp->type = 9;
	resp->len = strlen(RESP_403_HTML);
	resp->pregen = 1;
	resp->data = strdup(RESP_403_HTML);
	snprintf(resp->header, sizeof resp->header,
			 "HTTP/1.1 %s %s\r\n"
			 "Connection: close\r\n"
			 "Server: srv/0.1.1\r\n"
			 "Content-Length: %lu\r\n"
			 "Content-Type: %s\r\n"
			 "\r\n"
			 "\r\n",
			 resp_status[RESP_HTTP_403][0],
			 resp_status[RESP_HTTP_403][1],
			 (long unsigned)resp->len, mime_types[resp->type][1]);
}

void srv_resp_404(resp_t * resp)
{
#ifdef DEBUG
	assert(NULL != resp);
#endif

	resp->code = RESP_HTTP_404;
	resp->type = 9;
	resp->len = strlen(RESP_404_HTML);
	resp->pregen = 1;
	resp->data = strdup(RESP_404_HTML);
	snprintf(resp->header, sizeof resp->header,
			 "HTTP/1.1 %s %s\r\n"
			 "Connection: close\r\n"
			 "Server: srv/0.1.1\r\n"
			 "Content-Length: %lu\r\n"
			 "Content-Type: %s\r\n"
			 "\r\n"
			 "\r\n",
			 resp_status[RESP_HTTP_404][0],
			 resp_status[RESP_HTTP_404][1],
			 (long unsigned)resp->len, mime_types[resp->type][1]);
}

/**
 * get a file's extension, if it has one
 */
char *srv_get_extension(const char *filename)
{
	char *ext;

#ifdef DEBUG
	assert(NULL != filename);
#endif

	ext = strrchr(filename, '.');

	if (NULL == ext)
		return NULL;

	return ++ext;
}

/**
 * private function
 */
int _srv_list_filter(const struct dirent *ent)
{
#ifdef DEBUG
	assert(NULL != ent);
#endif

	if (*ent->d_name == '.')
		return 0;

	return 1;
}

/**
 * fix a path handed to us by the client
 */
char *srv_fix_req_path(const char *root, char *path)
{
	char tmp[1024], *c, *i;

#ifdef DEBUG
	assert(NULL != root);
	assert(NULL != path);
#endif

	memset(tmp, 0, sizeof tmp);
	strncpy(tmp, root, strlen(root) + 1);

	for (c = path, i = tmp + strlen(root); '\0' != *c; c++) {
		if (*c == '.') {
			if (!strncmp(c, "./", 2)) {
				/* dir/dir/./file <-- pointless */
				c++;
				continue;
			} else if (!strncmp(c, "../", 3)) {
				/* dir/dir/../file <-- naughty */
				c += 2;
				continue;
			}
		} else if (*c == '%') {
			if (!strncmp(c, "%20", 3)) {
				/* i'll add more later */
				*i = ' ';
				c += 2;
				i++;
				continue;
			}
		} else if (*c == '/') {
			if (!strncmp(c, "//", 2)) {
				/* no need for a double slash */
				*i = '/';
				c++;
				i++;
				continue;
			}
		}

		*i = *c;
		i++;
	}

	return strdup(tmp);
}

/**
 * list a directory
 */
file_t *srv_list_dir(const char *dir, unsigned int *cnt)
{
	file_t *list, *f;
	struct dirent **ent;
	char tmp[512], *c;
	struct stat st;
	struct tm *tm;
	int i;

#ifdef DEBUG
	assert(NULL != dir);
	assert(NULL != cnt);
#endif

	i = scandir(dir, &ent, _srv_list_filter, alphasort);

	if (i < 0) {
		/* an error occurred! */
		*cnt = 0;
		return NULL;
	}

	*cnt = i;
	list = calloc(*cnt, sizeof *list);

	if (NULL == list) {
		ERRF(__FILE__, __LINE__, "allocation error!\n");
		return NULL;
	}

	/* filename thinnng */
	snprintf(tmp, sizeof tmp, "%s/", dir);
	c = &tmp[strlen(tmp)];		/* right after the '/' */

	/* lets fill our list */
	for (i = 0; i < (signed)*cnt; i++) {
		f = &list[i];

		strcpy(c, ent[i]->d_name);
		stat(tmp, &st);

		tm = localtime(&st.st_mtime);

		f->name = strdup(ent[i]->d_name);
		f->size = st.st_size;
		f->type = st.st_mode;
		snprintf(f->mod, sizeof f->mod,
				 "%02u/%02u/%02u %02u:%02u:%02u", tm->tm_mon + 1,
				 tm->tm_mday, tm->tm_year + 1900, tm->tm_hour,
				 tm->tm_min, tm->tm_sec);

		free(ent[i]);
	}

	return list;
}

/**
 * free a file list
 */
void srv_filelist_free(file_t * list, unsigned int cnt)
{
	unsigned int i;

#ifdef DEBUG
	assert(NULL != list);
	assert(0 != cnt);
#endif

	for (i = 0; i < cnt; i++)
		if (NULL != list[i].name)
			free(list[i].name);

	free(list);
}

int srv_resp_cache(resp_t * resp, const char *path)
{
	int fd;
	size_t got, pos = 0;

	resp->data = calloc(1, resp->len);

	if (NULL == resp->data)
		return 0;

	if (!(fd = open(resp->file, O_RDONLY))) {
		ERRF(__FILE__, __LINE__, "opening file for caching\n");
		return 0;
	}

	while ((got = read(fd, &resp->data[pos], 1024)))
		pos += got;

	resp->pregen = 1;
	close(fd);

	return 1;
}

/**
 * generate a response from a request
 */
int srv_resp_generate(resp_t * resp, const char *root,
					  const char *req, const char *index,
					  struct req_param *params, unsigned int param_cnt,
					  hash_t * hide, hash_t * mps)
{
	struct stat st, ind;
	file_t *list;

	resp_t *cache;
	struct _modfunc *mf;
	unsigned int res, cnt;
	unsigned int i, size;
	struct tm *tm;
	time_t blah;
	char *path, *ext;
	char *ind_path;
	char date[30];

	struct _respptr *pt;
	struct srv_mod_trans mt;

#ifdef DEBUG
	assert(NULL != resp);
	assert(NULL != req);
	assert(NULL != root);
	assert(NULL != index);
#endif

	time(&blah);
	tm = gmtime(&blah);
	mf = NULL;

	/* clear the filename */
	if (resp->file)
		free(resp->file);

	/* clear the pre-existing data */
	if (resp->pregen && resp->data)
		free(resp->data);

	memset(&mt, 0, sizeof mt);
	memset(resp, 0, sizeof *resp);

	strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S GMT", tm);

	path = srv_fix_req_path(root, (char *)req);

	if (NULL != (mf = (struct _modfunc *)hash_get(mps, req))) {
		DEBUGF(__FILE__, __LINE__, "checking if %s needs a handler...\n", path);

		/* gotta handle this bitch with the function */
		if (NULL != (resp->data = mf->func(path, &mt, params, param_cnt))) {
			resp->pregen = 1;
			resp->len = mt.len;
			resp->type = mt.ftype;
			resp->code = (mt.status) ? RESP_HTTP_200 : RESP_HTTP_404;
			snprintf(resp->header, sizeof resp->header,
					 "HTTP/1.1 %s %s\r\n" "Connection: close\r\n"
					 "Date: %s\r\n" "Server: srv/0.1.1\r\n"
					 "Content-Length: %lu\r\n"
					 "Content-Type: %s\r\n" "\r\n",
					 resp_status[resp->code][0],
					 resp_status[resp->code][1], date,
					 (long unsigned)resp->len, mime_types[resp->type][1]);
		} else {
			/* TODO: gotta add error handling */
			srv_resp_404(resp);
		}

		return 1;
	}

	if (stat(path, &st)) {
		/* something happened */
		switch (errno) {
		case EACCES:
			srv_resp_403(resp);
			free(path);
			return 1;

		default:
			srv_resp_404(resp);
			free(path);
			return 1;
		}
	}
#if 0
	/* check if this file is cached, and if so, what's the deal */
	if (NULL != (pt = (struct _respptr *)hash_get(hide, path))) {
		/* get the goodies */
		cache = pt->r;

		if (RESP_HTTP_403 == cache->code) {
			/* we're done! */
			fprintf(stderr, "zoom!\n");
			resp = cache;
			free(path);
			return 1;
		} else if (st.st_mtime > cache->mtime) {
			/* been updated since last cache */
			if (srv_resp_cache(cache, path)) {
				resp = cache;
				resp->mtime = st.st_mtime;
			}

			fprintf(stderr, "sching!\n");

			free(path);
			return 1;
		}
	}
#endif

	if (S_ISDIR(st.st_mode)) {
		ind_path = srv_fix_req_path(path, (char *)index);

		if (!stat(ind_path, &ind)) {
			/* the index exists in this directory */
			resp->len = ind.st_size;
			resp->file = ind_path;	/* keep the name around */

			ext = srv_get_extension(ind_path);
			if (NULL == ext) {
				resp->type = 0;
			} else {
				for (i = 1; i < MIME_TYPE_CNT; i++) {
					if (!strcmp(ext, mime_types[i][0])) {
						/* found it! */
						resp->type = i;
						break;
					}
				}
			}
		} else {
			/* it's a directory so lets list that shiiit */
			list = srv_list_dir(path, &cnt);

			if (NULL == list) {
				/* couldn't build it? */
				return 0;
			}

			resp->pregen = 1;
			resp->data = srv_build_dir_index(req, list, cnt);
			resp->len = strlen(resp->data);
			resp->type = 9;		/* text/html */

			srv_filelist_free(list, cnt);
			free(ind_path);		/* lose the path */
		}
	} else {
		resp->len = st.st_size;

#if 0
		/* should we cache? */
		if (resp->len <= 512000)
			resp->cache = 1;
#endif

		resp->file = strdup(path);

		ext = srv_get_extension(path);
		if (NULL == ext) {
			resp->type = 0;
		} else {
			for (i = 1; i < MIME_TYPE_CNT; i++) {
				if (!strcmp(ext, mime_types[i][0])) {
					/* found it! */
					resp->type = i;
					break;
				}
			}
		}
	}

	resp->code = RESP_HTTP_200;
	snprintf(resp->header, sizeof resp->header,
			 "HTTP/1.1 %s %s\r\n"
			 "Connection: close\r\n"
			 "Date: %s\r\n"
			 "Server: srv/0.1.1\r\n"
			 "Content-Length: %lu\r\n"
			 "Content-Type: %s\r\n"
			 "\r\n",
			 resp_status[RESP_HTTP_200][0],
			 resp_status[RESP_HTTP_200][1],
			 date, (long unsigned)resp->len, mime_types[resp->type][1]);

#if 0
	/* cache it */
	if (resp->cache) {
		srv_resp_cache(resp, path);
		hash_insert(hide, path, resp);
		fprintf(stderr, "foop!\n");
	}
#endif

	free(path);
	return 1;
}

/**
 * build a directory index page
 */
char *srv_build_dir_index(const char *dir, file_t * list, unsigned int cnt)
{
	unsigned int i;
	char size[16];
	char buf[64 * 1024], ent[256];	/* 20 kb, huge directory */
	file_t *f;

	/* the beginning of our html */
	snprintf(buf, sizeof buf,
			 "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">"
			 "\n"
			 "<html>\n"
			 " <head>\n"
			 "  <title>index of %s</title>\n"
			 "  <style>\n"
			 "   body {\n"
			 "    font-family: courier new;\n"
			 "    font-size: 12;\n"
			 "   }\n"
			 "   table {\n"
			 "    font-family: courier;\n"
			 "    font-size: 12;\n" "   }\n" "  </style>\n" " </head>\n", dir);
	snprintf(buf + strlen(buf), sizeof buf - strlen(buf),
			 " <body>\n"
			 "  <h2>index of %s</h2>\n"
			 "  <table>\n"
			 "   <tr>\n"
			 "    <th align=\"left\" width=\"200\">name</th>\n"
			 "    <th align=\"left\" width=\"150\">last modified</th>\n"
			 "    <th align=\"left\" width=\"35\">size</th>\n"
			 "   </tr>\n   <tr><th colspan=\"5\"><hr></th></tr>", dir);

	for (i = 0; i < cnt; i++) {
		memset(ent, 0, sizeof ent);
		memset(size, 0, sizeof size);

		f = &list[i];

		/* if it's a dir, put dir, else put the size */
		if (S_ISDIR(f->type)) {
			snprintf(size, sizeof size, "dir");
		} else {
			if (f->size < 1024) {
				snprintf(size, sizeof size, "%lub", (long unsigned)f->size);
			} else if (f->size >= 1024 && f->size < 1048576) {
				snprintf(size, sizeof size, "%.1fk", (float)f->size / 1024);
			} else {
				snprintf(size, sizeof size, "%.1fm", (float)f->size / 1048576);
			}
		}

		/* this part of the html */
		snprintf(ent, sizeof ent,
				 "   <tr>\n"
				 "    <td><a href=\"%s/%s\">%s</a></td>\n"
				 "    <td>%s</td>\n"
				 "    <td>%s</td>\n"
				 "   </tr>\n",
				 !strcmp(dir, "/") ? "" : dir, list[i].name,
				 list[i].name, list[i].mod, size);

		strcat(buf, ent);
	}

	/* the end of the html block */
	snprintf(ent, sizeof ent,
			 "   <tr>\n"
			 "    <th colspan=\"5\"><hr></th>\n"
			 "   </tr>\n"
			 "  </table>\n"
			 "  <address>server powered by srv 0.1.1</address>\n"
			 " </body>\n" "</html>\n");
	strcat(buf, ent);

	return strdup(buf);
}
