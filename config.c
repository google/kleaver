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
#include <kleaver/config.h>
#include <kleaver/dep.h>
#include <kleaver/extcmd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static void get_regexp(const char *name_regex, struct strbuf *output)
{
	struct extcmd cmd;

	extcmd_init(&cmd, "git config");
	extcmd_arg(&cmd, "--file=%s", "Kleaver.config");
	extcmd_arg(&cmd, "--null");
	extcmd_arg(&cmd, "--get-regexp");
	extcmd_arg(&cmd, "%s", name_regex);
	cmd.output = output;
	extcmd_run(&cmd);
	extcmd_release(&cmd);
}

static void get(const char *name, struct strbuf *output)
{
	struct extcmd cmd;

	extcmd_init(&cmd, "git config");
	extcmd_arg(&cmd, "--file=%s", "Kleaver.config");
	extcmd_arg(&cmd, "--null");
	extcmd_arg(&cmd, "--get");
	extcmd_arg(&cmd, "%s", name);
	cmd.output = output;
	extcmd_run(&cmd);
	extcmd_release(&cmd);
}

static bool strbuf_mem_equals(const struct strbuf *sb, const void *buf,
			      size_t len)
{
	return sb->len == len && !memcmp(sb->buf, buf, len);
}

static bool str_mem_equals(const char *str, const void *buf, size_t len)
{
	return strlen(str) == len && !memcmp(str, buf, len);
}

extern struct strbuf pkg_build_cmd;
extern struct strbuf pkg_presubmit_cmd;

void config_init(void)
{
	const char *key, *value, *end, *name, *field;
	size_t remain, keylen, namelen, fieldlen;
	struct strbuf deps = STRBUF_INIT;
	struct dep *dep = NULL;

	get("pkg.buildcmd", &pkg_build_cmd);
	get("pkg.presubmitCmd", &pkg_presubmit_cmd);
	get_regexp("^dep\\.", &deps);
	if (!deps.len)
		return;
	key = deps.buf;
	remain = deps.len;
	while ((end = memchr(key, '\0', remain))) {
		value = memchr(key, '\n', end - key);
		if (value)
			keylen = (value++) - key;
		else
			keylen = end - key;
		name = memchr(key, '.', keylen);
		field = memrchr(key, '.', keylen);
		if (name == field)
			/* TODO implement dep.buildCmd */;
		namelen = (field++) - (++name);
		fieldlen = keylen - namelen - 5;  /* minus "dep" + two dots */
		if (!dep || !strbuf_mem_equals(&dep->name, name, namelen)) {
			dep = malloc(sizeof(struct dep));
			dep_init(dep);
			strbuf_add(&dep->name, name, namelen);
			list_add_tail(&dep->all_deps, &all_deps);
		}
		if (str_mem_equals("repo", field, fieldlen)) {
			if (!value)
				/* TODO report error */;
			strbuf_reset(&dep->repo);
			strbuf_add(&dep->repo, value, end - value);
		} else if (str_mem_equals("branch", field, fieldlen)) {
			if (!value)
				/* TODO report error */;
			strbuf_reset(&dep->branch);
			strbuf_add(&dep->branch, value, end - value);
		} else if (str_mem_equals("tag", field, fieldlen)) {
			if (!value)
				/* TODO report error */;
			strbuf_reset(&dep->tag);
			strbuf_add(&dep->tag, value, end - value);
		} else if (str_mem_equals("buildcmd", field, fieldlen)) {
			if (!value)
				/* TODO report error */;
			strbuf_reset(&dep->build_cmd);
			strbuf_add(&dep->build_cmd, value, end - value);
		} else {
			/* TODO report error */
		}
		remain -= end - key + 1;
		key = end + 1;
	}
	strbuf_release(&deps);
}
