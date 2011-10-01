/* util.c
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/**
 * print an error message
 * @param file the file that the error is in
 * @param line the line that the error is on
 * @param format the format to print the error message in
 * @param ... the format's corresponding arguments
 */
void ERRF(const char *file, unsigned int line, const char *format, ...)
{
    va_list ap;

#ifdef DEUBG
    assert(NULL != file);
    assert(NULL != format);
#endif

    va_start(ap, format);
    fprintf(stderr, "%s:%u - ERROR: ", file, line);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

/**
 * print a debugging message
 * @param file the file that the message is located in
 * @param line the line that the message is located in
 * @param format the format to print the debug message in
 * @param ... the format's corresponding arguments
 */
void DEBUGF(const char *file, unsigned int line, const char *format, ...)
{
#ifdef DEBUG
    va_list ap;

    assert(NULL != file);
    assert(NULL != format);

    va_start(ap, format);
    fprintf(stderr, "%s:%u - DEBUG: ", file, line);
    vfprintf(stderr, format, ap);
    va_end(ap);
#endif
}
