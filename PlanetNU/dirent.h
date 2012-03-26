/*****************************************************************************
 * dirent.h - dirent API for Microsoft Visual Studio
 *
 * Copyright (C) 2006 Toni Ronkko
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL TONI RONKKO BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Mar 15, 2011, Toni Ronkko
 * Defined FILE_ATTRIBUTE_DEVICE for MSVC 6.0.
 *
 * Aug 11, 2010, Toni Ronkko
 * Added d_type and d_namlen fields to dirent structure.  The former is
 * especially useful for determining whether directory entry represents a
 * file or a directory.  For more information, see
 * http://www.delorie.com/gnu/docs/glibc/libc_270.html
 *
 * Aug 11, 2010, Toni Ronkko
 * Improved conformance to the standards.  For example, errno is now set
 * properly on failure and assert() is never used.  Thanks to Peter Brockam
 * for suggestions.
 *
 * Aug 11, 2010, Toni Ronkko
 * Fixed a bug in rewinddir(): when using relative directory names, change
 * of working directory no longer causes rewinddir() to fail.
 *
 * Dec 15, 2009, John Cunningham
 * Added rewinddir member function
 *
 * Jan 18, 2008, Toni Ronkko
 * Using FindFirstFileA and WIN32_FIND_DATAA to avoid converting string
 * between multi-byte and unicode representations.  This makes the
 * code simpler and also allows the code to be compiled under MingW.  Thanks
 * to Azriel Fasten for the suggestion.
 *
 * Mar 4, 2007, Toni Ronkko
 * Bug fix: due to the strncpy_s() function this file only compiled in
 * Visual Studio 2005.  Using the new string functions only when the
 * compiler version allows.
 *
 * Nov  2, 2006, Toni Ronkko
 * Major update: removed support for Watcom C, MS-DOS and Turbo C to
 * simplify the file, updated the code to compile cleanly on Visual
 * Studio 2005 with both unicode and multi-byte character strings,
 * removed rewinddir() as it had a bug.
 *
 * Aug 20, 2006, Toni Ronkko
 * Removed all remarks about MSVC 1.0, which is antiqued now.  Simplified
 * comments by removing SGML tags.
 *
 * May 14 2002, Toni Ronkko
 * Embedded the function definitions directly to the header so that no
 * source modules need to be included in the Visual Studio project.  Removed
 * all the dependencies to other projects so that this very header can be
 * used independently.
 *
 * May 28 1998, Toni Ronkko
 * First version.
 *****************************************************************************/
#ifndef DIRENT_H
#define DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct dirent
{
   char d_name[MAX_PATH + 1];                  /* File name */
   size_t d_namlen;                            /* Length of name without \0 */
   int d_type;                                 /* File type */
} dirent;




typedef struct DIR DIR;
/* Forward declarations */
DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void rewinddir(DIR* dirp);

#ifdef __cplusplus
}
#endif
#endif /*DIRENT_H*/