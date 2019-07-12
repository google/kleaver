/* Copyright 2019 Google LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <kleaver/logger.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

/* Generic logging function. Thread-safe.
 *
 * Log lines have this form:
 *     Lmmdd hh:mm:ss.uuuuuu thrdid file:line] func: msg...
 * where the fields are defined as follows:
 *   L			A single character, representing the log level
 *   mm			The month (zero padded)
 *   dd			The day (zero padded)
 *   hh:mm:ss.uuuuuu	Time in hours, minutes and fractional seconds
 *   thrdid		The space-padded thread ID as returned by gettid()
 *   file		The file name
 *   line		The line number
 *   func		The calling function name
 *   msg		The user-supplied message
 */
void logger(const char *file, int line, const char *func, char level,
	    const char *fmt, ...)
{
	char buf[4096], *msg = buf, *path;
	int size, thread_id;
	struct timespec ts;
	struct tm t;
	va_list args;

	va_start(args, fmt);
	/* vsnprintf returns # of chars excluding the terminating NULL byte */
	size = vsnprintf(buf, sizeof(buf), fmt, args) + 1;
	if (size > sizeof(buf)) {
		msg = malloc(size);
		vsnprintf(msg, size, fmt, args);
	}
	va_end(args);
	clock_gettime(CLOCK_REALTIME, &ts);
	localtime_r(&ts.tv_sec, &t);
	thread_id = syscall(SYS_gettid);
	if (thread_id == -1)
		thread_id = getpid();
	path = strdup(file);
	fprintf(stderr, "%c%02d%02d %02d:%02d:%02d.%06ld %6d %s:%d] %s: %s\n",
		level, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
		ts.tv_nsec / 1000, thread_id, basename(path), line, func, msg);
	free(path);
	if (size > sizeof(buf))
		free(msg);
	/* TODO dump stack trace if FATAL */
	if (level == 'F' || level == 'E' || level == 'W')
		fflush(stderr);
	if (level == 'F' || level == 'E')
		fflush(stdout);
	if (level == 'F')
		fcloseall();
}
