/*
 * Copyright (C) 2002-5 by the Widelands Development Team
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

#ifndef __S__ZIP_FILESYSTEM_H
#define __S__ZIP_FILESYSTEM_H

#include <string>
#include "filesystem.h"
#include "unzip.h"
#include "zip.h"

class ZipFilesystem : public FileSystem {
public:
   ZipFilesystem( std::string );
	virtual ~ZipFilesystem();

	virtual bool IsWritable();

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results) ;

   virtual bool IsDirectory(std::string path) ;
	virtual bool FileExists(std::string path) ;

	virtual void *Load(std::string fname, int *length) ;
	virtual void Write(std::string fname, void *data, int length) ;
   virtual void EnsureDirectoryExists(std::string dirname) ;
   virtual void MakeDirectory(std::string dirname) ;

   virtual FileSystem*  MakeSubFileSystem( std::string dirname );
   virtual FileSystem*  CreateSubFileSystem( std::string dirname, Type );
   virtual void Unlink( std::string ) ;

public:
	static FileSystem *CreateFromDirectory(std::string directory);

	void listSubdirs() {printf("%s\n", m_basename.c_str());}

private:
   void m_OpenUnzip( void );
   void m_OpenZip( void );
   void m_Close( void );

private:
   enum State {
      STATE_IDLE,
      STATE_ZIPPING,
      STATE_UNZIPPPING
   };

   State       m_state;
   zipFile     m_zipfile;
   unzFile     m_unzipfile;
   std::string m_zipfilename;
   std::string m_basename;

};

#endif // __S__ZIP_FILESYSTEM_H
