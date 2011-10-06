/* util.c
 * Copyright (c) 2006
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
