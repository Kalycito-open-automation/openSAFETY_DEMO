/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>

#undef errno
extern int errno;

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

register char * stack_ptr __asm("sp");


caddr_t _sbrk(int incr)
{
    extern char end __asm("end");
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0)
        heap_end = &end;

    prev_heap_end = heap_end;

    if ((heap_end + incr) > stack_ptr)
    {
#if 0
        write(1, "Heap and stack collision\n", 25);
        abort();
#endif
        errno = ENOMEM;
        return (caddr_t) -1;
    }

    heap_end += incr;

    return (caddr_t) prev_heap_end;
}

int _gettimeofday (struct timeval * tp, struct timezone * tzp)
{
    (void)tp;

    /* Return fixed data for the timezone.  */

    if (tzp)
    {
        tzp->tz_minuteswest = 0;
        tzp->tz_dsttime = 0;
    }

    return 0;
}
void initialise_monitor_handles()
{
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;

    errno = EINVAL;

    return -1;
}

void _exit (int status)
{
    _kill(status, -1);
    while (1) {}
}

int _write(int file, char *ptr, int len)
{
    int DataIdx;

    (void)file;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar( *ptr++ );
    }

    return len;
}

int _close(int file)
{
    (void)file;

    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;

    st->st_mode = S_IFCHR;

    return 0;
}

int _isatty(int file)
{
    (void)file;

    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;

    return 0;
}

int _read(int file, char *ptr, int len)
{
    int DataIdx;

    (void)file;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = __io_getchar();
    }

   return len;
}

int _open(char *path, int flags, ...)
{
    (void)path;
    (void)flags;

    /* Pretend like we always fail */
    return -1;
}

int _wait(int *status)
{
    (void)status;

    errno = ECHILD;

    return -1;
}

int _unlink(char *name)
{
    (void)name;

    errno = ENOENT;

    return -1;
}

int _times(struct tms *buf)
{
    (void)buf;

    return -1;
}

int _stat(char *file, struct stat *st)
{
    (void)file;

    st->st_mode = S_IFCHR;

    return 0;
}

int _link(char *old, char *new)
{
    (void)old;
    (void)new;

    errno = EMLINK;

    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    (void)name;
    (void)argv;
    (void)env;

    errno = ENOMEM;
    return -1;
}
