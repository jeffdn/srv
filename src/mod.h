/* mod.h
 * copyright (c) 2011
 * jeff nettleton
 *   jeffdn@gmail.com
 */

#ifndef SRV_MOD_H
#define SRV_MOD_H

#define SRV_MOD_SUCCESS 1
#define SRV_MOD_FAILURE 0

struct srv_req_param {
	char *key;
	char *val;
};

struct srv_mod_trans {
	int status;
	size_t len;
	int ftype;		/* hacky for now */
};

#endif
