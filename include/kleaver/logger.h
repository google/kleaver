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
#pragma once

extern void logger(const char *file, int line, const char *func, char level,
		   const char *fmt, ...) __attribute__((format(printf, 5, 6)));

#define LOG_FATAL(fmt, args...)						\
	do {								\
		logger(__FILE__, __LINE__, __func__, 'F', fmt, ##args);	\
		exit(1);						\
	} while (0)

#define LOG_ERROR(fmt, args...)						\
	logger(__FILE__, __LINE__, __func__, 'E', fmt, ##args)

#define LOG_WARN(fmt, args...)						\
	logger(__FILE__, __LINE__, __func__, 'W', fmt, ##args)

#define LOG_INFO(fmt, args...)						\
	logger(__FILE__, __LINE__, __func__, 'I', fmt, ##args)

#define PLOG_FATAL(fmt, args...) LOG_FATAL(fmt ": %s", ##args, strerror(errno))
#define PLOG_ERROR(fmt, args...) LOG_ERROR(fmt ": %s", ##args, strerror(errno))

#define CHECK(cond, fmt, args...) if (!(cond)) LOG_FATAL(fmt, ##args)
