/* conf.c
 * Copyright (c) 2007
 * Jeff Nettleton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

#include <util/util.h>
#include <util/hash.h>

#include "conf.h"

/**
 * check if block opening
 */
int srv_conf_check_block(regex_t * r, const char *buf, char *key, size_t size)
{
	regmatch_t res[2];

	/* lets see if it's a block */
	if (!regexec(r, buf, sizeof res / sizeof res[0], res, 0)) {
		/* it is, lets pull the name */
		strncpy(key, buf + res[1].rm_so,
				((size_t) (res[1].rm_eo - res[1].rm_so) < size) ?
				(size_t) res[1].rm_eo - res[1].rm_so : size);

		return 1;
	}

	/* otherwise, skip */

	return 0;
}

/**
 * parse a line 
 */
int srv_conf_parse_line(regex_t * r, const char *buf, char *key,
						char *val, size_t size)
{
	regmatch_t res[3];

	/* parse the line with rexp */
	if (!regexec(r, buf, sizeof res / sizeof res[0], res, 0)) {
		/* we found a match */
		strncpy(key, buf + res[1].rm_so,
				((size_t) (res[1].rm_eo - res[1].rm_so) < size) ?
				(size_t) res[1].rm_eo - res[1].rm_so : size);

		strncpy(val, buf + res[2].rm_so,
				((size_t) (res[2].rm_eo - res[2].rm_so) < size) ?
				(size_t) res[2].rm_eo - res[2].rm_so : size);
	} else {
		/* no match */
		return 0;
	}

	return 1;
}

/**
 * strip the leading and trailing whitespace from a line
 */
void srv_conf_strip_line(char *buf)
{
	int i;

	/* strip leading whitespace */
	while (isspace(*buf))
		buf++;

	/* strip trailing whitespace */
	for (i = strlen(buf) - 1; i >= 0; i--) {
		if (!isspace(buf[i]))
			break;

		buf[i] = '\0';
	}
}

int srv_conf_handler_module(void *pnt, const char *key, const char *val)
{
	struct _srvmod_conf_t *mods = (struct _srvmod_conf_t *)pnt;

	DEBUGF(__FILE__, __LINE__, "got module config settings: %s, %s\n", key,
		   val);

	/* determine setting */
	if (!strncmp(key, "path", 4)) {
		mods->path = strdup(val);
	} else if (!strncmp(key, "name", 4)) {
		mods->name = strdup(val);
	} else if (!strncmp(key, "func", 4)) {
		mods->func = strdup(val);
	} else if (!strncmp(key, "hnd.", 4)) {
		if (!strncmp(key + 4, "dir", 3))
			mods->hnd[mods->hnd_cnt].type = SRV_HANDLER_DIR;
		else if (!strncmp(key + 4, "ext", 3))
			mods->hnd[mods->hnd_cnt].type = SRV_HANDLER_EXT;
		else if (!strncmp(key + 4, "file", 4))
			mods->hnd[mods->hnd_cnt].type = SRV_HANDLER_FILE;

		/* and we're done */
		mods->hnd[mods->hnd_cnt++].data = strdup(val);
	} else {
		return 0;
	}

	return 1;
}

int srv_conf_process_block(conf_t * conf, const char *blkname,
						   regex_t * r, regex_t * b, FILE * fp)
{
	char buf[256];
	char key[128], val[128];
	regmatch_t res[3];
	void *pnt = NULL;

	/* we'll determine the type */
	int (*srv_conf_block_handler) (void *, const char *, const char *) = NULL;

	DEBUGF(__FILE__, __LINE__, "we entered a block (type:%s)\n", blkname);

	switch (*blkname) {
	case 'v':
		/* new vhost */
		/* srv_conf_block_handler = srv_conf_handler_vhost; */
		/* pnt = conf->vhosts; */
		break;

	case 'm':
		/* new module */
		srv_conf_block_handler = srv_conf_handler_module;
		pnt = &conf->mods[conf->mod_cnt++];
		break;

	case 'a':
		/* new access rule */
		/* srv_conf_block_handler = srv_conf_handler_access; */
		/* pnt = conf->rules; */
		break;

	default:
		return 0;
	}

	while (fgets(buf, sizeof buf, fp)) {
		/* end of block */
		if (buf[0] == '}')
			return 1;

		if (!regexec(b, buf, sizeof res / sizeof res[0], res, 0)) {
			/* they tried to open a new block inside ours */
			ERRF(__FILE__, __LINE__,
				 "must close block before opening another!\n");
			return 0;
		}

		memset(key, '\0', sizeof key);
		memset(val, '\0', sizeof val);

		/* bad line, continue */
		if (!srv_conf_parse_line(r, buf, key, val, sizeof key))
			continue;

		/* handle the values accordingly */
		srv_conf_block_handler(pnt, key, val);

		memset(buf, 0, sizeof buf);
	}

	return 1;
}

/**
 * parse the config
 */
int srv_conf_parse(conf_t * conf, const char *file)
{
	FILE *fp;
	char *buf, line[256];
	char key[128], val[128];
	regex_t blk_r, lin_r;
	int i, open = 0;

	DEBUGF(__FILE__, __LINE__, "starting config parse...\n");

	if (NULL == (fp = fopen(file, "r"))) {
		/* :( */
		ERRF(__FILE__, __LINE__, "error opening %s for parsing!\n", file);
		return 0;
	}

	memset(conf, 0, sizeof *conf);
	memset(&blk_r, 0, sizeof blk_r);
	memset(&lin_r, 0, sizeof lin_r);

	regcomp(&blk_r, "([a-z]+)[[:space:]]*([{])", REG_EXTENDED);
	regcomp(&lin_r, "([a-z._]+)[[:space:]]*=[[:space:]]*\"(.+)\"",
			REG_EXTENDED);

	/* for now we'll just be basic */
	while (fgets(line, sizeof line, fp)) {
		memset(key, 0, sizeof key);
		memset(val, 0, sizeof val);

		buf = line;
		srv_conf_strip_line(buf);

		/* skip the whole line if it's a comment */
		if (*buf == '#' || *buf == '\0')
			continue;

		if (srv_conf_check_block(&blk_r, buf, key, sizeof key)) {
			/* we're entering a block */
			if (!srv_conf_process_block(conf, key, &lin_r, &blk_r, fp)) {
				/* there was an error in the block */
				ERRF(__FILE__, __LINE__, "error processing config!\n");
				exit(1);
			}

			/* ok, block processed successfully, return control */
			continue;
		}

		/* doesn't match */
		if (!srv_conf_parse_line(&lin_r, buf, key, val, sizeof key))
			continue;

		switch (key[0]) {
		case 'i':
			/* index */
			if (NULL != conf->index)
				free(conf->index);

			conf->index = strdup(val);
			DEBUGF(__FILE__, __LINE__, "got index location: %s\n", conf->index);
			break;

		case 'd':
			/* docroot */
			if (NULL != conf->docroot)
				free(conf->docroot);

			conf->docroot = strdup(val);
			DEBUGF(__FILE__, __LINE__, "got docroot path: %s\n", conf->docroot);
			break;

		case 'p':
			/* port */
			if (conf->port_cnt >= SRV_PORT_MAX) {
				/* we're full, so don't add another port */
				ERRF(__FILE__, __LINE__,
					 "host ports limited to %u!\n", SRV_PORT_MAX);
				break;
			}

			conf->ports[conf->port_cnt++] = strtol(val, NULL, 0);
			DEBUGF(__FILE__, __LINE__, "added a port: %d\n",
				   conf->ports[conf->port_cnt]);
			break;

		case 'h':
			if (key[1] == 'o') {
				/* hostname */
				if (NULL != conf->hostname)
					free(conf->hostname);

				conf->hostname = strdup(val);
				DEBUGF(__FILE__, __LINE__, "got a hostname: %s\n",
					   conf->hostname);
			} else if (key[1] == 'i') {
				/* hide this file! */
				if (conf->hide_cnt < SRV_CACHE_MAX)
					conf->hide[conf->hide_cnt++] = strdup(val);
			}
			break;

		case 'j':
			/* chroot jail */
			conf->chroot = (tolower(*val) == 'y'
							|| tolower(*val) == 't') ? 1 : 0;
			break;

#if 0							/* not implemented */
		case 'm':
			/* max_conn */
			conf->max_conn = strtol(val, NULL, 0);
			break;

		case 'c':
			/* conn_time */
			conf->conn_time = strtol(val, NULL, 0);
			break;
#endif

		case 'u':
			/* user */
			if (NULL != conf->user)
				free(conf->user);

			conf->user = strdup(val);
			DEBUGF(__FILE__, __LINE__, "got a username: %s\n", conf->user);
			break;

		case 'g':
			/* group */
			if (NULL != conf->group)
				free(conf->group);

			conf->group = strdup(val);
			DEBUGF(__FILE__, __LINE__, "got a group name: %s\n", conf->group);
			break;

		default:
			break;
		}

		memset(line, 0, sizeof line);
	}

	if (!conf->port_cnt) {
		/* didn't set port val */
		ERRF(__FILE__, __LINE__,
			 "config %s didn't set port, defaulting to 80\n", file);
		conf->ports[0] = 80;
		++conf->port_cnt;
	}

	if (NULL == conf->index) {
		/* didn't set index */
		ERRF(__FILE__, __LINE__,
			 "config %s didn't set index, defaulting " "to index.html\n", file);
		conf->index = strdup("index.html");
	}

	if (NULL == conf->docroot) {
		/* didn't set the docroot */
		ERRF(__FILE__, __LINE__, "no docroot set in %s, exiting!\n", file);
		conf->docroot = strdup("/var/www");
	}
#if 0							/* not implemented */
	if (!conf->conn_time) {
		/* no connection timeout time */
		ERRF(__FILE__, __LINE__,
			 "config %s didn't set max conn time, "
			 "defaulting to 150ms\n", file);
		conf->conn_time = 150;
	}

	if (!conf->max_conn) {
		/* no maximum connection count */
		ERRF(__FILE__, __LINE__,
			 "config %s didn't set maximum connections, "
			 "defaulting to 100\n", file);
		conf->max_conn = 2048;
	}
#endif

	DEBUGF(__FILE__, __LINE__, "config file parsed!\n");

	regfree(&blk_r);
	regfree(&lin_r);
	fclose(fp);

	return 1;
}
