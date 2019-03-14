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
#include <git/strbuf.h>
#include <kleaver/env.h>
#include <stdlib.h>
#include <unistd.h>

const char *HOME;
const char *PWD;
const char *XDG_CACHE_HOME;

void env_init(void)
{
	HOME = getenv("HOME");
	if (!HOME)
		/* TODO report error */;

	PWD = getenv("PWD");
	if (!PWD)
		PWD = get_current_dir_name();
	if (!PWD)
		/* TODO report error */;

	/* https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html */
	XDG_CACHE_HOME = getenv("XDG_CACHE_HOME");
	if (!XDG_CACHE_HOME)
		XDG_CACHE_HOME = xstrfmt("%s/.cache", HOME);
}
