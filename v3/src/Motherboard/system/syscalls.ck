#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "sys/syscall.h"
int errno;

int __read (char *ptr, int len);
int __write(char *ptr, int len);

/* This is used by _sbrk.  */
register char *stack_ptr asm ("r15");

int
_read (int file,
       char *ptr,
       int len)
{
  return __read( ptr, len);
}

int
_lseek (int file,
	int ptr,
	int dir)
{
  return -1;
}

int
_write ( int file,
	 char *ptr,
	 int len)
{
  return __write( ptr, len);
}

int
_close (int file)
{
  return -1;
}

int
_link (char *old, char *new)
{
  return -1;
}

caddr_t
_sbrk (int incr)
{
  extern char end;		/* Defined by the linker */
  static char *heap_end = &end;
  char *prev_heap_end;

  prev_heap_end = heap_end;
#if 0
  if ( heap_end + incr > sbrk_pool + SBRK_POOL_SIZE )
    {
      _write (1, "sbrk_pool full.\n", 16);
      abort ();
    }
#endif
  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

int
_fstat (int file,
	struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

int
_open (const char *path,
	int flags)
{
  return -1;
}

int
_creat (const char *path,
	int mode)
{
  return -1;
}

int
_unlink ()
{
  return -1;
}

isatty (fd)
     int fd;
{
  return 1;
}

_exit (n)
{
  return -1;
}

_kill (n, m)
{
  return -1;
}

_getpid (n)
{
  return 1;
}

_raise ()
{
}

int
_stat (const char *path, struct stat *st)

{
  return -1;
}

int
_chmod (const char *path, short mode)
{
  return -1;
}

int
_chown (const char *path, short owner, short group)
{
  return -1;
}

int
_utime (path, times)
     const char *path;
     char *times;
{
  return -1;
}

int
_fork ()
{
  return -1;
}

int
_wait (statusp)
     int *statusp;
{
  return -1;
}

int
_execve (const char *path, char *const argv[], char *const envp[])
{
  return -1;
}

int
_execv (const char *path, char *const argv[])
{
  return -1;
}

int
_pipe (int *fd)
{
  return -1;
}

/* This is only provided because _gettimeofday_r and _times_r are
   defined in the same module, so we avoid a link error.  */
clock_t
_times (struct tms *tp)
{
  return -1;
}

int
_gettimeofday (struct timeval *tv, struct timezone *tz)
{
  tv->tv_usec = 0;
  tv->tv_sec = 0;
  return 0;
}

int
__setup_argv_and_call_main ()
{
    return main ();
}
