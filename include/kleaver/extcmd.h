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
#include <git/strbuf.h>

struct extcmd {
	struct strbuf	command;
	struct strbuf	*output;
	int		exit_status;
};

extern void extcmd_init(struct extcmd *cmd, const char *cmdline);
extern void extcmd_release(struct extcmd *cmd);
extern void extcmd_arg(struct extcmd *cmd, const char *fmt, ...);
extern void extcmd_run(struct extcmd *cmd);
