/*
 * Copyright (C) 2002 by the Widelands Development Team
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
// this originally comes from Return to the Shadows (http://www.rtts.org/)
// files.cc: provides all the OS abstraction to access files

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include "errno.h"
#include "error.h"
#include "filesystem.h"
#include "zip_filesystem.h"

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #define stat _stat
#else
  #include <glob.h>
#endif

/*
==============
FS_AutoExtension

Append extension (e.g. ".foo") to the filename if it doesn't already have an extension
==============
*/
char *FS_AutoExtension(char *buf, int bufsize, const char *ext)
{
	char *dot;
	char *p;
	int extlen;

	dot = 0;

	for(p = buf; *p; p++) {
		if (*p == '/' || *p == '\\')
			dot = 0;
		else if (*p == '.')
			dot = p;
	}

	if (!dot) {
		extlen = strlen(ext);

		if (p - buf + extlen < bufsize)
			memcpy(p, ext, extlen + 1);
	}

	return buf;
}

/*
==============
FS_StripExtension

Strip the extension (if any) from the filename
==============
*/
char *FS_StripExtension(char *fname)
{
	char *p;
	char *dot = 0;

	for(p = fname; *p; p++) {
		if (*p == '/' || *p == '\\')
			dot = 0;
		else if (*p == '.')
			dot = p;
	}

	if (dot)
		*dot = 0;

	return fname;
}

/*
==============
FS_RelativePath

Translate filename so that it is relative to basefile.
Basically concatenates the two strings, but removes the filename part
of basefile (if any)
==============
*/
char *FS_RelativePath(char *buf, int buflen, const char *basefile, const char *filename)
{
	const char *p;
	int endbase;

	if (*filename == '/' || *filename == '\\') { // it's an absolute filename
		snprintf(buf, buflen, "%s", filename);
		return buf;
	}

	// find the end of the basefile name
	endbase = 0;
	for(p = basefile; *p; p++) {
		if (*p == '/' || *p == '\\')
			endbase = p-basefile+1;
	}

	if (!endbase) { // no path in basefile
		snprintf(buf, buflen, "%s", filename);
		return buf;
	}

	// copy the base path
	snprintf(buf, buflen, "%s%s", basefile, filename);

	return buf;
}

/*
==============
FS_CanonicalizeName

Turn the given path into a simpler one. Returns false for illegal paths.
==============
*/
bool FS_CanonicalizeName(char *buf, int bufsize, const char *path)
{
	const char *tok;
	int toklen;
	char *p;
   bool skip_token;
   
   // clear the dst buffer
   memset(buf, 0, bufsize);

	p = buf;
	while(*path) {
		path += strspn(path, "/\\");
		if (!*path)
			return false; // can't open directories

		// scan for the slash
		tok = path;
		toklen = strcspn(path, "/\\");
		path += toklen;


      skip_token=false;
      
		// check for '.', '..', ...
		if (toklen == 1 && !strncmp(tok, ".", 1))
			continue;
		if (toklen == 2 && !strncmp(tok, "..", 2)) {
         p--;
			while(p > buf) {
				if (*(p-1) == '/')
					break;
				p--;
			}
			if (p < buf)
				return false; // too many '..'s
         skip_token=true;
      }

		// use the token
		if (p + toklen >= buf + bufsize)
			return false; // path is too long

      if(!skip_token) {
         memcpy(p, tok, toklen);
         p += toklen;
      }
		if (*path)
			*p++ = '/';
	}
   *p = 0;
	if(*(p-1)=='/' || *(p-1)=='\\')
      *(p-1)=0;

	return true;

   
}

/*
 * Returns the filename of this path, everything after the last
 * / or \  (or the whole string)
 */
const char *FS_Filename(const char* buf) {
   int i=strlen(buf)-1;
   while(i>=0) {
      if(buf[i]=='/' || buf[i]=='\\') return &buf[i+1];
      --i;
   }
   return buf;
}


/*
==============================================================================

FileRead IMPLEMENTATION

==============================================================================
*/

/*
==============
FileRead::FileRead

Create the object with nothing to read
==============
*/
FileRead::FileRead()
{
	data = 0;
}

/*
==============
FileRead::~FileRead

Close the file if open
==============
*/
FileRead::~FileRead()
{
	if (data)
		Close();
}

/*
==============
FileRead::Open

Loads a file into memory.
Reserves one additional byte which is zeroed, so that text files can
be handled like a normal C string.
Throws an exception if the file couldn't be loaded for whatever reason.
==============
*/
void FileRead::Open(FileSystem *fs, std::string fname)
{
	assert(!data);

	data = fs->Load(fname, &length);
	filepos = 0;
}

/*
==============
FileRead::TryOpen

Works just like Open, but returns false when the load fails.
==============
*/
bool FileRead::TryOpen(FileSystem *fs, std::string fname)
{
	assert(!data);

	try {
		data = fs->Load(fname, &length);
		filepos = 0;
	} catch(std::exception &e) {
		//log("%s\n", e.what());
		return false;
	}

	return true;
}

/*
==============
FileRead::Close

Frees allocated memory
==============
*/
void FileRead::Close()
{
	assert(data);

	free(data);
	data = 0;
}

/*
==============
FileRead::SetFilePos

Set the file pointer to the given location.
Raises an exception when the pointer is out of bound
==============
*/
void FileRead::SetFilePos(int pos)
{
	assert(data);

	if (pos < 0 || pos >= length)
		throw wexception("SetFilePos: %i out of bound", pos);

	filepos = pos;
}

/*
==============
FileRead::CString

Read a zero-terminated string from the file
==============
*/
char *FileRead::CString(int pos)
{
	char *string, *p;
	int i;

	assert(data);

	i = pos;
	if (pos < 0)
		i = filepos;
	if (i >= length)
		throw wexception("File boundary exceeded");

	string = (char *)data + i;
	for(p = string; *p; p++, i++) ;
	i++; // beyond the NUL

	if (i > length)
		throw wexception("File boundary exceeded");

	if (pos < 0)
		filepos = i;

	return string;
}

/** FileRead::ReadLine(char *buf, int buflen)
 *
 * Reads a line from the file into the buffer.
 * The '\\r', '\\n' are consumed, but not stored in buf
 *
 * Returns true on EOF condition.
 */
bool FileRead::ReadLine(char *buf, int buflen)
{
	char *dst = buf;

	assert(data);

	if (filepos >= length)
		return false;

	while(filepos < length && buflen > 0) {
		char c = ((char *)data)[filepos];
		filepos++;

		if (c == '\r') // not perfectly correct, but it should work
			continue;
		if (c == '\n') {
			*dst++ = 0;
			buflen--;
			break;
		}

		*dst++ = c;
		buflen--;
	}

	if (!buflen && *(dst-1)) {
		*(dst-1) = 0;
		throw wexception("ReadLine: buffer overflow");
	}

	return true;
}

/*
==============================================================================

FileWrite IMPLEMENTATION

==============================================================================
*/

/*
==============
FileWrite::FileWrite

Set the buffer to empty
==============
*/
FileWrite::FileWrite()
{
	data = 0;
	length = 0;
	maxsize = 0;
	filepos = 0;
   counter = 0;
}

/*
==============
FileWrite::~FileWrite

Clear any remaining allocated data
==============
*/
FileWrite::~FileWrite()
{
	if (data)
		Clear();
}

/*
==============
FileWrite::Clear

Clears the object's buffer
==============
*/
void FileWrite::Clear()
{
	if (data)
		free(data);

	data = 0;
	length = 0;
	maxsize = 0;
	filepos = 0;
   counter = 0;
}

/*
==============
FileWrite::ResetByteCounter

Reset the byte counter to zero.
All bytes written then are added
to the byte counter
==============
*/
void FileWrite::ResetByteCounter(void) {
   counter = 0;
}

/*
==============
FileWrite::GetByteCounter

Returns the number of bytes written since
last ResetByteCounter
==============
*/
int FileWrite::GetByteCounter(void) {
   return counter;
}

/*
==============
FileWrite::Write

Actually write the file out to disk.
If successful, this clears the buffers. Otherwise, an exception
is raised but the buffer remains intact (don't worry, it will be
cleared by the destructor).
==============
*/
void FileWrite::Write(FileSystem *fs, std::string filename)
{
	fs->Write(filename, data, length);

	Clear();
}

/*
==============
FileWrite::TryWrite

Same as Write, but returns falls if the write fails
==============
*/
bool FileWrite::TryWrite(FileSystem *fs, std::string filename)
{
	try {
		fs->Write(filename, data, length);
	} catch(std::exception &e) {
		log("%s\n", e.what());
		return false;
	}

	Clear();
	return true;
}

/*
==============
FileWrite::SetFilePos

Set the file pointer to a new location. The position can be beyond
the current end of file.
==============
*/
void FileWrite::SetFilePos(int pos)
{
	assert(pos >= 0);
	filepos = pos;
}

/*
==============
FileWrite::GetFilePos

Set the file pointer to a new location. The position can be beyond
the current end of file.
==============
*/
int FileWrite::GetFilePos(void)
{
	return filepos;
}


/*
==============
FileWrite::Data

Write data at the given location. If pos is -1, write at the
file pointer and advance the file pointer.
==============
*/
void FileWrite::Data(const void *buf, int size, int pos)
{
	int i;

	assert(data || !length);

	i = pos;
	if (pos < 0) {
		i = filepos;
		filepos += size;
	}

	if (i+size > length) {
		if (i+size > maxsize) {
			maxsize += 4096;
			if (i+size > maxsize)
				maxsize = i+size;

			data = realloc(data, maxsize);
		}

		length = i+size;
	}

   counter += size; 

	memcpy((char*)data + i, buf, size);
}

/*
==============
FileWrite::Printf

This is a perfectly normal printf (actually it isn't because it's limited
to a maximum string size)
==============
*/
void FileWrite::Printf(const char *fmt, ...)
{
	char buf[2048];
	va_list va;
	int i;

	va_start(va, fmt);
	i = vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	if (i < 0)
		throw wexception("FileWrite::Printf: buffer exceeded");

	Data(buf, i, -1);
}


/*
==============================================================================

RealFSImpl IMPLEMENTATION

==============================================================================
*/

class RealFSImpl : public FileSystem {
private:
	std::string m_directory;

private:
   void m_unlink_directory( std::string file );
   void m_unlink_file( std::string file );

public:
	RealFSImpl(std::string sDirectory);
	~RealFSImpl();

	virtual bool IsWritable();

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results);

	virtual bool FileExists(std::string path);
   virtual bool IsDirectory(std::string path);
   virtual void EnsureDirectoryExists(std::string dirname);
   virtual void MakeDirectory(std::string dirname);

	virtual void *Load(std::string fname, int *length);
	virtual void Write(std::string fname, void *data, int length);
   
   virtual FileSystem* MakeSubFileSystem( std::string dirname );
   virtual FileSystem* CreateSubFileSystem( std::string dirname, Type );
   virtual void Unlink(std::string file);
};

/** RealFSImpl::RealFSImpl(const char *pszDirectory)
 *
 * Initialize the real file-system
 */
RealFSImpl::RealFSImpl(std::string sDirectory)
	: m_directory(sDirectory)
{
	// TODO: check OS permissions on whether the directory is writable!
}

/** RealFSImpl::~RealFSImpl()
 *
 * Cleanup code
 */
RealFSImpl::~RealFSImpl()
{
}

/** RealFSImpl::IsWritable()
 *
 * Return true if this directory is writable.
 */
bool RealFSImpl::IsWritable()
{
	return true; // should be checked in constructor
}

/*
===============
RealFSImpl::FindFiles

Returns the number of files found, and stores the filenames (without the pathname) in the results.
There doesn't seem to be an even remotely cross-platform way of
doing this
===============
*/
#ifdef _WIN32
// note: the Win32 version may be broken, feel free to fix it
int RealFSImpl::FindFiles(std::string path, std::string pattern, filenameset_t *results)
{
	std::string buf;
	struct _finddata_t c_file;
	long hFile;
	int count;

	if (path.size())
		buf = m_directory + '\\' + path + '\\' + pattern;
	else
		buf = m_directory + '\\' + pattern;

	count = 0;

	hFile = _findfirst(buf.c_str(), &c_file);
	if (hFile == -1)
		return 0;

	do {
		results->insert(std::string(path)+'/'+c_file.name);
	} while(_findnext(hFile, &c_file) == 0);

	_findclose(hFile);

	return results->size();
}
#else
int RealFSImpl::FindFiles(std::string path, std::string pattern, filenameset_t *results)
{
	std::string buf;
	glob_t gl;
	int i, count;
	int ofs;

	if (path.size()) {
		buf = m_directory + '/' + path + '/' + pattern;
		ofs = m_directory.length()+1;
	} else {
		buf = m_directory + '/' + pattern;
		ofs = m_directory.length()+1;
	}

	if (glob(buf.c_str(), 0, NULL, &gl))
		return 0;

	count = gl.gl_pathc;

	for(i = 0; i < count; i++) {
		results->insert(&gl.gl_pathv[i][ofs]);
	}

	globfree(&gl);

	return count;
}
#endif

/*
===============
RealFSImpl::FileExists

Returns true if the given file exists, and false if it doesn't.
Also returns false if the pathname is invalid
===============
*/
bool RealFSImpl::FileExists(std::string path)
{
	char canonical[256]; // erm...
	std::string fullname;
	struct stat st;

   if (!FS_CanonicalizeName(canonical, sizeof(canonical), path.c_str()))
		return false;

	fullname = m_directory + '/' + canonical;

	if (stat(fullname.c_str(), &st) == -1)
		return false;

	return true;
}

/*
===============
RealFSImpl::IsDirectory

Returns true if the given file is a directory, and false if it doesn't.
Also returns false if the pathname is invalid
===============
*/
bool RealFSImpl::IsDirectory(std::string path)
{
   if(!FileExists(path)) return false;

   char canonical[256];
	std::string fullname;
	struct stat st;

	if (!FS_CanonicalizeName(canonical, sizeof(canonical), path.c_str()))
		return false;

	fullname = m_directory + '/' + canonical;

	if (stat(fullname.c_str(), &st) == -1)
		return false;

	return S_ISDIR(st.st_mode);
}

/*
 * Create a sub filesystem out of this filesystem
 */
FileSystem* RealFSImpl::MakeSubFileSystem( std::string path ) {
   assert( FileExists( path ));
   char canonical[256];
   std::string fullname;

   if (!FS_CanonicalizeName(canonical, sizeof(canonical), path.c_str()))
      return false;

   fullname = m_directory + '/' + canonical;

   if( IsDirectory( path )) {
      return new RealFSImpl( fullname ); 
   } else {
      FileSystem* s =  new ZipFilesystem( fullname );
      return s;
   }
}

/*
 * Create a sub filesystem out of this filesystem
 */
FileSystem* RealFSImpl::CreateSubFileSystem( std::string path, Type fs ) {
   if( FileExists( path )) 
      throw wexception( "Path %s already exists. Can't create a filesystem from it!\n", path.c_str()); 
   
   char canonical[256];
   std::string fullname;

   if (!FS_CanonicalizeName(canonical, sizeof(canonical), path.c_str()))
      return false;

   fullname = m_directory + '/' + canonical;

   if( fs == FileSystem::FS_DIR ) {
      EnsureDirectoryExists( path );
      return new RealFSImpl( fullname ); 
   } else {
      FileSystem* s =  new ZipFilesystem( fullname );
      return s;
   }
}
    
/*
 * Remove a number of files
 */
void RealFSImpl::Unlink(std::string file) {
   if( !FileExists( file ))
      return;

   if(IsDirectory( file ))
      m_unlink_directory( file );
   else
      m_unlink_file( file );
}

/*
 * remove directory or file
 */
void RealFSImpl::m_unlink_file( std::string file ) {
   assert( FileExists( file ));
   assert(!IsDirectory( file ));
   
   char canonical[256];
	std::string fullname;

	if (!FS_CanonicalizeName(canonical, sizeof(canonical), file.c_str()))
		return;

	fullname = m_directory + '/' + canonical;

#ifndef __WIN32__
   unlink( fullname.c_str());
#else
   DeleteFile( fullname.c_str() );
#endif
}

void RealFSImpl::m_unlink_directory( std::string file ) {
   assert( FileExists( file ));
   assert( IsDirectory( file ));

   filenameset_t files;
   
   FindFiles( file, "*", &files );
   
   for(filenameset_t::iterator pname = files.begin(); pname != files.end(); pname++) {
	   std::string filename = FS_Filename( (*pname).c_str());
	if( filename == "CVS" ) // HACK: ignore CVS directory for this might be a campaign directory or similar
         continue;
	if( filename == ".." )
	 continue;
	if( filename == "." )
	 continue;

      if( IsDirectory( *pname ) )
         m_unlink_directory( *pname );
      else
         m_unlink_file( *pname );
   }

   // NOTE: this might fail if this directory contains CVS dir,
   // so no error checking here
   char canonical[256];
	std::string fullname;

	if (!FS_CanonicalizeName(canonical, sizeof(canonical), file.c_str()))
		return;

	fullname = m_directory + '/' + canonical;
#ifndef __WIN32__
   rmdir( fullname.c_str() );
#else
   RemoveDirectory( fullname.c_str());
#endif
}

/*
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void RealFSImpl::EnsureDirectoryExists(std::string dirname) {
   if(FileExists(dirname)) {
      if(IsDirectory(dirname)) return; // ok, dir is already there
   }
   MakeDirectory(dirname);
}

/*
 * Create this directory, throw an error if it already exists or
 * if a file is in the way or if the creation fails. 
 *
 * Pleas note, this function does not honor parents, 
 * MakeDirectory("onedir/otherdir/onemoredir") will fail
 * if either ondir or otherdir is missing
 */
void RealFSImpl::MakeDirectory(std::string dirname) {
   if(FileExists(dirname)) 
      throw wexception("A File with the name %s already exists\n", dirname.c_str());

   char canonical[256];
	std::string fullname;

	if (!FS_CanonicalizeName(canonical, sizeof(canonical), dirname.c_str()))
		return;

	fullname = m_directory + '/' + canonical;

   int retval=0;
#ifdef WIN32
   retval=mkdir(fullname.c_str());
#else
   retval=mkdir(fullname.c_str(), 0x1FF);
#endif
   if(retval==-1) 
      throw wexception("Couldn't create directory %s: %s\n", dirname.c_str(), strerror(errno));
}

/*
===============
RealFSImpl::Load

Read the given file into alloced memory; called by FileRead::Open.
Throws an exception if the file couldn't be opened.
===============
*/
void *RealFSImpl::Load(std::string fname, int *length)
{
	char canonical[256];
	std::string fullname;
	FILE *file=0;
	void *data=0;
	int size;

	if (!FS_CanonicalizeName(canonical, sizeof(canonical), fname.c_str()))
		throw wexception("Bad filename: %s", fname.c_str());

	fullname = m_directory + '/' + canonical;

	file = 0;
	data = 0;

	try
	{
		file = fopen(fullname.c_str(), "rb");
		if (!file)
			throw wexception("Couldn't open %s (%s)", fname.c_str(), fullname.c_str());

		// determine the size of the file (rather quirky, but it doesn't require
		// potentially unportable functions)
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);

		// allocate a buffer and read the entire file into it
		data = malloc(size + 1);
		if (fread(data, size, 1, file) != 1)
			throw wexception("Read failed for %s (%s)", fname.c_str(), fullname.c_str());
		((char *)data)[size] = 0;

		fclose(file);
		file = 0;
	}
	catch(...)
	{
		if (file)
			fclose(file);
		if (data) {
			free(data);
			data = 0;
		}
		throw;
	}

	if (length)
		*length = size;

	return data;
}

/*
===============
RealFSImpl::Write

Write the given block of memory to the repository.
Throws an exception if it fails.
===============
*/
void RealFSImpl::Write(std::string fname, void *data, int length)
{
	char canonical[256];
	std::string fullname;
	FILE *f;
	int c;

	if (!FS_CanonicalizeName(canonical, sizeof(canonical), fname.c_str()))
		throw wexception("Bad filename: %s", fname.c_str());

	fullname = m_directory + '/' + canonical;

	f = fopen(fullname.c_str(), "wb");
	if (!f)
		throw wexception("Couldn't open %s (%s) for writing", fname.c_str(), fullname.c_str());

	c = fwrite(data, length, 1, f);
	fclose(f);

	if (c != 1)
		throw wexception("Write to %s (%s) failed", fname.c_str(), fullname.c_str());
}

/*
===============
FileSystem::CreateFromDirectory [static]

Create a filesystem to access the given directory as served by the OS
===============
*/
FileSystem *FileSystem::CreateFromDirectory(std::string directory)
{
	return new RealFSImpl(directory);
}

/*
===============
FileSystem::CreateFromZip [static]

Create a filesystem from a zip file 
===============
*/
FileSystem *FileSystem::CreateFromZip(std::string filename)
{
	return new ZipFilesystem( filename );
}


/*
==============================================================================

LayeredFileSystem IMPLEMENTATION

==============================================================================
*/

class LayeredFSImpl : public LayeredFileSystem {
public:
	LayeredFSImpl();
	virtual ~LayeredFSImpl();

	virtual bool IsWritable();

	virtual void AddFileSystem(FileSystem *fs);

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results, int depth); // Overwritten from LayeredFileSystem
	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results); // overwritten from FileSystem

	virtual bool FileExists(std::string path);
   virtual bool IsDirectory(std::string path);
   virtual void EnsureDirectoryExists(std::string dirname);
   virtual void MakeDirectory(std::string dirname);

	virtual void *Load(std::string fname, int *length);
	virtual void Write(std::string fname, void *data, int length);
   
   virtual FileSystem* MakeSubFileSystem( std::string dirname );
   virtual FileSystem* CreateSubFileSystem( std::string dirname, Type );
   virtual void Unlink(std::string file);

private:
	typedef std::vector<FileSystem*>::reverse_iterator FileSystem_rit;

	std::vector<FileSystem*> m_filesystems;
};

/** LayeredFSImpl::LayeredFSImpl()
 *
 * Initialize
 */
LayeredFSImpl::LayeredFSImpl()
{
}

/** LayeredFSImpl::~LayeredFSImpl
 *
 * Free all sub-filesystems
 */
LayeredFSImpl::~LayeredFSImpl()
{
	while(!m_filesystems.empty()) {
		FileSystem *fs = m_filesystems.back();
		delete fs;
		m_filesystems.pop_back();
	}
}

/** LayeredFSImpl::IsWritable()
 *
 * Just assume that at least one of our child FSs is writable
 */
bool LayeredFSImpl::IsWritable()
{
	return true;
}

/** LayeredFSImpl::AddFileSystem(FileSystem *fs)
 *
 * Add a new filesystem to the top of the stack
 */
void LayeredFSImpl::AddFileSystem(FileSystem *fs)
{
	m_filesystems.push_back(fs);
}

/*
===============
LayeredFSImpl::FindFiles

Find files in all sub-filesystems in the given path, with the given pattern.
Store all found files in results. 

If depth is not 0 only search this many subfilesystems.

Returns the number of files found.
===============
*/
int LayeredFSImpl::FindFiles(std::string path, std::string pattern, filenameset_t *results, int depth)
{
   int i=0;
   if(!depth)
      depth=10000; // Wow, if you have so many filesystem you're my hero
   
	for(FileSystem_rit it = m_filesystems.rbegin(); (it != m_filesystems.rend()) && (i<depth); it++, i++) {
		filenameset_t files;
		(*it)->FindFiles(path, pattern, &files);

		// need to workaround MSVC++6 issues
		//results->insert(files.begin(), files.end());
		for(filenameset_t::iterator fnit = files.begin(); fnit != files.end(); fnit++)
			results->insert(*fnit);
	}

	return results->size();
}

int LayeredFSImpl::FindFiles(std::string path, std::string pattern, filenameset_t *results) {
   return FindFiles(path,pattern,results,0);
}

/*
===============
LayeredFSImpl::FileExists

Returns true if the file can be found in at least one of the sub-filesystems
===============
*/
bool LayeredFSImpl::FileExists(std::string path)
{
   for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if ((*it)->FileExists(path))
			return true;
	}

	return false;
}

/*
===============
LayeredFSImpl::IsDirectory

Returns true if the file can be found in at least one of the sub-filesystems
===============
*/
bool LayeredFSImpl::IsDirectory(std::string path)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if ((*it)->IsDirectory(path))
			return true;
	}

	return false;
}
/*
===============
LayeredFSImpl::Load

Read the given file into alloced memory; called by FileRead::Open.
Throws an exception if the file couldn't be opened.

Note: We first query the sub-filesystem whether the file exists. Otherwise,
we'd have problems differentiating the errors returned by the sub-FS.
Let's just avoid any possible hassles with that.
===============
*/
void *LayeredFSImpl::Load(std::string fname, int *length)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->FileExists(fname))
			continue;

		return (*it)->Load(fname, length);
	}

	throw wexception("Couldn't find file %s", fname.c_str());
}


/*
===============
LayeredFSImpl::Write

Write the given block of memory out as a file to the first writable sub-FS.
Throws an exception if it fails.
===============
*/
void LayeredFSImpl::Write(std::string fname, void *data, int length)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		(*it)->Write(fname, data, length);
		return;
	}

	throw wexception("LayeredFSImpl: No writable filesystem!");
}

/* 
 * MakeDir in first writable directory
 */
void LayeredFSImpl::MakeDirectory(std::string dirname) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		(*it)->MakeDirectory(dirname);
		return;
	}

	throw wexception("LayeredFSImpl: No writable filesystem!");
}

/*
 * EnsureDirectoryExists in first writable directory
 */
void LayeredFSImpl::EnsureDirectoryExists(std::string dirname) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		(*it)->EnsureDirectoryExists(dirname);
		return;
	}

	throw wexception("LayeredFSImpl: No writable filesystem!");
}

/*
 * Create a subfilesystem from this directory
 */
FileSystem* LayeredFSImpl::MakeSubFileSystem(std::string dirname ) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;
      
      if(! (*it)->FileExists(dirname)) 
         continue;
      
      return (*it)->MakeSubFileSystem(dirname);
	}

	throw wexception("LayeredFSImpl: Wasn't able to create sub filesystem!");
}

/*
 * Create a new subfilesystem 
 */
FileSystem* LayeredFSImpl::CreateSubFileSystem(std::string dirname, Type type ) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;
     
      return (*it)->CreateSubFileSystem( dirname, type );
	}

	throw wexception("LayeredFSImpl: Wasn't able to create sub filesystem!");
}
    
/*
 * Remove this file or directory. If it is a directory, remove it recursivly
 */
void LayeredFSImpl::Unlink( std::string file ) { 
  if( !FileExists( file ))
     return;
  
   for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;
		if (!(*it)->FileExists(file))
         continue;

		(*it)->Unlink(file);
		return;
	}
}

/** LayeredFileSystem::Create
 *
 * Create a LayeredFileSystem. This is mainly to hide the implementation details
 * from the rest of the world
 */
LayeredFileSystem *LayeredFileSystem::Create()
{
	return new LayeredFSImpl;
}

