/* Don't want to modify the minizip/zlib sources, so lets silence the warnings */
#include "compile_diagnostics.h"
GCC_DIAG_OFF("-Wold-style-cast")
GCC_DIAG_OFF("-Wswitch-default")
GCC_DIAG_OFF("-Wunused-macros")
CLANG_DIAG_OFF("-Wunused-macros")

/*
================================================================================

 * Below this point, all zip functions
 *
 * These functions have been verbatim copied from minizip in the contrib
 * directory of the zlib distribution. Thanks goes to the authors who hold the
 * copyright on this code.
 * Same applies to the headers of course!
 *
================================================================================
*/

/* unzip.c -- IO for uncompress .zip files using zlib
   Version 1.01d, September 22th, 2004

   Copyright  (C) 1998-2004 Gilles Vollant

   Read unzip.h for more info
*/

/* Decryption code comes from crypt.c by Info-ZIP but has been greatly reduced
 * in terms of
compatibility with older software. The following is from the original crypt.c.
Code woven in by Terry Thorsen 1/2003.
*/
/*
  Copyright (c) 1990-2000 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later (the contents
  of which are also included in zip.h) for terms of use.  If, for some reason,
  all these files are missing, the Info-ZIP license also may be found at:
ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
  crypt.c (full version) by Info-ZIP.      Last revised:  [see crypt.h]

  The encryption/decryption parts of this source code (as opposed to the
  non-echoing password parts) were originally written in Europe.  The whole
  source package can be freely distributed, including from the USA.  (Prior to
  January 2000, re-export from the US was a violation of US law.)
 */

/*
  This encryption code is a direct transcription of the algorithm from Roger
  Schlafly, described by Phil Katz in the file appnote.txt.  This file
  (appnote.txt) is distributed with the PKZIP program (even in the version
  without encryption capabilities).
 */

#include "unzip.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>


#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


const char unz_copyright[] =
" unzip 1.01 Copyright 1998-2004 Gilles Vollant - "
"http://www.winimage.com/zLibDll";

/* unz_file_info_interntal contain internal info about a file in zipfile*/
typedef struct unz_file_info_internal_s
{
	uLong offset_curfile; //  relative offset of local header 4 bytes
} unz_file_info_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
typedef struct
{
	char * read_buffer;         /* internal buffer for compressed data */
	z_stream stream;            /* zLib stream structure for inflate */

	uLong pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
	uLong stream_initialized;   /* flag set if stream structure is initialized*/

	uLong offset_local_extrafield; //  offset of the local extra field
	uInt  size_local_extrafield; //  size of the local extra field
	uLong pos_local_extrafield;   /* position in the local extra field in read*/

	uLong crc32;                /* crc32 of all data uncompressed */
	uLong crc32_wait;           /* crc32 we must obtain after decompress all */
	uLong rest_read_compressed; /* number of byte to be decompressed */
	uLong rest_read_uncompressed; //  number of byte to be obtained after decomp
	zlib_filefunc_def z_filefunc;
	voidpf filestream;        /* io structore of the zipfile */
	uLong compression_method;   /* compression method (0==store) */
	uLong byte_before_the_zipfile; //  byte before the zipfile, (>0 for sfx)
	int32_t   raw;
} file_in_zip_read_info_s;


/* unz_s contain internal information about the zipfile
*/
typedef struct
{
	zlib_filefunc_def z_filefunc;
	voidpf filestream;        /* io structore of the zipfile */
	unz_global_info gi;       /* public global information */
	uLong byte_before_the_zipfile; //  byte before the zipfile, (>0 for sfx)
	uLong num_file;           /* number of the current file in the zipfile*/
	uLong pos_in_central_dir; /* pos of the current file in the central dir*/
	uLong current_file_ok;    /* flag about the usability of the current file*/
	uLong central_pos;        /* position of the beginning of the central dir*/

	uLong size_central_dir;     /* size of the central directory  */

	//  offset of start of central directory with respect to the starting disk
	//  number
	uLong offset_central_dir;

	unz_file_info cur_file_info; /* public info about the current file in zip*/
	unz_file_info_internal cur_file_info_internal; /* private info about it*/

	//  structure about the current file if we are decompressing it
	file_in_zip_read_info_s * pfile_in_zip_read;

	int32_t encrypted;
} unz_s;


/* ===========================================================================
     Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been successfully opened for reading.
*/


local int32_t unzlocal_getByte OF
	((zlib_filefunc_def const * pzlib_filefunc_def,
	  voidpf                    filestream,
	  int32_t                 * pi));

local int32_t unzlocal_getByte
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream,
	 int32_t                 * const pi)
{
	unsigned char c;
	const int32_t err =
		static_cast<int32_t>(ZREAD(*pzlib_filefunc_def, filestream, &c, 1));
	if (err == 1) {
		*pi = static_cast<int32_t>(c);
		return UNZ_OK;
	} else
		return ZERROR(*pzlib_filefunc_def, filestream) ? UNZ_ERRNO : UNZ_EOF;
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local int32_t unzlocal_getShort OF
	((zlib_filefunc_def const * pzlib_filefunc_def,
	  voidpf                    filestream,
	  uLong                   * pX));

local int32_t unzlocal_getShort
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream,
	 uLong                   * const pX)
{
	uLong x;
	int32_t i = 0;
	int32_t err;

	err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
	x = static_cast<uLong>(i);

	if (err == UNZ_OK)
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) << 8;

	*pX = err == UNZ_OK ? x : 0;
	return err;
}

local int32_t unzlocal_getLong OF
	((zlib_filefunc_def const * pzlib_filefunc_def,
	  voidpf                    filestream,
	  uLong                   * pX));

local int32_t unzlocal_getLong
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream,
	 uLong                   * const pX)
{
	uLong x;
	int32_t i = 0;
	int32_t err;

	err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
	x = static_cast<uLong>(i);

	if (err == UNZ_OK)
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) << 8;

	if (err == UNZ_OK)
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) << 16;

	if (err == UNZ_OK)
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) << 24;

	*pX = err == UNZ_OK ? x : 0;
	return err;
}


#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local uLong unzlocal_SearchCentralDir OF
	((zlib_filefunc_def const * pzlib_filefunc_def, voidpf filestream));

local uLong unzlocal_SearchCentralDir
	(zlib_filefunc_def const * pzlib_filefunc_def, voidpf filestream)
{
	unsigned char * buf;
	uLong uSizeFile;
	uLong uBackRead;
	uLong uMaxBack  = 0xffff; /* maximum size of global comment */
	uLong uPosFound = 0;

	if (ZSEEK(*pzlib_filefunc_def, filestream, 0, ZLIB_FILEFUNC_SEEK_END))
		return 0;


	uSizeFile = ZTELL(*pzlib_filefunc_def, filestream);

	if (uMaxBack > uSizeFile)
		uMaxBack = uSizeFile;

	buf = static_cast<unsigned char *>(malloc(BUFREADCOMMENT + 4));
	if (not buf)
		return 0;

	uBackRead = 4;
	while (uBackRead < uMaxBack) {
		uLong uReadSize, uReadPos;
		int32_t i;
		if (uBackRead + BUFREADCOMMENT > uMaxBack)
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		uReadPos = uSizeFile - uBackRead;

		uReadSize =
			BUFREADCOMMENT + 4 < uSizeFile - uReadPos ?
			BUFREADCOMMENT + 4 : uSizeFile - uReadPos;
		if
			(ZSEEK
			 	(*pzlib_filefunc_def,
			 	 filestream,
			 	 uReadPos,
			 	 ZLIB_FILEFUNC_SEEK_SET))
			break;

		if (ZREAD(*pzlib_filefunc_def, filestream, buf, uReadSize) != uReadSize)
			break;

		for (i = static_cast<int32_t>(uReadSize) - 3; i-- > 0;)
			if
				(*(buf + i)     == 0x50 and
				 *(buf + i + 1) == 0x4b and
				 *(buf + i + 2) == 0x05 and
				 *(buf + i + 3) == 0x06)
			{
				uPosFound = uReadPos + i;
				break;
			}

		if (uPosFound != 0)
			break;
	}
	free(buf);
	return uPosFound;
}

/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows NT computer "c:\\test\\zlib114.zip" or on an Unix computer
     "zlib/zlib114.zip".
     If the zipfile cannot be opened (file doesn't exist or in not valid), the
       return value is nullptr.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
*/
extern unzFile ZEXPORT unzOpen2
	(char const * path, zlib_filefunc_def * const pzlib_filefunc_def)
{
	unz_s us;
	uLong central_pos, uL;

	//  number of the current dist, used for spaning ZIP, unsupported, always 0
	uLong number_disk;

	//  number the the disk with central dir, used for spaning ZIP, unsupported,
	//  always 0
	uLong number_disk_with_CD;

	//  total number of entries in the central dir (same than number_entry on
	//  nospan)
	uLong number_entry_CD;

	int32_t err = UNZ_OK;

	if (unz_copyright[0] != ' ')
		return nullptr;

	if (not pzlib_filefunc_def)
		fill_fopen_filefunc(&us.z_filefunc);
	else
		us.z_filefunc = *pzlib_filefunc_def;

	us.filestream =
		(*(us.z_filefunc.zopen_file))
		(us.z_filefunc.opaque,
		 path,
		 ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_EXISTING);
	if (not us.filestream)
		return nullptr;

	central_pos = unzlocal_SearchCentralDir(&us.z_filefunc, us.filestream);
	if (central_pos == 0)
		err = UNZ_ERRNO;

	if
		(ZSEEK
		 	(us.z_filefunc, us.filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET))
		err = UNZ_ERRNO;

	//  the signature, already checked
	if (unzlocal_getLong(&us.z_filefunc, us.filestream, &uL) != UNZ_OK)
		err = UNZ_ERRNO;

	//  number of this disk
	if
		(unzlocal_getShort(&us.z_filefunc, us.filestream, &number_disk)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	//  number of the disk with the start of the central directory
	if
		(unzlocal_getShort(&us.z_filefunc, us.filestream, &number_disk_with_CD)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	//  total number of entries in the central dir on this disk
	if
		(unzlocal_getShort(&us.z_filefunc, us.filestream, &us.gi.number_entry)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if //  total number of entries in the central dir
		(unzlocal_getShort(&us.z_filefunc, us.filestream, &number_entry_CD)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(number_entry_CD != us.gi.number_entry
		 or
		 number_disk_with_CD != 0
		 or
		 number_disk != 0)
		err = UNZ_BADZIPFILE;

	//  size of the central directory
	if
		(unzlocal_getLong(&us.z_filefunc, us.filestream, &us.size_central_dir)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	//  offset of start of central directory with respect to the starting disk
	//  number
	if
		(unzlocal_getLong(&us.z_filefunc, us.filestream, &us.offset_central_dir)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	//  zipfile comment length
	if
		(unzlocal_getShort(&us.z_filefunc, us.filestream, &us.gi.size_comment)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(central_pos < us.offset_central_dir + us.size_central_dir
		 and
		 err == UNZ_OK)
		err = UNZ_BADZIPFILE;

	if (err != UNZ_OK) {
		ZCLOSE(us.z_filefunc, us.filestream);
		return nullptr;
	}

	us.byte_before_the_zipfile =
		central_pos - (us.offset_central_dir + us.size_central_dir);
	us.central_pos             = central_pos;
	us.pfile_in_zip_read       = nullptr;
	us.encrypted               = 0;


	unz_s * const s = static_cast<unz_s *>(malloc(sizeof(unz_s)));
	*s = us;
	unzGoToFirstFile(static_cast<unzFile>(s));
	return static_cast<unzFile>(s);
}


extern unzFile ZEXPORT unzOpen (char const * path)
{
	return unzOpen2(path, nullptr);
}

/*
  Close a ZipFile opened with unzipOpen.  If there is files inside the .Zip
  opened with unzipOpenCurrentFile (see later), these files MUST be closed with
  unzipCloseCurrentFile before call unzipClose.  return UNZ_OK if there is no
  problem. */
extern int32_t ZEXPORT unzClose (unzFile file)
{
	if (not file)
		return UNZ_PARAMERROR;
	unz_s * const s = static_cast<unz_s *>(file);

	if (s->pfile_in_zip_read)
		unzCloseCurrentFile(file);

	ZCLOSE(s->z_filefunc, s->filestream);
	free(s);
	return UNZ_OK;
}


/*
   Translate date/time from Dos format to tm_unz (readable more easilty)
*/
local void unzlocal_DosDateToTmuDate (uLong ulDosDate, tm_unz * const ptm)
{
	uLong uDate;
	uDate = static_cast<uLong>(ulDosDate >> 16);
	ptm->tm_mday = static_cast<uInt>  (uDate & 0x0001f);
	ptm->tm_mon  = static_cast<uInt>(((uDate & 0x001E0) / 0x0020) -    1);
	ptm->tm_year = static_cast<uInt>(((uDate & 0x0FE00) / 0x0200) + 1980);

	ptm->tm_hour = static_cast<uInt>((ulDosDate & 0xF800) / 0x800);
	ptm->tm_min  = static_cast<uInt>((ulDosDate & 0x07E0) / 0x020);
	ptm->tm_sec  = static_cast<uInt>(2 * (ulDosDate & 0x1f));
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local int32_t unzlocal_GetCurrentFileInfoInternal OF
	((unzFile file,
	  unz_file_info          * const pfile_info,
	  unz_file_info_internal * const pfile_info_internal,
	  char * const szFileName, uLong fileNameBufferSize,
	  void * const extraField, uLong extraFieldBufferSize,
	  char * const szComment,  uLong commentBufferSize));

local int32_t unzlocal_GetCurrentFileInfoInternal
	(unzFile file,
	 unz_file_info          * pfile_info,
	 unz_file_info_internal * pfile_info_internal,
	 char * const szFileName, uLong fileNameBufferSize,
	 void * const extraField, uLong extraFieldBufferSize,
	 char * const szComment,  uLong commentBufferSize)
{
	unz_file_info file_info;
	unz_file_info_internal file_info_internal;
	int32_t err = UNZ_OK;
	uLong uMagic;
	long lSeek = 0;

	if (not file)
		return UNZ_PARAMERROR;
	unz_s * const s = static_cast<unz_s *>(file);
	if
		(ZSEEK
		 	(s->z_filefunc, s->filestream,
		 	 s->pos_in_central_dir + s->byte_before_the_zipfile,
		 	 ZLIB_FILEFUNC_SEEK_SET))
		err = UNZ_ERRNO;


	//  we check the magic
	if (err == UNZ_OK) {
		if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if (uMagic != 0x02014b50)
			err = UNZ_BADZIPFILE;
	}
	if
		(unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.version)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort
		 	(&s->z_filefunc, s->filestream, &file_info.version_needed)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.flag)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort
		 	(&s->z_filefunc, s->filestream, &file_info.compression_method)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.dosDate)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	unzlocal_DosDateToTmuDate(file_info.dosDate, &file_info.tmu_date);

	if
		(unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.crc)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getLong
		 	(&s->z_filefunc, s->filestream, &file_info.compressed_size)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getLong
		 	(&s->z_filefunc, s->filestream, &file_info.uncompressed_size)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort
		 	(&s->z_filefunc, s->filestream, &file_info.size_filename)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort
		 	(&s->z_filefunc, s->filestream, &file_info.size_file_extra)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort
		 	(&s->z_filefunc, s->filestream, &file_info.size_file_comment)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort
		 	(&s->z_filefunc, s->filestream, &file_info.disk_num_start)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.internal_fa)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.external_fa)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	if
		(unzlocal_getLong
		 	(&s->z_filefunc, s->filestream, &file_info_internal.offset_curfile)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;

	lSeek += file_info.size_filename;
	if (err == UNZ_OK and szFileName) {
		uLong uSizeRead;
		if (file_info.size_filename < fileNameBufferSize) {
			*(szFileName + file_info.size_filename) = '\0';
			uSizeRead = file_info.size_filename;
		} else
			uSizeRead = fileNameBufferSize;

		if
			(file_info.size_filename > 0 && fileNameBufferSize > 0
			 and
			 ZREAD(s->z_filefunc, s->filestream, szFileName, uSizeRead)
			 !=
			 uSizeRead)
			err = UNZ_ERRNO;
		lSeek -= uSizeRead;
	}


	if (err == UNZ_OK && extraField) {
		uLong uSizeRead;
		uSizeRead =
			file_info.size_file_extra < extraFieldBufferSize ?
			file_info.size_file_extra : extraFieldBufferSize;

		if (lSeek) {
			if
				(ZSEEK(s->z_filefunc, s->filestream, lSeek, ZLIB_FILEFUNC_SEEK_CUR)
				 ==
				 0)
				lSeek = 0;
			else
				err = UNZ_ERRNO;
		}
		if
			(file_info.size_file_extra > 0 && extraFieldBufferSize > 0
			 and
			 ZREAD(s->z_filefunc, s->filestream, extraField, uSizeRead)
			 !=
			 uSizeRead)
			err = UNZ_ERRNO;
		lSeek += file_info.size_file_extra - uSizeRead;
	} else
		lSeek += file_info.size_file_extra;


	if (err == UNZ_OK and szComment) {
		uLong uSizeRead;
		if (file_info.size_file_comment<commentBufferSize) {
			*(szComment + file_info.size_file_comment) = '\0';
			uSizeRead = file_info.size_file_comment;
		} else
			uSizeRead = commentBufferSize;

		if (lSeek != 0) {
			if
				(ZSEEK(s->z_filefunc, s->filestream, lSeek, ZLIB_FILEFUNC_SEEK_CUR)
				 ==
				 0)
				lSeek = 0;
		} else
			err = UNZ_ERRNO;
		if (file_info.size_file_comment > 0 && commentBufferSize > 0)
			if
				(ZREAD(s->z_filefunc, s->filestream, szComment, uSizeRead)
				 !=
				 uSizeRead)
				err = UNZ_ERRNO;
		lSeek += file_info.size_file_comment - uSizeRead;
	} else
		lSeek += file_info.size_file_comment;

	if (err == UNZ_OK && pfile_info)
		*pfile_info = file_info;

	if (err == UNZ_OK && pfile_info_internal)
		*pfile_info_internal = file_info_internal;

	return err;
}



//  Write info about the ZipFile in the *pglobal_info structure. No preparation
//  of the structure is needed. Return UNZ_OK if there is no problem.
extern int32_t ZEXPORT unzGetCurrentFileInfo
	(unzFile file, unz_file_info * const pfile_info,
	 char * const szFileName, uLong fileNameBufferSize,
	 void * const extraField, uLong extraFieldBufferSize,
	 char * const szComment,  uLong commentBufferSize)
{
	return
		unzlocal_GetCurrentFileInfoInternal
			(file, pfile_info, nullptr,
			 szFileName, fileNameBufferSize,
			 extraField, extraFieldBufferSize,
			 szComment,  commentBufferSize);
}

/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
extern int32_t ZEXPORT unzGoToFirstFile (unzFile file)
{
	int32_t err = UNZ_OK;
	if (not file)
		return UNZ_PARAMERROR;
	unz_s * const s = static_cast<unz_s *>(file);
	s->pos_in_central_dir = s->offset_central_dir;
	s->num_file           = 0;
	err =
		unzlocal_GetCurrentFileInfoInternal
			(file, &s->cur_file_info, &s->cur_file_info_internal,
			 nullptr, 0, nullptr, 0, nullptr, 0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
extern int32_t ZEXPORT unzGoToNextFile (unzFile file)
{
	int32_t err;

	if (not file)
		return UNZ_PARAMERROR;
	unz_s * const s = static_cast<unz_s *>(file);
	if (!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;
	if (s->gi.number_entry != 0xffff)    /* 2^16 files overflow hack */
		if (s->num_file + 1 == s->gi.number_entry)
			return UNZ_END_OF_LIST_OF_FILE;

	s->pos_in_central_dir +=
		SIZECENTRALDIRITEM               +
		s->cur_file_info.size_filename   +
		s->cur_file_info.size_file_extra +
		s->cur_file_info.size_file_comment;
	++s->num_file;
	err =
		unzlocal_GetCurrentFileInfoInternal
			(file, &s->cur_file_info, &s->cur_file_info_internal,
			 nullptr, 0, nullptr, 0, nullptr, 0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}


/*
///////////////////////////////////////////
// Contributed by Ryan Haksi (mailto://cryogen@infoserve.net)
// I need random access
//
// Further optimization could be realized by adding an ability
// to cache the directory in memory. The goal being a single
// comprehensive file read to put the file I need in a memory.
*/

/*
// Unzip Helper Functions - should be here?
///////////////////////////////////////////
*/

//  Read the local header of the current zipfile. Check the coherency of the
//  local header and info in the end of central directory about this file store
//  in *piSizeVar the size of extra info in local header (filename and size of
//  extra field data).
local int32_t unzlocal_CheckCurrentFileCoherencyHeader
	(unz_s * const s,
	 uInt  * const piSizeVar,
	 uLong * const poffset_local_extrafield,
	 uInt  * const psize_local_extrafield)
{
	uLong uMagic, uData, uFlags;
	uLong size_filename;
	uLong size_extra_field;
	int32_t err = UNZ_OK;

	*piSizeVar                = 0;
	*poffset_local_extrafield = 0;
	*psize_local_extrafield   = 0;

	if
		(ZSEEK
		 	(s->z_filefunc, s->filestream,
		 	 s->cur_file_info_internal.offset_curfile
		 	 +
		 	 s->byte_before_the_zipfile,
		 	 ZLIB_FILEFUNC_SEEK_SET))
		return UNZ_ERRNO;


	if (err == UNZ_OK) {
		if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if (uMagic != 0x04034b50)
			err = UNZ_BADZIPFILE;
	}

	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uFlags) != UNZ_OK)
		err = UNZ_ERRNO;

	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;
	else if (err == UNZ_OK && uData != s->cur_file_info.compression_method)
		err = UNZ_BADZIPFILE;

	if
		(err == UNZ_OK
		 and
		 s->cur_file_info.compression_method != 0
		 and
		 s->cur_file_info.compression_method != Z_DEFLATED)
		err = UNZ_BADZIPFILE;

	/* date/time */
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;

	/* crc */
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;
	else if
		(err == UNZ_OK
		 and
		 uData != s->cur_file_info.crc
		 and
		 (uFlags & 8) == 0)
		err = UNZ_BADZIPFILE;

	/* size compr */
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;
	else if
		(err == UNZ_OK
		 and
		 uData != s->cur_file_info.compressed_size
		 and
		 (uFlags & 8) == 0)
		err = UNZ_BADZIPFILE;

	/* size uncompr */
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK)
		err = UNZ_ERRNO;
	else if
		(err == UNZ_OK
		 and
		 uData != s->cur_file_info.uncompressed_size
		 and
		 (uFlags & 8) == 0)
		err = UNZ_BADZIPFILE;


	if
		(unzlocal_getShort(&s->z_filefunc, s->filestream, &size_filename)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;
	else if (err == UNZ_OK && size_filename != s->cur_file_info.size_filename)
		err = UNZ_BADZIPFILE;

	*piSizeVar += static_cast<uInt>(size_filename);

	if
		(unzlocal_getShort(&s->z_filefunc, s->filestream, &size_extra_field)
		 !=
		 UNZ_OK)
		err = UNZ_ERRNO;
	*poffset_local_extrafield =
		s->cur_file_info_internal.offset_curfile +
		SIZEZIPLOCALHEADER                       +
		size_filename;
	*psize_local_extrafield = static_cast<uInt>(size_extra_field);

	*piSizeVar += static_cast<uInt>(size_extra_field);

	return err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
extern int32_t ZEXPORT unzOpenCurrentFile3
	(unzFile file,
	 int32_t    * const method,
	 int32_t    * const level,
	 int32_t      const raw,
	 char const * const password)
{
	int32_t err = UNZ_OK;
	uInt iSizeVar;
	file_in_zip_read_info_s * pfile_in_zip_read_info;
	uLong offset_local_extrafield;  /* offset of the local extra field */
	uInt  size_local_extrafield;    /* size of the local extra field */
	if (password)
		return UNZ_PARAMERROR;

	if (not file)
		return UNZ_PARAMERROR;
	unz_s * const s = static_cast<unz_s *>(file);
	if (!s->current_file_ok)
		return UNZ_PARAMERROR;

	if (s->pfile_in_zip_read)
		unzCloseCurrentFile(file);

	if
		(unzlocal_CheckCurrentFileCoherencyHeader
		 	(s, &iSizeVar, &offset_local_extrafield, &size_local_extrafield)
		 !=
		 UNZ_OK)
		return UNZ_BADZIPFILE;

	pfile_in_zip_read_info =
		static_cast<file_in_zip_read_info_s *>
			(malloc(sizeof(file_in_zip_read_info_s)));
	if (not pfile_in_zip_read_info)
		return UNZ_INTERNALERROR;

	pfile_in_zip_read_info->read_buffer             =
		static_cast<char *>(malloc(UNZ_BUFSIZE));
	pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
	pfile_in_zip_read_info->size_local_extrafield   = size_local_extrafield;
	pfile_in_zip_read_info->pos_local_extrafield    = 0;
	pfile_in_zip_read_info->raw                     = raw;

	if (not pfile_in_zip_read_info->read_buffer) {
		free(pfile_in_zip_read_info);
		return UNZ_INTERNALERROR;
	}

	pfile_in_zip_read_info->stream_initialized = 0;

	if (method)
		*method = static_cast<int32_t>(s->cur_file_info.compression_method);

	if (level) {
		*level = 6;
		switch (s->cur_file_info.flag & 0x06) {
		case 6 : *level = 1; break;
		case 4 : *level = 2; break;
		case 2 : *level = 9; break;
		};
	}

	if
		(s->cur_file_info.compression_method != 0
		 and
		 s->cur_file_info.compression_method != Z_DEFLATED)
		err = UNZ_BADZIPFILE;

	pfile_in_zip_read_info->crc32_wait              = s->cur_file_info.crc;
	pfile_in_zip_read_info->crc32                   = 0;
	pfile_in_zip_read_info->compression_method      =
		s->cur_file_info.compression_method;
	pfile_in_zip_read_info->filestream              = s->filestream;
	pfile_in_zip_read_info->z_filefunc              = s->z_filefunc;
	pfile_in_zip_read_info->byte_before_the_zipfile =
		s->byte_before_the_zipfile;

	pfile_in_zip_read_info->stream.total_out = 0;

	if (s->cur_file_info.compression_method == Z_DEFLATED and not raw) {
		pfile_in_zip_read_info->stream.zalloc   = nullptr;
		pfile_in_zip_read_info->stream.zfree    = nullptr;
		pfile_in_zip_read_info->stream.opaque   = nullptr;
		pfile_in_zip_read_info->stream.next_in  = nullptr;
		pfile_in_zip_read_info->stream.avail_in = 0;

		err = inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
		if (err == Z_OK)
			pfile_in_zip_read_info->stream_initialized = 1;
		else
			return err;
		//  windowBits is passed < 0 to tell that there is no zlib header. Note
		//  that in this case inflate *requires* an extra "dummy" byte after the
		//  compressed stream in order to complete decompression and return
		//  Z_STREAM_END. In unzip, i don't wait absolutely Z_STREAM_END because
		//  I known the size of both compressed and uncompressed data.
	}
	pfile_in_zip_read_info->rest_read_compressed =
		s->cur_file_info.compressed_size;
	pfile_in_zip_read_info->rest_read_uncompressed =
		s->cur_file_info.uncompressed_size;


	pfile_in_zip_read_info->pos_in_zipfile =
		s->cur_file_info_internal.offset_curfile +
		SIZEZIPLOCALHEADER                       +
		iSizeVar;

	pfile_in_zip_read_info->stream.avail_in = 0;

	s->pfile_in_zip_read = pfile_in_zip_read_info;

	return UNZ_OK;
}

extern int32_t ZEXPORT unzOpenCurrentFile (unzFile file)
{
	return unzOpenCurrentFile3(file, nullptr, nullptr, 0, nullptr);
}

/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/
extern int32_t ZEXPORT unzReadCurrentFile
		(unzFile file, voidp buf, unsigned len)
{
	int32_t                   err   = UNZ_OK;
	uInt                      iRead = 0;
	file_in_zip_read_info_s * pfile_in_zip_read_info;
	if (not file)
		return UNZ_PARAMERROR;
	unz_s * const s = static_cast<unz_s *>(file);
	pfile_in_zip_read_info = s->pfile_in_zip_read;

	if (not pfile_in_zip_read_info)
		return UNZ_PARAMERROR;


	if (not pfile_in_zip_read_info->read_buffer)
		return UNZ_END_OF_LIST_OF_FILE;
	if (len == 0)
		return 0;

	pfile_in_zip_read_info->stream.next_out = static_cast<Bytef *>(buf);

	pfile_in_zip_read_info->stream.avail_out = static_cast<uInt>(len);

	if
		(len > pfile_in_zip_read_info->rest_read_uncompressed
		 and
		 not pfile_in_zip_read_info->raw)
		pfile_in_zip_read_info->stream.avail_out =
			static_cast<uInt>(pfile_in_zip_read_info->rest_read_uncompressed);

	if
		((len
		  >
		  pfile_in_zip_read_info->rest_read_compressed
		  +
		  pfile_in_zip_read_info->stream.avail_in)
		 &&
		 pfile_in_zip_read_info->raw)
		pfile_in_zip_read_info->stream.avail_out =
		static_cast<uInt>(pfile_in_zip_read_info->rest_read_compressed)
		+
		pfile_in_zip_read_info->stream.avail_in;

	while (pfile_in_zip_read_info->stream.avail_out > 0) {
		if
			(pfile_in_zip_read_info->stream.avail_in == 0
			 and
			 pfile_in_zip_read_info->rest_read_compressed > 0)
		{
			uInt uReadThis = UNZ_BUFSIZE;
			if (pfile_in_zip_read_info->rest_read_compressed < uReadThis)
				uReadThis = static_cast<uInt>
					(pfile_in_zip_read_info->rest_read_compressed);
			if (uReadThis == 0)
				return UNZ_EOF;
			if
				(ZSEEK
				 	(pfile_in_zip_read_info->z_filefunc,
				 	 pfile_in_zip_read_info->filestream,
				 	 pfile_in_zip_read_info->pos_in_zipfile +
				 	 pfile_in_zip_read_info->byte_before_the_zipfile,
				 	 ZLIB_FILEFUNC_SEEK_SET))
				return UNZ_ERRNO;
			if
				(ZREAD
				 	(pfile_in_zip_read_info->z_filefunc,
				 	 pfile_in_zip_read_info->filestream,
				 	 pfile_in_zip_read_info->read_buffer,
				 	 uReadThis)
				 !=
				 uReadThis)
				return UNZ_ERRNO;


			pfile_in_zip_read_info->pos_in_zipfile       += uReadThis;

			pfile_in_zip_read_info->rest_read_compressed -= uReadThis;

			pfile_in_zip_read_info->stream.next_in =
				reinterpret_cast<Bytef *>(pfile_in_zip_read_info->read_buffer);
			pfile_in_zip_read_info->stream.avail_in =
				static_cast<uInt>(uReadThis);
		}

		if
			(pfile_in_zip_read_info->compression_method == 0
			 or
			 pfile_in_zip_read_info->raw)
		{
			uInt uDoCopy, i;

			if
				(pfile_in_zip_read_info->stream.avail_in == 0
				 and
				 pfile_in_zip_read_info->rest_read_compressed == 0)
				return iRead == 0 ? UNZ_EOF : iRead;

			uDoCopy =
				pfile_in_zip_read_info->stream.avail_out
				<
				pfile_in_zip_read_info->stream.avail_in
				?
				pfile_in_zip_read_info->stream.avail_out
				:
				pfile_in_zip_read_info->stream.avail_in;

			for (i = 0; i < uDoCopy; ++i)
				*(pfile_in_zip_read_info->stream.next_out + i) =
					*(pfile_in_zip_read_info->stream.next_in + i);

			pfile_in_zip_read_info->crc32 =
				crc32
				(pfile_in_zip_read_info->crc32,
				 pfile_in_zip_read_info->stream.next_out,
				 uDoCopy);
			pfile_in_zip_read_info->rest_read_uncompressed -= uDoCopy;
			pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
			pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
			pfile_in_zip_read_info->stream.next_out += uDoCopy;
			pfile_in_zip_read_info->stream.next_in += uDoCopy;
			pfile_in_zip_read_info->stream.total_out += uDoCopy;
			iRead += uDoCopy;
		} else {
			uLong uTotalOutBefore, uTotalOutAfter;
			uLong uOutThis;
			int32_t flush = Z_SYNC_FLUSH;

			uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
			Bytef const * const bufBefore =
				pfile_in_zip_read_info->stream.next_out;

			err = inflate(&pfile_in_zip_read_info->stream, flush);

			if (err >= 0 && pfile_in_zip_read_info->stream.msg)
				err = Z_DATA_ERROR;

			uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
			uOutThis = uTotalOutAfter - uTotalOutBefore;

			pfile_in_zip_read_info->crc32 =
				crc32
				(pfile_in_zip_read_info->crc32,
				 bufBefore,
				 static_cast<uInt>(uOutThis));

			pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;

			iRead += static_cast<uInt>(uTotalOutAfter - uTotalOutBefore);

			if (err == Z_STREAM_END)
				return iRead == 0 ? UNZ_EOF : iRead;
			if (err != Z_OK)
				break;
		}
	}

	if (err == Z_OK)
		return iRead;
	return err;
}


/*
  Close the file in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
extern int32_t ZEXPORT unzCloseCurrentFile (unzFile file)
{
	int32_t                   err = UNZ_OK;
	file_in_zip_read_info_s * pfile_in_zip_read_info;
	if (not file)
		return UNZ_PARAMERROR;
	unz_s * const s = static_cast<unz_s *>(file);
	pfile_in_zip_read_info = s->pfile_in_zip_read;

	if (not pfile_in_zip_read_info)
		return UNZ_PARAMERROR;


	if
		(pfile_in_zip_read_info->rest_read_uncompressed == 0
		 and
		 not pfile_in_zip_read_info->raw)
	{
		if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
			err = UNZ_CRCERROR;
	}


	free(pfile_in_zip_read_info->read_buffer);
	pfile_in_zip_read_info->read_buffer = nullptr;
	if (pfile_in_zip_read_info->stream_initialized)
		inflateEnd(&pfile_in_zip_read_info->stream);

	pfile_in_zip_read_info->stream_initialized = 0;
	free(pfile_in_zip_read_info);

	s->pfile_in_zip_read = nullptr;

	return err;
}


/* zip.c -- IO on .zip files using zlib
   Version 1.01, May 8th, 2004

   Copyright (C) 1998-2004 Gilles Vollant

   Read zip.h for more info
*/


#include "zip.h"

/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform depedent */
#endif

#ifndef Z_BUFSIZE
#define Z_BUFSIZE (16384)
#endif

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
const char zip_copyright[] =
" zip 1.01 Copyright 1998-2004 Gilles Vollant - "
"http://www.winimage.com/zLibDll";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)

#define SIZECENTRALHEADER (0x2e) /* 46 */

typedef struct linkedlist_datablock_internal_s
{
	struct linkedlist_datablock_internal_s * next_datablock;
	uLong  avail_in_this_block;
	uLong  filled_in_this_block;
	uLong  unused; //  for future use and alignment
	unsigned char data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

typedef struct linkedlist_data_s
{
	linkedlist_datablock_internal * first_block;
	linkedlist_datablock_internal * last_block;
} linkedlist_data;


typedef struct
{
	z_stream stream;            /* zLib stream structure for inflate */
	int32_t stream_initialized; /* 1 is stream is initialized */
	uInt pos_in_buffered_data;  /* last written byte in buffered_data */

	uLong pos_local_header;     /* offset of the local header of the file
                                     currently writing */
	char * central_header;      /* central header data for the current file */
	uLong size_centralheader;   /* size of the central header for cur file */
	uLong flag;                 /* flag of the file currently writing */

	int32_t method;             /* compression method of file currently wr.*/
	int32_t raw;                /* 1 for directly writing raw data */
	Byte buffered_data[Z_BUFSIZE]; //  buffer contain compressed data to be writ
	uLong dosDate;
	uLong crc32;
	int32_t encrypt;
} curfile_info;

typedef struct
{
	zlib_filefunc_def z_filefunc;
	voidpf filestream;        /* io structore of the zipfile */
	linkedlist_data central_dir; //  datablock with central dir in construction
	int32_t  in_opened_file_inzip; //  1 if a file in the zip is currently writ.
	curfile_info ci;            /* info on the file curretly writing */

	uLong begin_pos;            /* position of the beginning of the zipfile */
	uLong add_position_when_writting_offset;
	uLong number_entry;
} zip_internal;



local linkedlist_datablock_internal * allocate_new_datablock()
{
	linkedlist_datablock_internal * const ldi =
		static_cast<linkedlist_datablock_internal *>
			(malloc(sizeof(linkedlist_datablock_internal)));
	if (ldi) {
		ldi->next_datablock       = nullptr;
		ldi->filled_in_this_block = 0;
		ldi->avail_in_this_block  = SIZEDATA_INDATABLOCK;
	}
	return ldi;
}

local void free_datablock(linkedlist_datablock_internal * ldi)
{
	while (ldi) {
		linkedlist_datablock_internal * const ldinext = ldi->next_datablock;
		free(ldi);
		ldi = ldinext;
	}
}

local void init_linkedlist(linkedlist_data * const ll)
{
	ll->first_block = ll->last_block = nullptr;
}

/* IS NOT USED
 * local void free_linkedlist(linkedlist_data* ll)
{
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = 0;
}
*/

local int32_t add_data_in_datablock
	(linkedlist_data * const ll, void const * const buf, uLong len)
{
	linkedlist_datablock_internal * ldi;
	unsigned char const           * from_copy;

	if (not ll)
		return ZIP_INTERNALERROR;

	if (not ll->last_block) {
		ll->first_block = ll->last_block = allocate_new_datablock();
		if (not ll->first_block)
			return ZIP_INTERNALERROR;
	}

	ldi = ll->last_block;
	from_copy = static_cast<const unsigned char *>(buf);

	while (len > 0) {
		uInt            copy_this;
		uInt            i;
		unsigned char * to_copy;

		if (ldi->avail_in_this_block == 0) {
			ldi->next_datablock = allocate_new_datablock();
			if (not ldi->next_datablock)
				return ZIP_INTERNALERROR;
			ldi = ldi->next_datablock;
			ll->last_block = ldi;
		}

		copy_this =
			ldi->avail_in_this_block < len ?
			static_cast<uInt>(ldi->avail_in_this_block) : static_cast<uInt>(len);

		to_copy = &(ldi->data[ldi->filled_in_this_block]);

		for (i = 0; i < copy_this; ++i)
			*(to_copy + i) = *(from_copy + i);

		ldi->filled_in_this_block += copy_this;
		ldi->avail_in_this_block  -= copy_this;
		from_copy                 += copy_this;
		len                       -= copy_this;
	}
	return ZIP_OK;
}



/****************************************************************************/

#ifndef NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 or 4 (byte, short or long)
*/

local int32_t ziplocal_putValue OF
	((zlib_filefunc_def const * pzlib_filefunc_def,
	  voidpf                    filestream,
	  uLong                     x,
	  int32_t                   nbByte));
local int32_t ziplocal_putValue
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream,
	 uLong                           x,
	 int32_t                   const nbByte)
{
	unsigned char buf[4];
	int32_t n;
	for (n = 0; n < nbByte; ++n) {
		buf[n] = static_cast<unsigned char>(x & 0xff);
		x >>= 8;
	}
	// data overflow - hack for ZIP64 (X Roche)
	if (x != 0)
		for (n = 0; n < nbByte; ++n)
			buf[n] = 0xff;

	return
		ZWRITE(*pzlib_filefunc_def, filestream, buf, nbByte)
		!=
		static_cast<uLong>(nbByte)
		?
		ZIP_ERRNO : ZIP_OK;
}

local void ziplocal_putValue_inmemory OF
	((void * dest, uLong x, int32_t nbByte));
local void ziplocal_putValue_inmemory
	(void * const dest, uLong x, int32_t const nbByte)
{
	unsigned char * const buf = static_cast<unsigned char *>(dest);
	for (int32_t n = 0; n < nbByte; ++n) {
		buf[n] = static_cast<unsigned char>(x & 0xff);
		x >>= 8;
	}

	if (x != 0) //  data overflow - hack for ZIP64
		for (int32_t n = 0; n < nbByte; ++n)
			buf[n] = 0xff;
}

/****************************************************************************/


local uLong ziplocal_TmzDateToDosDate(tm_zip const * ptm, uLong) {
	uLong year = static_cast<uLong>(ptm->tm_year);
	if      (year > 1980)
		year -= 1980;
	else if (year >   80)
		year -= 80;
	return
		static_cast<uLong>
			((ptm->tm_mday + 32 * (ptm->tm_mon + 1) + 512 * year) << 16)
		|
		(ptm->tm_sec / 2 +
		 32 * ptm->tm_min +
		 2048 * static_cast<uLong>(ptm->tm_hour));
}


/****************************************************************************/

local int32_t ziplocal_getByte OF
	((zlib_filefunc_def const * pzlib_filefunc_def,
	  voidpf                    filestream,
	  int32_t                 * pi));

local int32_t ziplocal_getByte
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream,
	 int32_t                 * const pi)
{
	unsigned char c;
	const int32_t err =
		static_cast<int32_t>(ZREAD(*pzlib_filefunc_def, filestream, &c, 1));
	if (err == 1) {
		*pi = static_cast<int32_t>(c);
		return ZIP_OK;
	} else
		return ZERROR(*pzlib_filefunc_def, filestream) ? ZIP_ERRNO : ZIP_EOF;
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local int32_t ziplocal_getShort OF
	((zlib_filefunc_def const * pzlib_filefunc_def,
	  voidpf                    filestream,
	  uLong                   * pX));

local int32_t ziplocal_getShort
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream,
	 uLong                   * const pX)
{
	uLong x;
	int32_t i = 0;
	int32_t err;

	err = ziplocal_getByte(pzlib_filefunc_def, filestream, &i);
	x = static_cast<uLong>(i);

	if (err == ZIP_OK)
		err = ziplocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) << 8;

	*pX = err == ZIP_OK ? x : 0;
	return err;
}

local int32_t ziplocal_getLong OF
	((zlib_filefunc_def const * pzlib_filefunc_def,
	  voidpf                    filestream,
	  uLong                   * pX));

local int32_t ziplocal_getLong
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream,
	 uLong                   * const pX)
{
	uLong x;
	int32_t i = 0;
	int32_t err;

	err = ziplocal_getByte(pzlib_filefunc_def, filestream, &i);
	x  = static_cast<uLong>(i);

	if (err == ZIP_OK)
		err = ziplocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) <<  8;

	if (err == ZIP_OK)
		err = ziplocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) << 16;

	if (err == ZIP_OK)
		err = ziplocal_getByte(pzlib_filefunc_def, filestream, &i);
	x += static_cast<uLong>(i) << 24;

	*pX = err == ZIP_OK ? x : 0;
	return err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local uLong ziplocal_SearchCentralDir OF
	((zlib_filefunc_def const * pzlib_filefunc_def, voidpf filestream));

local uLong ziplocal_SearchCentralDir
	(zlib_filefunc_def const * const pzlib_filefunc_def,
	 voidpf                          filestream)
{
	unsigned char * buf;
	uLong uSizeFile;
	uLong uBackRead;
	uLong uMaxBack  = 0xffff; /* maximum size of global comment */
	uLong uPosFound = 0;

	if (ZSEEK(*pzlib_filefunc_def, filestream, 0, ZLIB_FILEFUNC_SEEK_END))
		return 0;


	uSizeFile = ZTELL(*pzlib_filefunc_def, filestream);

	if (uMaxBack > uSizeFile)
		uMaxBack = uSizeFile;

	buf = static_cast<unsigned char *>(malloc(BUFREADCOMMENT + 4));
	if (not buf)
		return 0;

	uBackRead = 4;
	while (uBackRead < uMaxBack) {
		uLong uReadSize, uReadPos;
		int32_t i;
		if (uBackRead + BUFREADCOMMENT > uMaxBack)
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		uReadPos = uSizeFile - uBackRead;

		uReadSize =
			BUFREADCOMMENT + 4 < uSizeFile - uReadPos ?
			BUFREADCOMMENT + 4 : uSizeFile - uReadPos;
		if
			(ZSEEK
			 	(*pzlib_filefunc_def,
			 	 filestream,
			 	 uReadPos,
			 	 ZLIB_FILEFUNC_SEEK_SET))
			break;

		if (ZREAD(*pzlib_filefunc_def, filestream, buf, uReadSize) != uReadSize)
			break;

		for (i = static_cast<int32_t>(uReadSize) - 3; i-- > 0;)
			if
				(*(buf + i)     == 0x50 and
				 *(buf + i + 1) == 0x4b and
				 *(buf + i + 2) == 0x05 and
				 *(buf + i + 3) == 0x06)
			{
				uPosFound = uReadPos + i;
				break;
			}

		if (uPosFound)
			break;
	}
	free(buf);
	return uPosFound;
}
#endif /* !NO_ADDFILEINEXISTINGZIP*/

/************************************************************/
extern zipFile ZEXPORT zipOpen2
	(const char        * pathname,
	 int32_t             append,
	 zipcharpc         *,
	 zlib_filefunc_def * pzlib_filefunc_def)
{
	zip_internal ziinit;
	int32_t err = ZIP_OK;


	if (not pzlib_filefunc_def)
		fill_fopen_filefunc(&ziinit.z_filefunc);
	else
		ziinit.z_filefunc = *pzlib_filefunc_def;

	ziinit.filestream =
		(*ziinit.z_filefunc.zopen_file)
		(ziinit.z_filefunc.opaque,
		 pathname,
		 append == APPEND_STATUS_CREATE ?
		 ZLIB_FILEFUNC_MODE_READ  |
		 ZLIB_FILEFUNC_MODE_WRITE |
		 ZLIB_FILEFUNC_MODE_CREATE
		 :
		 ZLIB_FILEFUNC_MODE_READ  |
		 ZLIB_FILEFUNC_MODE_WRITE |
		 ZLIB_FILEFUNC_MODE_EXISTING);

	if (not ziinit.filestream)
		return nullptr;
	ziinit.begin_pos = ZTELL(ziinit.z_filefunc, ziinit.filestream);
	ziinit.in_opened_file_inzip = 0;
	ziinit.ci.stream_initialized = 0;
	ziinit.number_entry = 0;
	ziinit.add_position_when_writting_offset = 0;
	init_linkedlist(&(ziinit.central_dir));


	zip_internal * const zi =
		static_cast<zip_internal *>(malloc(sizeof(zip_internal)));
	if (not zi) {
		ZCLOSE(ziinit.z_filefunc, ziinit.filestream);
		return nullptr;
	}

	//  now we add file in a zipfile
#    ifndef NO_ADDFILEINEXISTINGZIP
	if (append == APPEND_STATUS_ADDINZIP) {
		uLong byte_before_the_zipfile; //  byte before the zipfile, (>0 for sfx)

		uLong size_central_dir;     /* size of the central directory  */
		uLong offset_central_dir;   /* offset of start of central directory */
		uLong central_pos, uL;

		//  number of the current dist, used for spaning ZIP, unsupported, always
		//  0
		uLong number_disk;


		//  number the the disk with central dir, used for spaning ZIP,
		//  unsupported, always 0
		uLong number_disk_with_CD;

		uLong number_entry;

		//  total number of entries in the central dir (same than number_entry on
		//  nospan)
		uLong number_entry_CD;

		uLong size_comment;

		central_pos =
			ziplocal_SearchCentralDir(&ziinit.z_filefunc, ziinit.filestream);
		if (central_pos == 0)
			err = ZIP_ERRNO;

		if
			(ZSEEK
			 	(ziinit.z_filefunc,
			 	 ziinit.filestream,
			 	 central_pos,
			 	 ZLIB_FILEFUNC_SEEK_SET)
			 !=
			 0)
			err = ZIP_ERRNO;

		if //  the signature, already checked
			(ziplocal_getLong
			 	(&ziinit.z_filefunc, ziinit.filestream, &uL)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		if //  number of this disk
			(ziplocal_getShort
			 	(&ziinit.z_filefunc, ziinit.filestream, &number_disk)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		if //  number of the disk with the start of the central directory
			(ziplocal_getShort
			 	(&ziinit.z_filefunc, ziinit.filestream, &number_disk_with_CD)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		if //  total number of entries in the central dir on this disk
			(ziplocal_getShort
			 	(&ziinit.z_filefunc, ziinit.filestream, &number_entry)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		if //  total number of entries in the central dir
			(ziplocal_getShort
			 	(&ziinit.z_filefunc, ziinit.filestream, &number_entry_CD)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		if
			(number_entry_CD != number_entry
			 or
			 number_disk_with_CD != 0
			 or
			 number_disk != 0)
			err = ZIP_BADZIPFILE;

		if // size of the central directory
			(ziplocal_getLong
			 	(&ziinit.z_filefunc, ziinit.filestream, &size_central_dir)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		//  offset of start of central directory with respect to the starting
		//  disk number
		if
			(ziplocal_getLong
			 	(&ziinit.z_filefunc, ziinit.filestream, &offset_central_dir)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		if // zipfile comment length
			(ziplocal_getShort
			 	(&ziinit.z_filefunc, ziinit.filestream, &size_comment)
			 !=
			 ZIP_OK)
			err = ZIP_ERRNO;

		if
			(central_pos < offset_central_dir + size_central_dir
			 and
			 err == ZIP_OK)
			err = ZIP_BADZIPFILE;

		if (err != ZIP_OK) {
			ZCLOSE(ziinit.z_filefunc, ziinit.filestream);
			free(zi);
			return nullptr;
		}

		byte_before_the_zipfile =
			central_pos - (offset_central_dir + size_central_dir);
		ziinit.add_position_when_writting_offset = byte_before_the_zipfile;

		{
			uLong size_central_dir_to_read = size_central_dir;
			size_t buf_size = SIZEDATA_INDATABLOCK;
			void * const buf_read = malloc(buf_size);
			if
				(ZSEEK
				 	(ziinit.z_filefunc, ziinit.filestream,
				 	 offset_central_dir + byte_before_the_zipfile,
				 	 ZLIB_FILEFUNC_SEEK_SET)
				 !=
				 0)
				err = ZIP_ERRNO;

			while (size_central_dir_to_read > 0 and err == ZIP_OK) {
				uLong read_this = SIZEDATA_INDATABLOCK;
				if (read_this > size_central_dir_to_read)
					read_this = size_central_dir_to_read;
				if
					(ZREAD
					 	(ziinit.z_filefunc, ziinit.filestream, buf_read, read_this)
					 !=
					 read_this)
					err = ZIP_ERRNO;

				if (err == ZIP_OK)
					err =
						add_data_in_datablock
							(&ziinit.central_dir,
							 buf_read,
							 static_cast<uLong>(read_this));
				size_central_dir_to_read -= read_this;
			}
			free(buf_read);
		}
		ziinit.begin_pos = byte_before_the_zipfile;
		ziinit.number_entry = number_entry_CD;

		if
			(ZSEEK
			 	(ziinit.z_filefunc, ziinit.filestream,
			 	 offset_central_dir + byte_before_the_zipfile,
			 	 ZLIB_FILEFUNC_SEEK_SET)
			 !=
			 0)
			err = ZIP_ERRNO;
	}
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

	if (err != ZIP_OK) {
		free(zi);
		return nullptr;
	} else {
		*zi = ziinit;
		return static_cast<zipFile>(zi);
	}
}

extern zipFile ZEXPORT zipOpen
	(char const * const pathname, int32_t const append)
{
	return zipOpen2(pathname, append, nullptr, nullptr);
}

extern int32_t ZEXPORT zipOpenNewFileInZip3
	(zipFile              file,
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
	 uLong)
{
	zip_internal * zi;
	uInt size_filename;
	uInt size_comment;
	uInt i;
	int32_t err = ZIP_OK;

	if (not file)
		return ZIP_PARAMERROR;
	if (method != 0 && method != Z_DEFLATED)
		return ZIP_PARAMERROR;

	zi = static_cast<zip_internal *>(file);

	if (zi->in_opened_file_inzip == 1) {
		err = zipCloseFileInZip (file);
		if (err != ZIP_OK)
			return err;
	}


	if (not filename)
		filename = "-";

	size_comment = comment ? static_cast<uInt>(strlen(comment)) : 0;

	size_filename = static_cast<uInt>(strlen(filename));

	zi->ci.dosDate =
		not zipfi           ? 0              :
		zipfi->dosDate      ? zipfi->dosDate :
		ziplocal_TmzDateToDosDate(&zipfi->tmz_date, zipfi->dosDate);

	zi->ci.flag = 0;
	if (level == 8 || level == 9)
		zi->ci.flag |= 2;
	if (level == 2)
		zi->ci.flag |= 4;
	if (level == 1)
		zi->ci.flag |= 6;
	if (password)
		zi->ci.flag |= 1;

	zi->ci.crc32                = 0;
	zi->ci.method               = method;
	zi->ci.encrypt              = 0;
	zi->ci.stream_initialized   = 0;
	zi->ci.pos_in_buffered_data = 0;
	zi->ci.raw                  = raw;
	zi->ci.pos_local_header     = ZTELL(zi->z_filefunc, zi->filestream);
	zi->ci.size_centralheader   =
		SIZECENTRALHEADER      +
		size_filename          +
		size_extrafield_global +
		size_comment;
	zi->ci.central_header       =
		static_cast<char *>(malloc(static_cast<uInt>(zi->ci.size_centralheader)));

	ziplocal_putValue_inmemory
		(zi->ci.central_header +  0,
		 static_cast<uLong>(CENTRALHEADERMAGIC),
		 4);
	// version info
	ziplocal_putValue_inmemory
		(zi->ci.central_header +  4,
		 static_cast<uLong>(VERSIONMADEBY),
		 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header +  6, 20UL,
		 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header +  8,
		 static_cast<uLong>(zi->ci.flag),
		 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 10,
		 static_cast<uLong>(zi->ci.method),
		 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 12,
		 static_cast<uLong>(zi->ci.dosDate),
		 4);
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 16, 0UL,
		 4); //  crc
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 20, 0UL,
		 4); //  compr size
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 24, 0UL,
		 4); //  uncompr size
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 28,
		 static_cast<uLong>(size_filename),
		 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 30,
		 static_cast<uLong>(size_extrafield_global),
		 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 32,
		 static_cast<uLong>(size_comment),
		 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 34,
		 0UL,
		 2); //  disk nm start

	ziplocal_putValue_inmemory
		(zi->ci.central_header + 36,
		 static_cast<uLong>(zipfi ? zipfi->internal_fa : 0),
		 2);

	ziplocal_putValue_inmemory
		(zi->ci.central_header + 38,
		 static_cast<uLong>(zipfi ? zipfi->external_fa : 0),
		 4);

	ziplocal_putValue_inmemory
		(zi->ci.central_header + 42,
		 static_cast<uLong>(zi->ci.pos_local_header)
		 -
		 zi->add_position_when_writting_offset,
		 4);

	for (i = 0; i < size_filename; ++i)
		*(zi->ci.central_header + SIZECENTRALHEADER + i) = *(filename + i);

	for (i = 0; i < size_extrafield_global; ++i)
		*(zi->ci.central_header + SIZECENTRALHEADER + size_filename + i) =
			*((static_cast<const char *>(extrafield_global) + i));

	for (i = 0; i < size_comment; ++i)
		*
		(zi->ci.central_header  +
		 SIZECENTRALHEADER      +
		 size_filename          +
		 size_extrafield_global +
		 i)
		=
		*(comment + i);
	if (not zi->ci.central_header)
		return ZIP_INTERNALERROR;

	//  write the local header
	err =
		ziplocal_putValue
			(&zi->z_filefunc,
			 zi->filestream,
			 static_cast<uLong>(LOCALHEADERMAGIC),
			 4);

	// version needed to extract
	if (err == ZIP_OK)
		err = ziplocal_putValue(&zi->z_filefunc, zi->filestream, 20UL, 2);

	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(zi->ci.flag),
				 2);

	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(zi->ci.method),
				 2);

	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(zi->ci.dosDate),
				 4);

	//  crc 32, unknown
	if (err == ZIP_OK)
		err = ziplocal_putValue
			(&zi->z_filefunc,
			 zi->filestream,
			 0UL,
			 4);

	//  compressed size, unknown
	if (err == ZIP_OK)
		err = ziplocal_putValue
			(&zi->z_filefunc,
			 zi->filestream,
			 0UL,
			 4);

	//  uncompressed size, unknown
	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 0UL,
				 4);

	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(size_filename),
				 2);

	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(size_extrafield_local),
				 2);

	if (err == ZIP_OK && size_filename > 0)
		if
			(ZWRITE(zi->z_filefunc, zi->filestream, filename, size_filename)
			 !=
			 size_filename)
			err = ZIP_ERRNO;

	if (err == ZIP_OK && size_extrafield_local > 0)
		if
			(ZWRITE
			 	(zi->z_filefunc,
			 	 zi->filestream,
			 	 extrafield_local,
			 	 size_extrafield_local)
			 !=
			 size_extrafield_local)
			err = ZIP_ERRNO;

	zi->ci.stream.avail_in  = static_cast<uInt>(0);
	zi->ci.stream.avail_out = static_cast<uInt>(Z_BUFSIZE);
	zi->ci.stream.next_out  = zi->ci.buffered_data;
	zi->ci.stream.total_in  = 0;
	zi->ci.stream.total_out = 0;

	if (err == ZIP_OK && zi->ci.method == Z_DEFLATED && !zi->ci.raw) {
		zi->ci.stream.zalloc = static_cast<alloc_func>(nullptr);
		zi->ci.stream.zfree  = static_cast<free_func> (nullptr);
		zi->ci.stream.opaque = static_cast<voidpf>    (nullptr);

		if (windowBits > 0)
			windowBits = -windowBits;

		err =
			deflateInit2
			(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

		if (err == Z_OK)
			zi->ci.stream_initialized = 1;
	}

	if (err == Z_OK)
		zi->in_opened_file_inzip = 1;
	return err;
}


local int32_t zipFlushWriteBuffer(zip_internal * const zi)
{
	int32_t err = ZIP_OK;

	if
		(ZWRITE
		 	(zi->z_filefunc,
		 	 zi->filestream,
		 	 zi->ci.buffered_data,
		 	 zi->ci.pos_in_buffered_data)
		 !=
		 zi->ci.pos_in_buffered_data)
		err = ZIP_ERRNO;
	zi->ci.pos_in_buffered_data = 0;
	return err;
}

extern int32_t ZEXPORT zipWriteInFileInZip
	(zipFile            file,
	 void const * const buf,
	 unsigned     const len)
{
	zip_internal * zi;
	int32_t err = ZIP_OK;

	if (not file)
		return ZIP_PARAMERROR;
	zi = static_cast<zip_internal *>(file);

	if (zi->in_opened_file_inzip == 0)
		return ZIP_PARAMERROR;

	zi->ci.stream.next_in =
		const_cast<Bytef *>(static_cast<const Bytef *>(buf));
	zi->ci.stream.avail_in = len;
	zi->ci.crc32 = crc32(zi->ci.crc32, static_cast<const Bytef *>(buf), len);

	while (err == ZIP_OK and zi->ci.stream.avail_in > 0) {
		if (zi->ci.stream.avail_out == 0) {
			if (zipFlushWriteBuffer(zi) == ZIP_ERRNO)
				err = ZIP_ERRNO;
			zi->ci.stream.avail_out = static_cast<uInt>(Z_BUFSIZE);
			zi->ci.stream.next_out  = zi->ci.buffered_data;
		}


		if (err != ZIP_OK)
			break;

		if (zi->ci.method == Z_DEFLATED and not zi->ci.raw) {
			uLong uTotalOutBefore = zi->ci.stream.total_out;
			err = deflate(&zi->ci.stream, Z_NO_FLUSH);
			zi->ci.pos_in_buffered_data +=
				static_cast<uInt>(zi->ci.stream.total_out - uTotalOutBefore);

		} else {
			uInt copy_this, i;
			if (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
				copy_this = zi->ci.stream.avail_in;
			else
				copy_this = zi->ci.stream.avail_out;
			for (i = 0; i < copy_this; ++i)
				*(reinterpret_cast<char *>(zi->ci.stream.next_out) + i) =
					*(reinterpret_cast<char const *>(zi->ci.stream.next_in) + i);
			{
				zi->ci.stream.avail_in      -= copy_this;
				zi->ci.stream.avail_out     -= copy_this;
				zi->ci.stream.next_in       += copy_this;
				zi->ci.stream.next_out      += copy_this;
				zi->ci.stream.total_in      += copy_this;
				zi->ci.stream.total_out     += copy_this;
				zi->ci.pos_in_buffered_data += copy_this;
			}
		}
	}

	return err;
}

extern int32_t ZEXPORT zipCloseFileInZipRaw
		(zipFile file, uLong uncompressed_size, uLong crc32)
{
	zip_internal * zi;
	uLong compressed_size;
	int32_t err = ZIP_OK;

	if (not file)
		return ZIP_PARAMERROR;
	zi = static_cast<zip_internal *>(file);

	if (zi->in_opened_file_inzip == 0)
		return ZIP_PARAMERROR;
	zi->ci.stream.avail_in = 0;

	if (zi->ci.method == Z_DEFLATED and not zi->ci.raw) while (err == ZIP_OK) {
		uLong uTotalOutBefore;
		if (zi->ci.stream.avail_out == 0) {
			if (zipFlushWriteBuffer(zi) == ZIP_ERRNO)
				err = ZIP_ERRNO;
			zi->ci.stream.avail_out = static_cast<uInt>(Z_BUFSIZE);
			zi->ci.stream.next_out  = zi->ci.buffered_data;
		}
		uTotalOutBefore = zi->ci.stream.total_out;
		err = deflate(&zi->ci.stream,  Z_FINISH);
		zi->ci.pos_in_buffered_data +=
			static_cast<uInt>(zi->ci.stream.total_out - uTotalOutBefore);
	}

	if (err == Z_STREAM_END)
		err = ZIP_OK; //  this is normal

	if (zi->ci.pos_in_buffered_data > 0 && err == ZIP_OK)
		if (zipFlushWriteBuffer(zi) == ZIP_ERRNO)
			err = ZIP_ERRNO;

	if ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw)) {
		err = deflateEnd(&zi->ci.stream);
		zi->ci.stream_initialized = 0;
	}

	if (!zi->ci.raw) {
		crc32 = static_cast<uLong>(zi->ci.crc32);
		uncompressed_size = static_cast<uLong>(zi->ci.stream.total_in);
	}
	compressed_size = static_cast<uLong>(zi->ci.stream.total_out);

	ziplocal_putValue_inmemory(zi->ci.central_header + 16, crc32, 4); //  crc
	ziplocal_putValue_inmemory(zi->ci.central_header + 20, compressed_size, 4);
	if (zi->ci.stream.data_type == Z_ASCII)
		ziplocal_putValue_inmemory
			(zi->ci.central_header + 36, static_cast<uLong>(Z_ASCII), 2);
	ziplocal_putValue_inmemory
		(zi->ci.central_header + 24, uncompressed_size, 4); //  uncompr size

	if (err == ZIP_OK)
		err =
			add_data_in_datablock
				(&zi->central_dir,
				 zi->ci.central_header,
				 static_cast<uLong>(zi->ci.size_centralheader));
	free(zi->ci.central_header);

	if (err == ZIP_OK) {
		long cur_pos_inzip = ZTELL(zi->z_filefunc, zi->filestream);
		if
			(ZSEEK
			 	(zi->z_filefunc,
			 	 zi->filestream,
			 	 zi->ci.pos_local_header + 14,
			 	 ZLIB_FILEFUNC_SEEK_SET))
			err = ZIP_ERRNO;

		if (err == ZIP_OK) //  crc 32, unknown
			err = ziplocal_putValue(&zi->z_filefunc, zi->filestream, crc32, 4);

		if (err == ZIP_OK) /* compressed size, unknown */
			err =
				ziplocal_putValue
					(&zi->z_filefunc, zi->filestream,   compressed_size, 4);

		if (err == ZIP_OK) /* uncompressed size, unknown */
			err =
				ziplocal_putValue
					(&zi->z_filefunc, zi->filestream, uncompressed_size, 4);

		if
			(ZSEEK
			 	(zi->z_filefunc,
			 	 zi->filestream,
			 	 cur_pos_inzip,
			 	 ZLIB_FILEFUNC_SEEK_SET))
			err = ZIP_ERRNO;
	}

	++zi->number_entry;
	zi->in_opened_file_inzip = 0;

	return err;
}

extern int32_t ZEXPORT zipCloseFileInZip (zipFile file)
{
	return zipCloseFileInZipRaw (file, 0, 0);
}

extern int32_t ZEXPORT zipClose
	(zipFile file, char const * const global_comment)
{
	zip_internal * zi;
	int32_t err = 0;
	uLong size_centraldir = 0;
	uLong centraldir_pos_inzip;
	uInt size_global_comment;
	if (not file)
		return ZIP_PARAMERROR;
	zi = static_cast<zip_internal *>(file);

	if (zi->in_opened_file_inzip == 1)
		err = zipCloseFileInZip (file);

	size_global_comment =
		global_comment ? static_cast<uInt>(strlen(global_comment)) : 0;


	centraldir_pos_inzip = ZTELL(zi->z_filefunc, zi->filestream);
	if (err == ZIP_OK) {
		linkedlist_datablock_internal * ldi = zi->central_dir.first_block;
		while (ldi) {
			if (err == ZIP_OK and ldi->filled_in_this_block > 0)
				if
					(ZWRITE
					 	(zi->z_filefunc, zi->filestream,
					 	 ldi->data, ldi->filled_in_this_block)
					 !=
					 ldi->filled_in_this_block)
					err = ZIP_ERRNO;

			size_centraldir += ldi->filled_in_this_block;
			ldi = ldi->next_datablock;
		}
	}
	free_datablock(zi->central_dir.first_block);

	//  Magic End
	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(ENDHEADERMAGIC),
				 4);

	//  number of this disk
	if (err == ZIP_OK)
		err = ziplocal_putValue(&zi->z_filefunc, zi->filestream, 0UL, 2);

	//  number of the disk with the start of the central directory
	if (err == ZIP_OK)
		err = ziplocal_putValue(&zi->z_filefunc, zi->filestream, 0UL, 2);

	//  total number of entries in the central dir on this disk
	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(zi->number_entry),
				 2);

	//  total number of entries in the central dir
	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(zi->number_entry),
				 2);

	//  size of the central directory
	if (err == ZIP_OK)
		err = ziplocal_putValue
			(&zi->z_filefunc,
			 zi->filestream,
			 static_cast<uLong>(size_centraldir),
			 4);

	//  offset of start of central directory with respect to the starting disk
	//  number
	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>
				 (centraldir_pos_inzip - zi->add_position_when_writting_offset),
				 4);

	//  zipfile comment length
	if (err == ZIP_OK)
		err =
			ziplocal_putValue
				(&zi->z_filefunc,
				 zi->filestream,
				 static_cast<uLong>(size_global_comment),
				 2);

	if
		(err == ZIP_OK
		 &&
		 size_global_comment > 0
		 and
		 ZWRITE
		 	(zi->z_filefunc, zi->filestream, global_comment, size_global_comment)
		 !=
		 size_global_comment)
		err = ZIP_ERRNO;

	if (ZCLOSE(zi->z_filefunc, zi->filestream) != 0 and err == ZIP_OK)
		err = ZIP_ERRNO;

	free(zi);

	return err;
}

/* ioapi.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API

   Version 1.01, May 8th, 2004

   Copyright (C) 1998-2004 Gilles Vollant
*/

#include "zlib.h"
#include "ioapi.h"



/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

voidpf ZCALLBACK fopen_file_func   OF
	((voidpf opaque, const char * filename, int32_t mode));

uLong ZCALLBACK fread_file_func    OF
	((voidpf opaque, voidpf stream, void * buf, uLong size));

uLong ZCALLBACK fwrite_file_func   OF
	((voidpf opaque, voidpf stream, const void * buf, uLong size));

long ZCALLBACK ftell_file_func     OF
	((voidpf opaque, voidpf stream));

long ZCALLBACK fseek_file_func     OF
	((voidpf opaque, voidpf stream, uLong offset, int32_t origin));

int32_t ZCALLBACK fclose_file_func OF
	((voidpf opaque, voidpf stream));

int32_t ZCALLBACK ferror_file_func OF
	((voidpf opaque, voidpf stream));


voidpf ZCALLBACK fopen_file_func
	(voidpf,
	 char const * const filename,
	 int32_t      const mode)
{
	FILE * file = nullptr;
	char const * const mode_fopen =
		(mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ
		?                                    "rb"  :
		mode & ZLIB_FILEFUNC_MODE_EXISTING ? "r+b" :
		mode & ZLIB_FILEFUNC_MODE_CREATE   ? "wb"  : nullptr;

	if (filename && mode_fopen)
		file = fopen(filename, mode_fopen);
	return file;
}


uLong ZCALLBACK fread_file_func
	(voidpf, voidpf stream, void * const buf, uLong const size)
{
	return
		fread(buf, 1, static_cast<size_t>(size), static_cast<FILE *>(stream));
}


uLong ZCALLBACK fwrite_file_func
	(voidpf, voidpf stream, void const * const buf, uLong const size)
{
	return
		fwrite(buf, 1, static_cast<size_t>(size), static_cast<FILE *>(stream));
}

long ZCALLBACK ftell_file_func (voidpf, voidpf stream) {
	return ftell(static_cast<FILE *>(stream));
}

long ZCALLBACK fseek_file_func
		(voidpf, voidpf stream, uLong offset, int32_t origin)
{
	int32_t fseek_origin = 0;
	switch (origin) {
	case ZLIB_FILEFUNC_SEEK_CUR :
		fseek_origin = SEEK_CUR;
		break;
	case ZLIB_FILEFUNC_SEEK_END :
		fseek_origin = SEEK_END;
		break;
	case ZLIB_FILEFUNC_SEEK_SET :
		fseek_origin = SEEK_SET;
		break;
	default:
		return -1;
	}
	fseek(static_cast<FILE *>(stream), offset, fseek_origin);
	return 0;
}

int32_t ZCALLBACK fclose_file_func (voidpf, voidpf stream) {
	return fclose(static_cast<FILE *>(stream));
}

int32_t ZCALLBACK ferror_file_func (voidpf, voidpf stream) {
	return ferror(static_cast<FILE *>(stream));
}

void fill_fopen_filefunc (zlib_filefunc_def * const pzlib_filefunc_def)
{
	pzlib_filefunc_def->zopen_file  = fopen_file_func;
	pzlib_filefunc_def->zread_file  = fread_file_func;
	pzlib_filefunc_def->zwrite_file = fwrite_file_func;
	pzlib_filefunc_def->ztell_file  = ftell_file_func;
	pzlib_filefunc_def->zseek_file  = fseek_file_func;
	pzlib_filefunc_def->zclose_file = fclose_file_func;
	pzlib_filefunc_def->zerror_file = ferror_file_func;
	pzlib_filefunc_def->opaque      = nullptr;
}
