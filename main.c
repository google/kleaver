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
#include <string.h>

int main(int argc, char** argv)
{
	/* TODO parse argv */
	env_init();
	config_init();
	if (argc >= 3 && !strcmp(argv[2], "--presubmit"))
		return kleaver_presubmit();
	return kleaver_build();
}
