/* fsusage.c -- return space usage of mounted file systems

   Copyright (C) 1991, 1992, 1996, 1998, 1999, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
     /* POSIX 1003.1-2001 (and later) with XSI */
#include "fsusage.h"

#include <limits.h>
#include <sys/types.h>
#include <sys/statvfs.h>

/* The results of open() in this file are not used with fchdir,
   therefore save some unnecessary work in fchdir.c.  */
#undef open
#undef close

/* Many space usage primitives use all 1 bits to denote a value that is
   not applicable or unknown.  Propagate this information by returning
   a uintmax_t value that is all 1 bits if X is all 1 bits, even if X
   is unsigned and narrower than uintmax_t.  */
#define PROPAGATE_ALL_ONES(x) \
  ((sizeof (x) < sizeof (uintmax_t) \
    && (~ (x) == (sizeof (x) < sizeof (int) \
          ? - (1 << (sizeof (x) * CHAR_BIT)) \
          : 0))) \
   ? UINTMAX_MAX : (uintmax_t) (x))

/* Extract the top bit of X as an uintmax_t value.  */
#define EXTRACT_TOP_BIT(x) ((x) \
                & ((uintmax_t) 1 << (sizeof (x) * CHAR_BIT - 1)))

/* If a value is negative, many space usage primitives store it into an
   integer variable by assignment, even if the variable's type is unsigned.
   So, if a space usage variable X's top bit is set, convert X to the
   uintmax_t value V such that (- (uintmax_t) V) is the negative of
   the original value.  If X's top bit is clear, just yield X.
   Use PROPAGATE_TOP_BIT if the original value might be negative;
   otherwise, use PROPAGATE_ALL_ONES.  */
#define PROPAGATE_TOP_BIT(x) ((x) | ~ (EXTRACT_TOP_BIT (x) - 1))

/* Fill in the fields of FSP with information about space usage for
   the file system on which FILE resides.
   DISK is the device on which FILE is mounted, for space-getting
   methods that need to know it.
   Return 0 if successful, -1 if not.  When returning -1, ensure that
   ERRNO is either a system error value, or zero if DISK is NULL
   on a system that requires a non-NULL value.  */
int get_fs_usage (char const *file, char const *disk, struct fs_usage *fsp) {

  struct statvfs fsd;

  if (statvfs (file, &fsd) < 0)
    return -1;

  /* f_frsize isn't guaranteed to be supported.  */
  fsp->fsu_blocksize = (fsd.f_frsize
            ? PROPAGATE_ALL_ONES (fsd.f_frsize)
            : PROPAGATE_ALL_ONES (fsd.f_bsize));

  fsp->fsu_blocks = PROPAGATE_ALL_ONES (fsd.f_blocks);
  fsp->fsu_bfree = PROPAGATE_ALL_ONES (fsd.f_bfree);
  fsp->fsu_bavail = PROPAGATE_TOP_BIT (fsd.f_bavail);
  fsp->fsu_bavail_top_bit_set = EXTRACT_TOP_BIT (fsd.f_bavail) != 0;
  fsp->fsu_files = PROPAGATE_ALL_ONES (fsd.f_files);
  fsp->fsu_ffree = PROPAGATE_ALL_ONES (fsd.f_ffree);
  return 0;
}

