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
#include <linux/list.h>

struct dep {
	struct list_head	all_deps;

	struct strbuf	name;
	struct strbuf	repo, branch, tag;
	struct strbuf	worktree;
	struct strbuf	build_cmd;
	struct strbuf	build_dir;
};

extern struct list_head	all_deps;

extern void dep_init(struct dep *dep);
extern void dep_release(struct dep *dep);
extern void dep_fetch(struct dep *dep);
extern void dep_checkout(struct dep *dep);
