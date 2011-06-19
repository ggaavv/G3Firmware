/*
 * newlibMinimal.h
 *
 *  Created on: Dec 15, 2010
 *      Author: dejagerd
 */

#ifndef NEWLIBMINIMAL_H_
#define NEWLIBMINIMAL_H_
#include <errno.h>
#include <string.h>
typedef struct {
	const char *name;
	int (*open_r)(struct _reent *r, const char *path, int flags, int mode);
	int (*close_r)(struct _reent *r, int fd);
	long (*write_r)(struct _reent *r, int fd, const char *ptr, int len);
	long (*read_r)(struct _reent *r, int fd, char *ptr, int len);
} devoptab_t;
__attribute__((warning("call to default open_r")))
int _undef_open_r(struct _reent *r, const char *path, int flags, int mode) {
	return -1;
}
__attribute__((warning("call to default close_r")))
int _undef_close_r(struct _reent *r, int fd) {
	return -1;
}
__attribute__((warning("call to default write_r")))
long _undef_write_r(struct _reent *r, int fd, const char *ptr, int len) {
	return len;
}
__attribute__((warning("call to default read_r")))
long _undef_read_r(struct _reent *r, int fd, char *ptr, int len) {
	return len;
}

__attribute__(())    const devoptab_t devoptab_tty0 = { "tty0", _undef_open_r,
		_undef_close_r, _undef_write_r, _undef_read_r };

const devoptab_t *devoptab_list[] = { &devoptab_tty0, /* standard input */
&devoptab_tty0, /* standard output */
&devoptab_tty0, /* standard error */
/*add more here as necessary*/
0 /* terminates the list */
};

long _write_r(struct _reent *ptr, int fd, const char *buf, size_t cnt) {
	return devoptab_list[fd]->write_r(ptr, fd, buf, cnt);
}

long _read_r(struct _reent *ptr, int fd, char *buf, size_t cnt) {
	return devoptab_list[fd]->read_r(ptr, fd, buf, cnt);
}

int _open_r(struct _reent *ptr, const char *file, int flags, int mode) {
	int which_devoptab = 0;
	int fd = -1;
	/* search for "file" in dotab_list[]->name */
	do {
		if (strcmp(devoptab_list[which_devoptab]->name, file) == 0) {
			fd = which_devoptab;
			break;
		}
	} while (devoptab_list[which_devoptab++] != 0);
	/* if we found the requested file/device, invoke the device's open_r() */
	if (fd != -1) {
		devoptab_list[fd]->open_r(ptr, file, flags, mode);
	} /* it doesn't exist! */
	else {
		ptr->_errno = ENODEV;
	}
	return fd;
}
long _close_r(struct _reent *ptr, int fd) {
	return devoptab_list[fd]->close_r(ptr, fd);
}
unsigned char _heap[0x1000];
char* _sbrk_r(int incr) {
	register char *SP asm ("sp");

	extern char _end; /* Defined by the linker */
	static char *heap_end;
	char *prev_heap_end;
	if (heap_end == 0) {
		heap_end = &_end;
	}
	prev_heap_end = heap_end;
	if (heap_end + incr > SP) {
		return 0;
	}
	heap_end += incr;
	return prev_heap_end;

}

#endif /* NEWLIBMINIMAL_H_ */
