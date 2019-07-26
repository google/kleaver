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
#include <kleaver/build.h>
#include <kleaver/config.h>
#include <kleaver/env.h>
#include <kleaver/extcmd.h>
#include <kleaver/flag.h>
#include <kleaver/logger.h>
#include <stdlib.h>
#include <string.h>

static int kleaver_selftests(void)
{
	LOG_INFO("This is an INFO log line: %d", 42);
	LOG_WARN("This is a warning.");
	LOG_ERROR("This is an error.");
	LOG_FATAL("This is a fatal error!");
	LOG_FATAL("This must be unreachable.");
}

int main(int argc, char** argv)
{
	int flags_offset;

	flags_offset = flag_init(argc, argv);
	argc -= flags_offset;
	argv += flags_offset;
	env_init();
	config_init();
	if (!strcmp(argv[0], "selftests"))
		return kleaver_selftests();
	return kleaver_build();
}
