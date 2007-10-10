	/*
 * Copyright (C) 2002-2006 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "zip_filesystem.h"

#include "filesystem_exceptions.h"
#include "zip_exceptions.h"
#include "wexception.h"

#include <cassert>
#include <string>


/**
 * Initialize the real file-system
 */
ZipFilesystem::ZipFilesystem(const std::string zipfile)
{
   m_basename = FS_Filename(zipfile.c_str());
   m_zipfilename = zipfile;
   m_zipfile = m_unzipfile = 0;

   m_state = STATE_IDLE;

   // TODO: check OS permissions on whether the file is writable
}

/**
 * Cleanup code
 */
ZipFilesystem::~ZipFilesystem()
{
   m_Close();
}

/**
 * Return true if this directory is writable.
 */
const bool ZipFilesystem::IsWritable() const
{
	return true; // should be checked in constructor
}

/**
 * Returns the number of files found, and stores the filenames (without the
 * pathname) in the results. There doesn't seem to be an even remotely
 * cross-platform way of doing this
 */
const int32_t ZipFilesystem::FindFiles
(std::string path,
 const std::string
#ifndef NDEBUG
 pattern
#endif
,
 filenameset_t * results, uint32_t)
{
   m_OpenUnzip();

   assert(pattern == "*"); // If you need something else, implement a proper glob() here. I do not want to! -- Holger

   if (path[path.size()-1] != '/')
      path += '/';
   if (path[0] != '/')
      path = '/'+path;

   unzCloseCurrentFile(m_unzipfile);
   unzGoToFirstFile(m_unzipfile);

   unz_file_info file_info;
   char filename_inzip[256];
   while (1) {
      unzGetCurrentFileInfo(m_unzipfile, &file_info, filename_inzip,
                            sizeof(filename_inzip), NULL, 0, NULL, 0);

      std::string complete_filename = &filename_inzip[ m_basename.size() ];
      std::string filename = FS_Filename(complete_filename.c_str());
      std::string filepath = complete_filename.substr(0,
                                                       complete_filename.size()-
                                                       filename.size());

      //TODO: sth. strange is going on wrt the leading slash! This is just an ugly
      //workaround and does not solve the real problem (which remains undiscovered)
      if ((('/'+path==filepath) || (path==filepath))&& (filename != "")) {
         results->insert(complete_filename);
		}

      if (unzGoToNextFile(m_unzipfile) == UNZ_END_OF_LIST_OF_FILE)
         break;
	}

   return results->size();
}

/**
 * Returns true if the given file exists, and false if it doesn't.
 * Also returns false if the pathname is invalid
 */
const bool ZipFilesystem::FileExists(std::string path)
{
   try {
      m_OpenUnzip();
      //TODO: check return code
	} catch (...) {
      return false;
	}

   unzGoToFirstFile(m_unzipfile);
   unz_file_info file_info;
   char filename_inzip[256];
   memset(filename_inzip, ' ', 256);

   if (path[0] != '/')
      path = '/' + path;

   while (1) {
      unzGetCurrentFileInfo(m_unzipfile, &file_info, filename_inzip,
                            sizeof(filename_inzip), NULL, 0, NULL, 0);

      std::string complete_filename = &filename_inzip[ m_basename.size() ];
      if (complete_filename[ complete_filename.size() - 1 ] == '/')
         complete_filename.resize(complete_filename.size() - 1);

      if (path == complete_filename) {
         return true;
		}

      if (unzGoToNextFile(m_unzipfile) == UNZ_END_OF_LIST_OF_FILE)
         break;
	}

   return false;
}

/**
 * Returns true if the given file is a directory, and false if it doesn't.
 * Also returns false if the pathname is invalid
 */
const bool ZipFilesystem::IsDirectory(const std::string path)
{

   if (!FileExists(path)) {
      return false;
	}

   unz_file_info file_info;
   char filename_inzip[256];

   unzGetCurrentFileInfo(m_unzipfile, &file_info, filename_inzip,
								 sizeof(filename_inzip), NULL, 0, NULL, 0);

   if (filename_inzip[ strlen(filename_inzip) - 1 ] == '/') {
      return true;
	}

   return false;
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem* ZipFilesystem::MakeSubFileSystem(std::string path)
{
   m_OpenUnzip();

   assert(FileExists(path));
   assert(IsDirectory(path));

   m_Close();

   ZipFilesystem* newfs = new ZipFilesystem(m_zipfilename);
   newfs->m_basename = m_basename + '/' + path;

   return newfs;
}

/**
 * Make a new Subfilesystem in this
 * \todo type should be recognized automatically, \see Filesystem::Create
 * \throw ZipOperation_error
 */
FileSystem* ZipFilesystem::CreateSubFileSystem(std::string path, Type type)
{
   assert(!FileExists(path));

	if (type != FileSystem::DIR)
		throw ZipOperation_error
			("ZipFilesystem::CreateSubFileSystem",
			 path, m_zipfilename,
			 "can't create ZipFilesystem inside another ZipFilesystem");

   EnsureDirectoryExists(path);

   m_Close();

   ZipFilesystem* newfs = new ZipFilesystem(*this);
   newfs->m_basename = m_basename + '/' + path;

   return newfs;
}
/**
 * Remove a number of files
 * \throw ZipOperation_error
 */
void ZipFilesystem::Unlink(const std::string filename)
{
	throw ZipOperation_error
		("ZipFilesystem::Unlink",
		 filename,
		 m_zipfilename,
		 "unlinking is not supported inside zipfiles");
}

/**
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void ZipFilesystem::EnsureDirectoryExists(const std::string dirname)
{
   if (FileExists(dirname) && IsDirectory(dirname))
      return;

   MakeDirectory(dirname);
}

/**
 * Create this directory, throw an error if it already exists or
 * if a file is in the way or if the creation fails.
 *
 * Pleas note, this function does not honor parents,
 * MakeDirectory("onedir/otherdir/onemoredir") will fail
 * if either ondir or otherdir is missing
 */
void ZipFilesystem::MakeDirectory(std::string dirname) {
   m_OpenZip();

   zip_fileinfo zi;

   zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
      zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
   zi.dosDate = 0;
   zi.internal_fa = 0;
   zi.external_fa = 0;

   if (dirname[ dirname.size() - 1] != '/')
      dirname += '/';

   std::string complete_file = m_basename + '/' + dirname;
#ifndef NDEBUG
	int32_t err =
#endif
		zipOpenNewFileInZip3
		(m_zipfile,
		 complete_file.c_str(),
		 &zi,
		 NULL, 0, NULL, 0, NULL /* comment*/,
		 Z_DEFLATED,
		 Z_BEST_COMPRESSION,
		 0,
		 -MAX_WBITS,
		 DEF_MEM_LEVEL,
		 Z_DEFAULT_STRATEGY,
		 0,
		 0);
   assert(err == ZIP_OK);

   // CloseFile
   zipCloseFileInZip(m_zipfile);
}

/**
 * Read the given file into alloced memory; called by FileRead::Open.
 * \throw FileNotFound_error if the file couldn't be opened.
 */
void * ZipFilesystem::Load(const std::string & fname, size_t & length) {
   if (!FileExists(fname.c_str()) || IsDirectory(fname.c_str()))
		throw ZipOperation_error
			("ZipFilesystem::Load",
			 fname,
			 m_zipfilename,
			 "couldn't open file from zipfile");

   char buffer[1024];
	size_t totallen = 0;
   unzOpenCurrentFile(m_unzipfile);
	while
		(const size_t len =
		  unzReadCurrentFile(m_unzipfile, buffer, sizeof(buffer)))
		totallen += len;
   unzCloseCurrentFile(m_unzipfile);

   void* retdata = malloc(totallen);
   unzOpenCurrentFile(m_unzipfile);
   unzReadCurrentFile(m_unzipfile, retdata, totallen);
   unzCloseCurrentFile(m_unzipfile);

   length = totallen;

   return retdata;
}

/**
 * Write the given block of memory to the repository.
 * Throws an exception if it fails.
 */
void ZipFilesystem::Write
(const std::string fname, const void * const data, const int32_t length)
{
   m_OpenZip();

   zip_fileinfo zi;

   zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
      zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
   zi.dosDate = 0;
   zi.internal_fa = 0;
   zi.external_fa = 0;

   // Create File
   std::string complete_file = m_basename + '/' + fname;
   int32_t err = zipOpenNewFileInZip3(m_zipfile, complete_file.c_str(), &zi,
         NULL, 0, NULL, 0, NULL /* comment*/,
         Z_DEFLATED,
         Z_BEST_COMPRESSION, 0,
         -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
         0, 0);
   assert(err == ZIP_OK);

   // Write File
   err = zipWriteInFileInZip (m_zipfile, data, length);
   assert(err == ZIP_OK) ;

   // CloseFile
   zipCloseFileInZip(m_zipfile);
}

StreamRead * ZipFilesystem::OpenStreamRead
(const std::string & fname)
{
	throw wexception("OpenStreamRead(%s) not yet supported in ZipFilesystem", fname.c_str());
}

StreamWrite * ZipFilesystem::OpenStreamWrite
(const std::string & fname)
{
	throw wexception("OpenStreamWrite(%s) not yet supported in ZipFilesystem", fname.c_str());
}


/**
 * Private Functions below
 */
void ZipFilesystem::m_Close() {
   if (m_state == STATE_ZIPPING) {
      zipClose(m_zipfile, 0);
	} else if (m_state == STATE_UNZIPPPING) {
      unzClose(m_unzipfile);
	}

   m_state = STATE_IDLE;
}

/**
 * Open a zipfile for compressing
 */
void ZipFilesystem::m_OpenZip() {
   if (m_state == STATE_ZIPPING)
      return;

   m_Close();

   m_zipfile = zipOpen(m_zipfilename.c_str(), APPEND_STATUS_ADDINZIP);
   if (!m_zipfile) {
      // Couldn't open for append, so create new
      m_zipfile = zipOpen(m_zipfilename.c_str(), APPEND_STATUS_CREATE);
	}

   m_state = STATE_ZIPPING;
}

/**
 * Open a zipfile for extraction
 * \throw FileType_error
 */
void ZipFilesystem::m_OpenUnzip() {
   if (m_state == STATE_UNZIPPPING)
      return;

   m_Close();

   m_unzipfile = unzOpen(m_zipfilename.c_str());
   if (!m_unzipfile)
      throw FileType_error("ZipFilesystem::m_OpenUnzip", m_zipfilename,
								   "not a .zip file");

   m_state = STATE_UNZIPPPING;
}

void ZipFilesystem::Rename
(const std::string &, const std::string &)
{
	assert (false and "rename inside zip FS is not implemented yet");
}
