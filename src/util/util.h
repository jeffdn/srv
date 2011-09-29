/* util.h
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#ifndef BASE_UTIL_H
#define BASE_UTIL_H

#ifdef WIN32
	/* why must windows be so fucking gay? */
#define snprintf _snprintf

#ifdef _DEBUG
#define DEBUG
#endif
#endif

/* print an error message */
void ERRF(const char *, unsigned int, const char *, ...);
void DEBUGF(const char *, unsigned int, const char *, ...);

#endif
