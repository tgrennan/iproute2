/*
 * iplink_torus.c	torus driver
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Tom Grennan <tmgrennan@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include "utils.h"
#include "ip_common.h"
#include <linux/torus.h>

#define	USAGE								\
	"Usage:	... torus node TORUS\n"					\
	"	... torus clone DEVICE [CLONE]\n"			\
	"	... torus toroid TOROID [ROWSxCOLS[xCLONES]]\n"		\
	"\n"								\
	"TORUS	:= TOROID.NODE[xCLONES]\n"				\
	"TOROID	:= %d..%d\n"						\
	"NODE	:= %d..%d\n"						\
	"CLONES	:= %d..%d		# Per node (default, %d)\n"	\
	"CLONE	:= %d..%d		# default, first unused entry\n"\
	"ROWS 	:= %d..%d 		# default, %d\n"		\
	"COLS 	:= %d..%d 		# default, %d\n"		\
	, TORUS_MIN_TOROID_ID, TORUS_MAX_TOROID_ID			\
	, TORUS_MIN_NODE_ID, TORUS_MAX_NODE_ID				\
	, TORUS_MIN_CLONES, TORUS_MAX_CLONES, TORUS_DEFAULT_CLONES	\
	, TORUS_MIN_CLONE_ID, TORUS_MAX_CLONE_ID			\
	, TORUS_MIN_ROWS, TORUS_MAX_ROWS, TORUS_DEFAULT_ROWS		\
	, TORUS_MIN_COLS, TORUS_MAX_COLS, TORUS_DEFAULT_COLS

static const char *help = "help";
#define	this_may_help(argp,...)						\
	do {								\
		if (!strcmp((argp), help)) {				\
			fprintf(stderr, __VA_ARGS__);			\
			return -1;					\
		}							\
	} while (0)

#define	MAXLEN	1024

static const char *out_of_range_msg = "out of range";
#define	add_this_attr(hdr,attr,val,argp)				\
	do {								\
		if (!((val) >= TORUS_MIN_ ## attr			\
		      && (val) <= TORUS_MAX_ ## attr))			\
			invarg(out_of_range_msg, (argp));		\
		addattr32((hdr), MAXLEN, TORUS_ ## attr ## _ATTR,	\
			  (val));					\
	} while (0)

static const char *too_long_msg = "too long";
#define	add_this_ifname(hdr,full_attr,argp)				\
	do {								\
		size_t	len = strlen(argp) + 1;				\
		if (len > IFNAMSIZ)					\
			invarg(too_long_msg, (argp));			\
		addattr_l(hdr, MAXLEN, (full_attr), (argp), len);	\
	} while (0)

static inline int parse_this_node(struct link_util *lu, int argc, char **argv,
				  struct nlmsghdr *hdr)
{
	int	i;
	__u32	toroid_id = TORUS_MIN_TOROID_ID,
		node_id = TORUS_MIN_NODE_ID,
		clones = TORUS_MIN_CLONES;
	NEXT_ARG();
	this_may_help(*argv, "Expect TORUS.\n");
	i = sscanf(*argv, "%u.%ux%u", &toroid_id, &node_id, &clones);
	if (i < 2)
		invarg("ill formated TORUS", *argv);
	add_this_attr(hdr, TOROID_ID, toroid_id, *argv);
	add_this_attr(hdr, NODE_ID, node_id, *argv);
	add_this_attr(hdr, CLONES, clones, *argv);
	if (argv++, --argc)
		invarg("unexpected", *argv);
	return 0;
}

static inline int parse_this_clone(struct link_util *lu, int argc, char **argv,
				   struct nlmsghdr *hdr)
{
	__u32	clone_id;

	NEXT_ARG();
	this_may_help(*argv, "Must define node DEVICE.\n");
	add_this_ifname(hdr, TORUS_NODE_ATTR, *argv);
	if (argv++, --argc) {
		this_may_help(*argv, "Optional CLONE ID.\n");
		if (get_u32(&clone_id, *argv, 0))
			invarg("Non-numeric ID", *argv);
		add_this_attr(hdr, CLONE_ID, clone_id, *argv);
		if (argv++, --argc)
			invarg("unexpected", *argv);
	}
	return 0;
}

static inline int parse_this_toroid(struct link_util *lu, int argc,
				     char **argv, struct nlmsghdr *hdr)
{
	int	i;
	__u32	toroid_id = TORUS_MIN_TOROID_ID,
		rows = TORUS_DEFAULT_ROWS,
		cols = TORUS_DEFAULT_COLS,
		clones = TORUS_DEFAULT_CLONES;
	NEXT_ARG();
	this_may_help(*argv, "Expect TOROID.\n");
	if (get_u32(&toroid_id, *argv, 0))
		invarg("Non-numeric ID", *argv);
	add_this_attr(hdr, TOROID_ID, toroid_id, *argv);
	if (argv++, --argc) {
		this_may_help(*argv, "Optional ROWSxCOLS[xCLONES].\n");
		i = sscanf(*argv, "%ux%ux%u", &rows, &cols, &clones);
		if (i < 2)
			invarg("ill formated ROWSxCOLS", *argv);
		if (argv++, --argc)
			invarg("unexpected", *argv);
	}
	add_this_attr(hdr, ROWS, rows, *argv);
	add_this_attr(hdr, COLS, cols, *argv);
	add_this_attr(hdr, CLONES, clones, *argv);
	return 0;
}

static int parse_this_opt(struct link_util *lu, int argc, char **argv,
			  struct nlmsghdr *hdr)
{
	this_may_help(*argv, USAGE);
	addattr32(hdr, MAXLEN, TORUS_VERSION_ATTR, TORUS_VERSION);
#define	__may_do(this)							\
	if (!strcmp(*argv, #this))					\
		return parse_this_ ## this (lu, argc, argv, hdr)
	__may_do(node);
	__may_do(clone);
	__may_do(toroid);
	invarg("try \"help\"", *argv);
}

struct link_util torus_link_util = {
	.id = "torus",
	.parse_opt = parse_this_opt,
};
