/* string.h
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#ifndef UTIL_STRING_H
#define UTIL_STRING_H

/* just some basic string functions... */

/* split a string */
char **strsplit(const char *, const char *, unsigned int, unsigned int *);
/* join a split string together */
char *strjoin(const char **, const char *, unsigned int);
/* get a segment of a string */
char *strsubstr(const char *, unsigned int, int);
/* replace part of a string with another */
char *strreplace(char *, const char *, const char *, unsigned int);

#endif
