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
#include <ctype.h>
#include <git/strbuf.h>
#include <kleaver/build.h>
#include <kleaver/dep.h>
#include <kleaver/env.h>
#include <kleaver/extcmd.h>
#include <kleaver/flag.h>
#include <kleaver/logger.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct strbuf pkg_build_cmd = STRBUF_INIT;
struct strbuf pkg_presubmit_cmd = STRBUF_INIT;

static struct strbuf pkg_commit = STRBUF_INIT;

static DEFINE_string(pkg_commit, NULL, "package commit hash");

static bool is_valid_commit(const char *commit)
{
	int i, len;

	len = strlen(commit);
	if (len != 40)
		return false;
	for (i = 0; i < len; i++)
		if (!isxdigit(commit[i]))
			return false;
	return true;
}

static void init_pkg_commit(void)
{
	struct extcmd cmd;

	if (FLAG_pkg_commit) {
		if (!is_valid_commit(FLAG_pkg_commit))
			LOG_FATAL("invalid --pkg_commit %s", FLAG_pkg_commit);
		strbuf_reset(&pkg_commit);
		strbuf_addstr(&pkg_commit, FLAG_pkg_commit);
		return;
	}
	extcmd_init(&cmd, "git rev-parse HEAD");
	cmd.output = &pkg_commit;
	extcmd_run(&cmd);
	extcmd_release(&cmd);
	strbuf_trim_trailing_newline(&pkg_commit);
}

static void expand(const struct strbuf *build_cmd, const struct dep *dep,
		   struct strbuf *cmd)
{
	int i, ch, replace = 0;

	strbuf_reset(cmd);
	for (i = 0; i < build_cmd->len; i++) {
		ch = build_cmd->buf[i];
		if (replace) {
			if (ch == '%')
				strbuf_addch(cmd, ch);
			else if (ch == 'K')
				strbuf_addbuf(cmd, &dep->worktree);
			else if (ch == 'O')
				strbuf_addbuf(cmd, &dep->build_dir);
			else if (ch == 'M')
				strbuf_addstr(cmd, PWD);
			else if (ch == 'C')
				strbuf_addbuf(cmd, &pkg_commit);
			else if (ch == 'H')
				strbuf_addbuf(cmd, &dep->commit);
			else
				/* TODO report error */;
			replace = 0;
			continue;
		}
		if (ch == '%') {
			replace = 1;
			continue;
		}
		strbuf_addch(cmd, ch);
	}
	if (replace)
		/* TODO report error */;
}

static int kleaver_presubmit(void)
{
	struct strbuf expanded_cmd = STRBUF_INIT;
	struct strbuf output = STRBUF_INIT;
	struct extcmd cmd;
	struct dep *dep;

	list_for_each_entry(dep, &all_deps, all_deps) {
		dep_resolve_ref(dep);
		strbuf_reset(&expanded_cmd);
		strbuf_reset(&output);
		expand(&pkg_presubmit_cmd, dep, &expanded_cmd);
		extcmd_init(&cmd, expanded_cmd.buf);
		cmd.output = &output;
		extcmd_run(&cmd);
		extcmd_release(&cmd);
		/* TODO puts() adds an extra new line.
		 * We need to revamp the whole extcmd things in the near future.
		 * It's currently fundamentally broken.
		 */
		puts(output.buf);
		if (!cmd.ok)
			break;
	}
	strbuf_release(&expanded_cmd);
	strbuf_release(&output);
	return cmd.ok ? 0 : -1;
}

static DEFINE_bool(presubmit, false, "Use presubmitCmd instead of buildCmd");

int kleaver_build(void)
{
	struct strbuf build_cmd = STRBUF_INIT;
	struct extcmd cmd;
	struct dep *dep;

	init_pkg_commit();
	if (FLAG_presubmit)
		return kleaver_presubmit();
	/* TODO avoid fetching a repo more than once */
	list_for_each_entry(dep, &all_deps, all_deps)
		dep_fetch(dep);
	list_for_each_entry(dep, &all_deps, all_deps)
		dep_checkout(dep);
	list_for_each_entry(dep, &all_deps, all_deps) {
		extcmd_init(&cmd, "mkdir -p");
		extcmd_arg(&cmd, "%.*s", dep->build_dir.len, dep->build_dir.buf);
		extcmd_run(&cmd);
		extcmd_release(&cmd);
		if (!cmd.ok)
			break;

		expand(&dep->build_cmd, dep, &build_cmd);
		extcmd_init(&cmd, build_cmd.buf);
		extcmd_run(&cmd);
		extcmd_release(&cmd);
		if (!cmd.ok)
			break;

		expand(&pkg_build_cmd, dep, &build_cmd);
		extcmd_init(&cmd, build_cmd.buf);
		extcmd_run(&cmd);
		extcmd_release(&cmd);
		if (!cmd.ok)
			break;
	}
	strbuf_release(&build_cmd);
	return cmd.ok ? 0 : -1;
}
