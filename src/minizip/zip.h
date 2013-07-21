/* zip.h -- IO for compress .zip files using zlib
   Version 1.01, May 8th, 2004

   Copyright (C) 1998-2004 Gilles Vollant

   This unzip package allow creates .ZIP file, compatible with PKZip 2.04g
     WinZip, InfoZip tools and compatible.
   Encryption and multi volume ZipFile (span) are not supported.
   Old compressions used by old PKZip 1.x are not supported

  For uncompress .zip file, look at unzip.h


   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.html for evolution

   Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.


*/

/* for more info about .ZIP format, see
      http://www.info-zip.org/pub/infozip/doc/appnote-981119-iz.zip
      http://www.info-zip.org/pub/infozip/doc/
   PkWare has also a specification at :
      ftp://ftp.pkware.com/probdesc.zip
*/

#ifndef ZIP_H
#define ZIP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZLIB_H
#include "zlib.h"
#endif

#ifndef _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#if defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of _WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagzipFile__ {int32_t unused;} zipFile__;
typedef zipFile__ * zipFile;
#else
typedef voidp zipFile;
#endif

#define ZIP_OK                          (0)
#define ZIP_EOF                         (0)
#define ZIP_ERRNO                       (Z_ERRNO)
#define ZIP_PARAMERROR                  (-102)
#define ZIP_BADZIPFILE                  (-103)
#define ZIP_INTERNALERROR               (-104)

#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif
/* default memLevel */

/* tm_zip contain date/time info */
typedef struct tm_zip_s
{
	uInt tm_sec;            /* seconds after the minute - [0, 59] */
	uInt tm_min;            /* minutes after the hour - [0, 59] */
	uInt tm_hour;           /* hours since midnight - [0, 23] */
	uInt tm_mday;           /* day of the month - [1, 31] */
	uInt tm_mon;            /* months since January - [0, 11] */
	uInt tm_year;           /* years - [1980..2044] */
} tm_zip;

typedef struct
{
	tm_zip      tmz_date;       /* date in understandable format           */
	uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
#if 0
	uLong       flag;           /* general purpose bit flag        2 bytes */
#endif

	uLong       internal_fa;    /* internal file attributes        2 bytes */
	uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

typedef char const * zipcharpc;


#define APPEND_STATUS_CREATE        (0)
#define APPEND_STATUS_CREATEAFTER   (1)
#define APPEND_STATUS_ADDINZIP      (2)

extern zipFile ZEXPORT zipOpen OF((char const * pathname, int32_t append));
/*
  Create a zipfile.  pathname contain on Windows XP a filename like
  "c:\\zlib\\zlib113.zip" or on an Unix computer "zlib/zlib113.zip".  if the
  file pathname exist and append==APPEND_STATUS_CREATEAFTER, the zip will be
  created at the end of the file.  (useful if the file contain a self extractor
  code) if the file pathname exist and append==APPEND_STATUS_ADDINZIP, we will
  add files in existing zip (be sure you don't add file that doesn't exist) If
  the zipfile cannot be opened, the return value is nullptr.  Else, the return
  value is a zipFile Handle, usable with other function of this zip package.
*/

/* Note : there is no delete function into a zipfile.
	If you want delete file into a zipfile, you must open a zipfile, and create
	another Of couse, you can use RAW reading and writing to copy the file you
	did not want delte
*/

extern zipFile ZEXPORT zipOpen2 OF
	((char const        * pathname,
	  int32_t             append,
	  zipcharpc         * globalcomment,
	  zlib_filefunc_def * pzlib_filefunc_def));

extern int32_t ZEXPORT zipOpenNewFileInZip OF
	((zipFile              file,
	  char         const * filename,
	  zip_fileinfo const * zipfi,
	  void         const * extrafield_local,
	  uInt                 size_extrafield_local,
	  void         const * extrafield_global,
	  uInt                 size_extrafield_global,
	  char         const * comment,
	  int32_t              method,
	  int32_t              level));
/*
  Open a file in the ZIP for writing.
  filename : the filename in zip (if nullptr, '-' without quote will be used
  *zipfi contain supplemental information
  if extrafield_local!=nullptr and size_extrafield_local>0, extrafield_local
    contains the extrafield data the the local header
  if extrafield_global!=nullptr and size_extrafield_global>0, extrafield_global
    contains the extrafield data the the local header
  if comment != nullptr, comment contain the comment string
  method contain the compression method (0 for store, Z_DEFLATED for deflate)
  level contain the level of compression (can be Z_DEFAULT_COMPRESSION)
*/


/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

extern int32_t ZEXPORT zipOpenNewFileInZip3 OF
	((zipFile              file,
	  char         const * filename,
	  zip_fileinfo const * zipfi,
	  void         const * extrafield_local,
	  uInt                 size_extrafield_local,
	  void         const * extrafield_global,
	  uInt                 size_extrafield_global,
	  char         const * comment,
	  int32_t              method,
	  int32_t              level,
	  int32_t              raw,
	  int32_t              windowBits,
	  int32_t              memLevel,
	  int32_t              strategy,
	  char         const * password,
	  uLong                crcForCtypting));

/*
  Same than zipOpenNewFileInZip2, except
    windowBits, memLevel, strategy : see parameter strategy in deflateInit2
    password : crypting password (nullptr for no crypting)
    crcForCtypting : crc of file to compress (needed for crypting)
 */


extern int32_t ZEXPORT zipWriteInFileInZip OF
	((zipFile file, void const * buf, unsigned len));
/*
  Write data in the zipfile
*/

extern int32_t ZEXPORT zipCloseFileInZip OF((zipFile file));
/*
  Close the current file in the zipfile
*/

extern int32_t ZEXPORT zipCloseFileInZipRaw OF
	((zipFile file, uLong uncompressed_size, uLong crc32));
/*
  Close the current file in the zipfile, for fiel opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
*/

extern int32_t ZEXPORT zipClose OF((zipFile file, char const * global_comment));
/*
  Close the zipfile
*/

#ifdef __cplusplus
}
#endif

#endif
