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
#include <linux/list.h>
#include <stdbool.h>

struct flag {
	const char	*name, *type, *help;
	int		has_arg;

	int	found;
	void	*var;
	int	(*parse)(const char *in, void *out);

	struct list_head	all_flags;
};

extern void flag_add(struct flag *);

#define DEFINE_FLAG(TYPE, NAME, DEFAULT, HELP, HAS_ARG, PARSER)		\
	TYPE FLAG_##NAME = DEFAULT;					\
	struct flag FLAG_##NAME##_desc = {				\
		.name = #NAME,						\
		.type = #TYPE,						\
		.help = HELP " (default " #DEFAULT ")",			\
		.has_arg = HAS_ARG,					\
		.var = &FLAG_##NAME,					\
		.parse = PARSER,					\
	};								\
	static void __attribute__((constructor)) FLAG_##NAME##_init(void) \
	{								\
		flag_add(&FLAG_##NAME##_desc);				\
	}

extern int flag_parse_bool(const char *in, void *out);

#define DEFINE_bool(NAME, DEFAULT, HELP)				\
	DEFINE_FLAG(bool, NAME, DEFAULT, HELP,				\
		    2 /* optional_argument */, flag_parse_bool)

/**
 * Parse command-line flags.
 *
 * See also getopt(3).
 *
 * @returns the number of elements processed
 */
extern int flag_init(int argc, char **argv);
