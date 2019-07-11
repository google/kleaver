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
#include <kleaver/extcmd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void extcmd_init(struct extcmd *cmd, const char *cmdline)
{
	memset(cmd, 0, sizeof(*cmd));
	strbuf_init(&cmd->command, 0);
	strbuf_addstr(&cmd->command, cmdline);
}

void extcmd_release(struct extcmd *cmd)
{
	strbuf_release(&cmd->command);
}

void extcmd_arg(struct extcmd *cmd, const char *fmt, ...)
{
	va_list ap;

	strbuf_addstr(&cmd->command, " '");
	va_start(ap, fmt);
	strbuf_vaddf(&cmd->command, fmt, ap);
	va_end(ap);
	strbuf_addch(&cmd->command, '\'');
}

void extcmd_run(struct extcmd *cmd)
{
	struct strbuf s = STRBUF_INIT, *output = &s;
	FILE *p;

	p = popen(cmd->command.buf, "r");
	if (!p)
		/* TODO report error */;
	if (cmd->output)
		output = cmd->output;
	strbuf_reset(output);
	while (strbuf_fread(output, 1000, p) > 0)
		;
	/* TODO handle errors properly */
	cmd->exit_status = pclose(p);
	strbuf_release(&s);
}
