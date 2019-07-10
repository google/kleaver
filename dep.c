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
#include <kleaver/dep.h>
#include <kleaver/env.h>
#include <kleaver/extcmd.h>
#include <linux/list.h>
#include <stdlib.h>
#include <string.h>

LIST_HEAD(all_deps);

void dep_init(struct dep *dep)
{
	memset(dep, 0, sizeof(*dep));
	INIT_LIST_HEAD(&dep->all_deps);
	strbuf_init(&dep->name, 0);
	strbuf_init(&dep->repo, 0);
	strbuf_init(&dep->branch, 0);
	strbuf_init(&dep->tag, 0);
	strbuf_init(&dep->worktree, 0);
	strbuf_init(&dep->build_cmd, 0);
	strbuf_init(&dep->build_dir, 0);
	strbuf_init(&dep->commit, 0);
}

void dep_release(struct dep *dep)
{
	strbuf_release(&dep->name);
	strbuf_release(&dep->repo);
	strbuf_release(&dep->branch);
	strbuf_release(&dep->tag);
	strbuf_release(&dep->worktree);
	strbuf_release(&dep->build_cmd);
	strbuf_release(&dep->build_dir);
	strbuf_release(&dep->commit);
}

static void git_init(const char *git_dir)
{
	struct extcmd cmd;

	extcmd_init(&cmd, "git init");
	extcmd_arg(&cmd, "%s", git_dir);
	extcmd_run(&cmd);
	extcmd_release(&cmd);
}

static void git_config_remove_section(const char *config_file,
				      const char *section_name)
{
	struct extcmd cmd;

	extcmd_init(&cmd, "git config");
	extcmd_arg(&cmd, "--file=%s", config_file);
	extcmd_arg(&cmd, "--remove-section");
	extcmd_arg(&cmd, "%s", section_name);
	extcmd_run(&cmd);
	extcmd_release(&cmd);
}

static void git_config_add(const char *config_file, const char *key,
			   const char *value)
{
	struct extcmd cmd;

	extcmd_init(&cmd, "git config");
	extcmd_arg(&cmd, "--file=%s", config_file);
	extcmd_arg(&cmd, "--add");
	extcmd_arg(&cmd, "%s", key);
	extcmd_arg(&cmd, "%s", value);
	extcmd_run(&cmd);
	extcmd_release(&cmd);
}

static void git_fetch(const char *git_dir, const char *remote_name)
{
	struct extcmd cmd;

	extcmd_init(&cmd, "git");
	extcmd_arg(&cmd, "--git-dir=%s", git_dir);
	extcmd_arg(&cmd, "fetch");
	extcmd_arg(&cmd, "%s", remote_name);
	extcmd_run(&cmd);
	extcmd_release(&cmd);
}

static char *repo_remote_name(const struct strbuf *repo)
{
	struct strbuf name = STRBUF_INIT;
	int i, last = -1;

	for (i = 0; i < repo->len; i++) {
		if (repo->buf[i] == ':')
			continue;
		if (repo->buf[i] == '/' && last == '/')
			continue;
		strbuf_addch(&name, repo->buf[i]);
		last = repo->buf[i];
	}
	return strbuf_detach(&name, NULL);
}

void dep_fetch(struct dep *dep)
{
	char *remote, *section, *url, *fetch, *tag_opt, *tags, *heads;
	char *git, *git_dir, *config;

	if (!(dep->repo.len && (dep->branch.len || dep->tag.len)))
		return;
	remote	= repo_remote_name(&dep->repo);
	section	= xstrfmt("remote.%s",	remote);
	url	= xstrfmt("%s.url",	section);
	fetch	= xstrfmt("%s.fetch",	section);
	tag_opt	= xstrfmt("%s.tagOpt",	section);
	tags	= xstrfmt("+refs/tags/*:refs/tags/%s/*", remote);
	heads	= xstrfmt("+refs/heads/*:refs/heads/%s/*", remote);
	git	= xstrfmt("%s/kleaver/v1/git", XDG_CACHE_HOME);
	git_dir	= xstrfmt("%s/.git", git);
	config	= xstrfmt("%s/config", git_dir);
	git_init(git);
	git_config_remove_section(config, section);
	git_config_add(config, url,	dep->repo.buf);
	git_config_add(config, fetch,	tags);
	git_config_add(config, fetch,	heads);
	git_config_add(config, tag_opt,	"--no-tags");
	git_fetch(git_dir, remote);
	free(remote);
	free(section);
	free(url);
	free(fetch);
	free(tag_opt);
	free(tags);
	free(heads);
	free(git);
	free(git_dir);
	free(config);
}

void dep_checkout(struct dep *dep)
{
	struct strbuf hash = STRBUF_INIT;
	char *remote, *git_dir;
	struct extcmd cmd;

	if (!(dep->repo.len && (dep->branch.len || dep->tag.len)))
		return;
	remote	= repo_remote_name(&dep->repo);
	git_dir	= xstrfmt("%s/kleaver/v1/git/.git", XDG_CACHE_HOME);
	extcmd_init(&cmd, "git");
	extcmd_arg(&cmd, "--git-dir=%s", git_dir);
	extcmd_arg(&cmd, "show-ref");
	extcmd_arg(&cmd, "--hash");
	if (dep->branch.len) {
		extcmd_arg(&cmd, "--heads");
		extcmd_arg(&cmd, "%s/%s", remote, dep->branch.buf);
	} else if (dep->tag.len) {
		extcmd_arg(&cmd, "--tags");
		extcmd_arg(&cmd, "%s/%s", remote, dep->tag.buf);
	}
	cmd.output = &hash;
	extcmd_run(&cmd);
	extcmd_release(&cmd);
	strbuf_trim_trailing_newline(&hash);
	strbuf_reset(&dep->commit);
	strbuf_addbuf(&dep->commit, &hash);
	strbuf_reset(&dep->worktree);
	strbuf_addstr(&dep->worktree, XDG_CACHE_HOME);
	strbuf_addstr(&dep->worktree, "/kleaver/v1/worktrees/");
	strbuf_addbuf(&dep->worktree, &hash);
	extcmd_init(&cmd, "git");
	extcmd_arg(&cmd, "--git-dir=%s", git_dir);
	extcmd_arg(&cmd, "worktree");
	extcmd_arg(&cmd, "add");
	extcmd_arg(&cmd, "-f");
	extcmd_arg(&cmd, "%s", dep->worktree.buf);
	extcmd_arg(&cmd, "%s", hash.buf);
	extcmd_run(&cmd);
	extcmd_release(&cmd);
	strbuf_reset(&dep->build_dir);
	strbuf_addstr(&dep->build_dir, XDG_CACHE_HOME);
	strbuf_addstr(&dep->build_dir, "/kleaver/v1/builds/");
	strbuf_addbuf(&dep->build_dir, &hash);
	strbuf_release(&hash);
	free(remote);
	free(git_dir);
}

void dep_resolve_ref(struct dep *dep)
{
	struct strbuf output = STRBUF_INIT;
	struct extcmd cmd;
	char* first_tab;

	extcmd_init(&cmd, "git ls-remote");
	extcmd_arg(&cmd, "%s", dep->repo.buf);
	if (dep->branch.len) {
		extcmd_arg(&cmd, "%s", "--heads");
		extcmd_arg(&cmd, "%s", dep->branch.buf);
	}
	else {
		extcmd_arg(&cmd, "%s", "--tags");
		extcmd_arg(&cmd, "%s", dep->tag.buf);
	}
	cmd.output = &output;
	extcmd_run(&cmd);
	extcmd_release(&cmd);
	/* Trim everything after first tab
	 * git ls-remote output
	 * commit_hash<tab>ref_name */
	first_tab = strchr(output.buf, '\t');
	if (!first_tab) {
		/* TODO report error */
		return;
	}
	strbuf_setlen(&output, first_tab - output.buf);
	dep->commit = output;
}
