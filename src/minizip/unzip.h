/* unzip.h -- IO for uncompress .zip files using zlib
   Version 1.01, May 8th, 2004

   Copyright (C) 1998-2004 Gilles Vollant

	This unzip package allow extract file from .ZIP file, compatible with PKZip
	2.04g WinZip, InfoZip tools and compatible.  Encryption and multi volume
	ZipFile (span) are not supported.  Old compressions used by old PKZip 1.x
	are not supported


   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

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

#ifndef UNZIP_H
#define UNZIP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZLIB_H
#include "zlib.h"
#endif

#ifndef _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of _WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagunzFile__ {int32_t unused;} unzFile__;
typedef unzFile__ * unzFile;
#else
typedef voidp unzFile;
#endif


#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

/* tm_unz contain date/time info */
typedef struct tm_unz_s
{
	uInt tm_sec;            /* seconds after the minute - [0, 59] */
	uInt tm_min;            /* minutes after the hour - [0, 59] */
	uInt tm_hour;           /* hours since midnight - [0, 23] */
	uInt tm_mday;           /* day of the month - [1, 31] */
	uInt tm_mon;            /* months since January - [0, 11] */
	uInt tm_year;           /* years - [1980..2044] */
} tm_unz;

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
typedef struct unz_global_info_s
{
	//  total number of entries in the central dir on this disk
	uLong number_entry;
	uLong size_comment;         /* size of the global comment of the zipfile */
} unz_global_info;


/* unz_file_info contain information about a file in the zipfile */
typedef struct unz_file_info_s
{
	uLong version;              /* version made by                 2 bytes */
	uLong version_needed;       /* version needed to extract       2 bytes */
	uLong flag;                 /* general purpose bit flag        2 bytes */
	uLong compression_method;   /* compression method              2 bytes */
	uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
	uLong crc;                  /* crc-32                          4 bytes */
	uLong compressed_size;      /* compressed size                 4 bytes */
	uLong uncompressed_size;    /* uncompressed size               4 bytes */
	uLong size_filename;        /* filename length                 2 bytes */
	uLong size_file_extra;      /* extra field length              2 bytes */
	uLong size_file_comment;    /* file comment length             2 bytes */

	uLong disk_num_start;       /* disk number start               2 bytes */
	uLong internal_fa;          /* internal file attributes        2 bytes */
	uLong external_fa;          /* external file attributes        4 bytes */

	tm_unz tmu_date;
} unz_file_info;


extern unzFile ZEXPORT unzOpen OF((char const * path));
/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows XP computer "c:\\zlib\\zlib113.zip" or on an Unix computer
     "zlib/zlib113.zip".
     If the zipfile cannot be opened (file don't exist or in not valid), the
       return value is nullptr.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
*/

extern unzFile ZEXPORT unzOpen2 OF
	((char const * path, zlib_filefunc_def * pzlib_filefunc_def));
/*
   Open a Zip file, like unzOpen, but provide a set of file low level API
      for read/write the zip file (see ioapi.h)
*/

extern int32_t ZEXPORT unzClose OF((unzFile file));
/*
  Close a ZipFile opened with unzipOpen.  If there is files inside the .Zip
  opened with unzOpenCurrentFile (see later), these files MUST be closed with
  unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */


/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

extern int32_t ZEXPORT unzGoToFirstFile OF((unzFile file));
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/

extern int32_t ZEXPORT unzGoToNextFile OF((unzFile file));
/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

/* ****************************************** */

extern int32_t ZEXPORT unzGetCurrentFileInfo OF
	((unzFile         file,
	  unz_file_info * pfile_info,
	  char          * szFileName,
	  uLong           fileNameBufferSize,
	  void          * extraField,
	  uLong           extraFieldBufferSize,
	  char          * szComment,
	  uLong           commentBufferSize));
	//  Get Info about the current file
	//  * If pfile_info, the *pfile_info structure will contain some info about
	//    the current file.
	//  * If szFileName, the filemane string will be copied in szFileName
	//    (fileNameBufferSize is the size of the buffer).
	//  * If extraField, the extra field information will be copied in
	//    extraField (extraFieldBufferSize is the size of the buffer). This is
	//    the Central-header version of the extra field.
	//  * If szComment, the comment string of the file will be copied in
	//    szComment (commentBufferSize is the size of the buffer).

/***************************************************************************/
/* for reading the content of the current zipfile, you can open it, read data
   from it, and close it (you can close it before reading all the file)
   */

extern int32_t ZEXPORT unzOpenCurrentFile OF((unzFile file));
/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
*/

extern int32_t ZEXPORT unzOpenCurrentFile3 OF
	((unzFile      file,
	  int32_t    * method,
	  int32_t    * level,
	  int32_t      raw,
	  char const * password));
/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
  *method will receive method of compression, *level will receive level of
     compression
  note : you can set level parameter as nullptr (if you did not want known level,
         but you CANNOT set method parameter as nullptr
*/


extern int32_t ZEXPORT unzCloseCurrentFile OF((unzFile file));
/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/

extern int32_t ZEXPORT unzReadCurrentFile OF
	((unzFile file, voidp buf, unsigned len));
/*
  Read bytes from the current file (opened by unzOpenCurrentFile)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/

/***************************************************************************/



#ifdef __cplusplus
}
#endif

#endif
