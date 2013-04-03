#ifndef __LINUX_TORUS_H__
#define __LINUX_TORUS_H__

#define	TORUS		"torus"
#define	TORUS_PREFIX	"te"

#define	TORUS_VERSION		2
#define	TORUS_MIN_VERSION	2
#define	TORUS_MAX_VERSION	TORUS_VERSION
#define	TORUS_MIN_ROWS		2
#define	TORUS_MAX_ROWS		8
#define	TORUS_MIN_COLS		2
#define	TORUS_MAX_COLS		8
#define	TORUS_MIN_MTU		64
#define	TORUS_MAX_MTU		9000

enum {
	__TORUS_FIRST_ATTR,
	TORUS_VERSION_ATTR,
	TORUS_ROWS_ATTR,
	TORUS_COLS_ATTR,
	TORUS_MASTER_ATTR,
	__TORUS_LAST_ATTR
#define	TORUS_LAST_ATTR		(__TORUS_LAST_ATTR - 1)
#define TORUS_POLICIES		__TORUS_LAST_ATTR
};

#endif /* __LINUX_TORUS_H__ */