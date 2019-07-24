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
#include <errno.h>
#include <getopt.h>
#include <kleaver/flag.h>
#include <kleaver/logger.h>
#include <stdlib.h>
#include <string.h>

static LIST_HEAD(all_flags);

void flag_add(struct flag *flag)
{
	INIT_LIST_HEAD(&flag->all_flags);
	list_add_tail(&flag->all_flags, &all_flags);
}

int flag_parse_bool(const char *in, void *out)
{
	if (!in || !strcmp(in, "true")) {
		*(bool *)out = true;
		return 0;
	}
	if (!strcmp(in, "false")) {
		*(bool *)out = false;
		return 0;
	}
	return -1;
}

int flag_parse_string(const char *in, void *out)
{
	*(const char **)out = in;
	return 0;
}

int flag_init(int argc, char **argv)
{
	struct option *longopts, *opt;
	int num_flags = 0, i, c;
	struct flag *flag;

	list_for_each_entry(flag, &all_flags, all_flags)
		num_flags++;
	if (!num_flags)
		return 0;
	/* allocate an extra element for the end of the array (all zeros) */
	longopts = calloc(num_flags + 1, sizeof(longopts[0]));
	if (!longopts)
		PLOG_FATAL("calloc %d longopts", num_flags + 1);
	opt = longopts;
	list_for_each_entry(flag, &all_flags, all_flags) {
		opt->name = flag->name;
		opt->has_arg = flag->has_arg;
		opt->flag = &flag->found;
		opt->val = 1;
		opt++;
	}
	/* always start with the 1st element of argv */
	optind = 1;
	/* suppress getopt() error message output */
	opterr = 0;
	while ((c = getopt_long_only(argc, argv, ":", longopts, &i)) != -1) {
		if (c == '?')
			/* TODO unrecognized flag */;
		if (c == ':')
			/* TODO flag missing argument */;
		CHECK(c == 0, "getopt_long_only returns %d != 0", c);
		flag = container_of(longopts[i].flag, struct flag, found);
		LOG_INFO("flag %s", flag->name);
		/* TODO check return value */
		flag->parse(optarg, flag->var);
	}
	free(longopts);
	return optind;
}
