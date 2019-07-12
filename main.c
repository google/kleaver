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
	/* TODO parse argv */
	env_init();
	config_init();
	if (argc == 2 && !strcmp(argv[1], "selftests"))
		return kleaver_selftests();
	if (argc >= 3 && !strcmp(argv[2], "--presubmit"))
		return kleaver_presubmit();
	return kleaver_build();
}
