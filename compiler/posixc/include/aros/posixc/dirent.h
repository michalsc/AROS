#ifndef _POSIXC_DIRENT_H_
#define _POSIXC_DIRENT_H_

/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: POSIX.1-2008 header file dirent.h
*/

#include <aros/features.h>
#include <aros/system.h>

#include <aros/types/ino_t.h>
#include <aros/types/off_t.h>

#if defined(__USE_NIXCOMMON)

/* d_type */
#define DT_UNKNOWN     0
#define DT_FIFO        1
#define DT_CHR         2
#define DT_DIR         4
#define DT_BLK         6
#define DT_REG         8
#define DT_LNK        10
#define DT_SOCK       12
#define DT_WHT        14

#endif /* !_POSIX_SOURCE */

/* NB we must use the __xxx_t types here, because the xxx_t version
 * may not be defined */
struct dirent
{
/* NOTIMPL #if defined(__USE_FILE_OFFSET64)
    __ino64_t d_ino;
    __off64_t d_off;
#else */
    __ino_t d_ino;
    __off_t d_off;
/* #endif */
    unsigned short int d_reclen;
    unsigned char d_type;
    /* it is not allowed to include limits.h, so we must
     * hard code the size here. names must be no longer
     * than this */
    char    d_name[256];	
};

#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_TYPE

/* defined for backwards compatibility..  */
#define d_fileno        d_ino

/* NOTIMP
#ifdef __USE_LARGEFILE64
struct dirent64
{
    __ino64_t d_ino;
    __off64_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];
};
#endif */

/* opaque structure describing directory stream objects */
struct __dirdesc;
typedef struct __dirdesc DIR;

__BEGIN_DECLS

#if defined(__USE_XOPEN2K8)
/* NOTIMPL DIR *fdopendir(int); */
#endif

DIR *opendir (const char *filename);
int closedir(DIR *dir);
struct dirent *readdir (DIR *dir);
#ifdef __USE_LARGEFILE64
/* NOTIMP struct dirent64 *readdir64 (DIR *dir); */
#endif
/* NOTIMPL int readdir_r (DIR *dir, struct dirent *entry, struct dirent **result); */
# ifdef __USE_LARGEFILE64
/* NOTIMPL 
int readdir64_r (DIR *dir,
                        struct dirent64 *entry,
                        struct dirent64 **result) */
# endif
#ifdef __USE_XOPEN2K8
int dirfd(DIR *dir);
int scandir (const char *dir, struct dirent ***namelist,
              int (*select)(const struct dirent *),
              int (*compar)(const struct dirent **,
                            const struct dirent **));
#if defined(__USE_GNU) && defined(__USE_LARGEFILE64)
/* NOTIMPL
int scandir64 (const char *dir,
                      struct dirent64 ***namelist,
                      int (*select) (const struct dirent64 *),
                      int (*compar) (const struct dirent64 **,
                                    const struct dirent64 **)) */
#endif
int alphasort(const struct dirent **a, const struct dirent **b);
#if defined(__USE_GNU) && defined(__USE_LARGEFILE64)
/* NOTIMPL
int alphasort64 (const struct dirent64 **a, const struct dirent64 **b) */
#endif
#endif

void rewinddir(DIR *dir);

#if defined(__USE_NIXCOMMON) || defined(__USE_XOPEN)
void seekdir(DIR *dir, off_t loc);
long telldir(DIR *dir);
#endif

__END_DECLS

#endif /* !_POSIXC_DIRENT_H_ */
