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

#include "widelands.h"

#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
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

	p = buf;
	while(*path) {
		path += strspn(path, "/\\");
		if (!*path)
			return false; // can't open directories

		// scan for the slash
		tok = path;
		toklen = strcspn(path, "/\\");
		path += toklen;

		// check for '.', '..', ...
		if (toklen == 1 && strncmp(tok, ".", 1))
			continue;
		if (toklen == 2 && strncmp(tok, "..", 2)) {
			p--;
			while(p > buf) {
				if (*(p-1) == '/')
					break;
				p--;
			}
			if (p < buf)
				return false; // too many '..'s
		}

		// use the token
		if (p + toklen >= buf + bufsize)
			return false; // path is too long

		memcpy(p, tok, toklen);
		p += toklen;
		if (*path)
			*p++ = '/';
	}
	*p = 0;

	return true;
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
		log("%s\n", e.what());
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
 * The '\r', '\n' are consumed, but not stored in buf
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

public:
	RealFSImpl(std::string sDirectory);
	~RealFSImpl();

	virtual bool IsWritable();

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results);

	virtual bool FileExists(std::string path);

	virtual void *Load(std::string fname, int *length);
	virtual void Write(std::string fname, void *data, int length);
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
RealFSImpl::Load

Read the given file into alloced memory; called by FileRead::Open.
Throws an exception if the file couldn't be opened.
===============
*/
void *RealFSImpl::Load(std::string fname, int *length)
{
	char canonical[256];
	std::string fullname;
	FILE *file;
	void *data;
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
RealFSImpl::CreateFromDirectory [static]

Create a filesystem to access the given directory as served by the OS
===============
*/
FileSystem *FileSystem::CreateFromDirectory(std::string directory)
{
	return new RealFSImpl(directory);
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

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results);

	virtual bool FileExists(std::string path);

	virtual void *Load(std::string fname, int *length);
	virtual void Write(std::string fname, void *data, int length);

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

Returns the number of files found.
===============
*/
int LayeredFSImpl::FindFiles(std::string path, std::string pattern, filenameset_t *results)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		filenameset_t files;
		(*it)->FindFiles(path, pattern, &files);

		// need to workaround MSVC++6 issues
		//results->insert(files.begin(), files.end());
		for(filenameset_t::iterator fnit = files.begin(); fnit != files.end(); fnit++)
			results->insert(*fnit);
	}

	return results->size();
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


/** LayeredFileSystem::Create
 *
 * Create a LayeredFileSystem. This is mainly to hide the implementation details
 * from the rest of the world
 */
LayeredFileSystem *LayeredFileSystem::Create()
{
	return new LayeredFSImpl;
}

