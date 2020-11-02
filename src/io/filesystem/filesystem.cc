/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
// this originally comes from Return to the Shadows (http://www.rtts.org/)
// files.cc: provides all the OS abstraction to access files

#include "io/filesystem/filesystem.h"

#include <cassert>
#include <cstdlib>
#include <list>
#include <string>
#ifdef _WIN32
#include <cstdio>
#endif

// We have to add Boost to this block to make codecheck happy
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <glob.h>
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <unistd.h>

#include "base/i18n.h"
#include "base/log.h"
#include "config.h"
#include "graphic/text_layout.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filesystem/zip_filesystem.h"

#ifdef _WIN32
#define stat _stat
#endif

#ifdef _MSC_VER
#define S_ISDIR(x) ((x & _S_IFDIR) ? 1 : 0)
#define S_ISREG(x) ((x & _S_IFREG) ? 1 : 0)
#define PATH_MAX MAX_PATH
#endif

namespace {
// Characters that are allowed in filenames, but not at the beginning
const std::vector<std::string> illegal_filename_starting_characters{
   ".", "-",
   " ",  // Keep the blank last
};

// Characters that are disallowed anywhere in a filename
// No potential file separators or other potentially illegal characters
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
// http://www.linfo.org/file_name.html
// https://support.apple.com/en-us/HT202808
// We can't just regex for word & digit characters here because of non-Latin scripts.
const std::vector<std::string> illegal_filename_characters{
   "<", ">", ":", "\"", "|", "?", "*", "/", "\\",
};

/// A class that makes iteration over filename_?.* templates easy. It is much faster than using
/// regex.
class NumberGlob {
public:
	explicit NumberGlob(const std::string& file_template);

	/// If there is a next filename, puts it in 's' and returns true.
	bool next(std::string* s);

private:
	std::string template_;
	std::string format_;
	std::string to_replace_;
	uint32_t current_;
	uint32_t max_;

	DISALLOW_COPY_AND_ASSIGN(NumberGlob);
};

/**
 * Implementation for NumberGlob.
 */
NumberGlob::NumberGlob(const std::string& file_template) : template_(file_template), current_(0) {
	int nchars = count(file_template.begin(), file_template.end(), '?');
	format_ = "%0" + boost::lexical_cast<std::string>(nchars) + "i";

	max_ = 1;
	for (int i = 0; i < nchars; ++i) {
		max_ *= 10;
		to_replace_ += "?";
	}
	--max_;
}

bool NumberGlob::next(std::string* s) {
	if (current_ > max_) {
		return false;
	}

	if (max_) {
		*s = boost::replace_last_copy(
		   template_, to_replace_, (boost::format(format_) % current_).str());
	} else {
		*s = template_;
	}
	++current_;
	return true;
}
}  // namespace

/**
 * \param path A file or directory name
 * \return True if ref path is absolute and within this FileSystem, false otherwise
 */
bool FileSystem::is_path_absolute(const std::string& path) const {
	std::string::size_type const path_size = path.size();
	std::string::size_type const root_size = root_.size();

	if (path_size < root_size) {
		return false;
	}
	if (path_size == root_size) {
		return path == root_;
	}
	if (path.compare(0, root_.size(), root_)) {
		return false;
	}

#ifdef _WIN32
	if (path.size() >= 3 && path[1] == ':' && path[2] == '\\')  // "C:\"
	{
		return true;
	}
#endif
	assert(root_size < path_size);  //  Otherwise an invalid read happens below.
	return path[root_size] == file_separator();
}

/**
 * Fix a path that might come from another OS.
 * This function is used to make sure that paths send via network are usable
 * on local OS.
 */
std::string FileSystem::fix_cross_file(const std::string& path) const {
	uint32_t path_size = path.size();
	std::string fixedPath(path);
	std::string temp;
	for (uint32_t i = 0; i < path_size; ++i) {
		temp = path.at(i);
#ifdef _WIN32
		if (temp == ":") {
			fixedPath.at(i) = '-';
		} else if (temp == "/") {
#else
		if (temp == "\\") {
#endif
			fixedPath.at(i) = file_separator();
		}
		// As a security measure, eat all:
		// * tildes
		// * double dots
		// * dots with following slash/backslash (but not a single dot - we need it in e.g. "xyz.wmf")
		// away to avoid misuse of the file transfer function.
		if (temp == "~") {
			fixedPath.at(i) = '_';
		}
		if (temp == "." && (i + 1 < path_size)) {
			std::string temp2;
			temp2 = path.at(i + 1);
			if (temp2 == "." || temp2 == "\\" || temp2 == "/") {
				fixedPath.at(i) = '_';
			}
		}
	}
	return fixedPath;
}

/**
 * \return The process' current working directory
 */
// static
std::string FileSystem::get_working_directory() {
	char cwd[PATH_MAX + 1];
	char* const result = getcwd(cwd, PATH_MAX);
	if (!result) {
		throw FileError("FileSystem::get_working_directory()", "widelands", "can not run getcwd");
	}

	return std::string(cwd);
}

// static
char FileSystem::file_separator() {
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}

bool FileSystem::is_legal_filename(const std::string& filename) {
	if (filename.empty()) {
		return false;
	}
	for (const std::string& illegal_start : illegal_filename_starting_characters) {
		if (boost::starts_with(filename, illegal_start)) {
			return false;
		}
	}
	for (const std::string& illegal_char : illegal_filename_characters) {
		if (boost::contains(filename, illegal_char)) {
			return false;
		}
	}
	return true;
}

std::string FileSystem::illegal_filename_tooltip() {
	std::vector<std::string> starting_characters;
	for (const std::string& character : illegal_filename_starting_characters) {
		if (character == " ") {
			/** TRANSLATORS: Part of tooltip entry for characters in illegal filenames. replaces tha
			 * blank space in a list of illegal characters */
			starting_characters.push_back(pgettext("illegal_filename_characters", "blank space"));
		} else {
			starting_characters.push_back(character);
		}
	}
	const std::string illegal_start(as_listitem(
	   /** TRANSLATORS: Tooltip entry for characters in illegal filenames.
	    *  %s is a list of illegal characters */
	   (boost::format(pgettext("illegal_filename_characters", "%s at the start of the filename")) %
	    richtext_escape(i18n::localize_list(starting_characters, i18n::ConcatenateWith::OR)))
	      .str(),
	   UI::FontStyle::kWuiMessageParagraph));

	const std::string illegal(as_listitem(
	   /** TRANSLATORS: Tooltip entry for characters in illegal filenames.
	    * %s is a list of illegal characters */
	   (boost::format(pgettext("illegal_filename_characters", "%s anywhere in the filename")) %
	    richtext_escape(i18n::localize_list(illegal_filename_characters, i18n::ConcatenateWith::OR)))
	      .str(),
	   UI::FontStyle::kWuiMessageParagraph));

	return (boost::format("%s%s%s") %
	        /** TRANSLATORS: Tooltip header for characters in illegal filenames.
	         * This is followed by a list of bullet points */
	        pgettext("illegal_filename_characters", "The following characters are not allowed:") %
	        illegal_start % illegal)
	   .str();
}

// TODO(unknown): Write homedir detection for non-getenv-systems
std::string FileSystem::get_homedir() {
	std::string homedir;
#ifdef _WIN32
	// Trying to get it compatible to ALL windows versions...
	// Could anybody please hit the Megasoft devs for not keeping
	// their own "standards"?
	char const* home_char;
#define TRY_USE_AS_HOMEDIR(name)                                                                   \
	home_char = getenv(name);                                                                       \
	if (home_char != nullptr && *home_char != 0 && check_writeable_for_data(home_char))             \
		return home_char;

	TRY_USE_AS_HOMEDIR("USERPROFILE")
	TRY_USE_AS_HOMEDIR("HOMEPATH")
	TRY_USE_AS_HOMEDIR("HOME")
	TRY_USE_AS_HOMEDIR("APPDATA")

	log_warn("None of the directories was useable - falling back to \".\"\n");
#else
#ifdef HAS_GETENV
	if (char const* const h = getenv("HOME")) {
		homedir = h;
	}
#endif
#endif

	if (homedir.empty()) {
		log_warn("\neither we can not detect your home directory "
		         "or you do not have one! Please contact the developers.\n\n");

		// TODO(unknown): is it really a good idea to set homedir to "." then ??

		log_warn("Instead of your home directory, '.' will be used.\n\n");
		homedir = ".";
	}

	return homedir;
}

#ifdef USE_XDG
/**
 * Return $XDG_DATA_HOME/widelands. Falls back to $HOME/.local/share/widelands
 * https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
 * Prioritises $HOME/.widelands if it exists.
 */
std::string FileSystem::get_userdatadir() {
	std::string userdatadir = get_homedir();

	// Use dotfolder for backwards compatibility if it exists.
	RealFSImpl dot(userdatadir);
	if (dot.is_directory(".widelands")) {
		userdatadir = userdatadir + "/.widelands";
	}
#ifdef HAS_GETENV
	else {
		if (char const* const datahome = getenv("XDG_DATA_HOME")) {
			userdatadir = std::string(datahome) + "/widelands";
		} else {
			// If XDG_DATA_HOME is not set, the default path is used.
			userdatadir = userdatadir + "/.local/share/widelands";
		}
	}
#else
	else {
		// Fallback to not dump all files into the current working dir.
		userdatadir = userdatadir + "/.widelands";
	}
#endif

	// Unlike the homedir function, this function includes the program name.
	// This is handled in 'src/wlapplication.cc'.
	return userdatadir;
}

/**
 * Return $XDG_CONFIG_HOME/widelands. Falls back to $HOME/.config/widelands
 * https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
 * Prioritises $HOME/.widelands if it exists.
 */
std::string FileSystem::get_userconfigdir() {
	std::string userconfigdir = get_homedir();

	// Use dotfolder for backwards compatibility if it exists.
	RealFSImpl dot(userconfigdir);
	if (dot.is_directory(".widelands")) {
		userconfigdir = userconfigdir + "/.widelands";
	}
#ifdef HAS_GETENV
	else {
		if (char const* const confighome = getenv("XDG_CONFIG_HOME")) {
			userconfigdir = std::string(confighome) + "/widelands";
		} else {
			// If XDG_CONFIG_HOME is not set, the default path is used.
			userconfigdir = userconfigdir + "/.config/widelands";
		}
	}
#else
	else {
		// Fallback to not dump all files into the current working dir.
		userconfigdir = userconfigdir + "/.widelands";
	}
#endif

	// Unlike the homedir function, this function includes the program name.
	// This is handled in 'src/wlapplication.cc'.
	return userconfigdir;
}

/**
 * Return $XDG_DATA_DIRS. Falls back to /usr/local/share:/usr/share
 * https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
 */
std::vector<std::string> FileSystem::get_xdgdatadirs() {
	std::vector<std::string> xdgdatadirs;
	const char* environment_char = nullptr;
#ifdef HAS_GETENV
	environment_char = getenv("XDG_DATA_DIRS");
#endif
	std::string environment(environment_char == nullptr || *environment_char == 0 ?
	                           "/usr/local/share:/usr/share" :
	                           environment_char);

	// https://stackoverflow.com/a/14266139
	std::string token;
	std::string delimiter = ":";
	size_t pos = 0;
	while ((pos = environment.find(delimiter)) != std::string::npos) {
		token = environment.substr(0, pos);
		xdgdatadirs.push_back(token);
		environment.erase(0, pos + delimiter.length());
	}
	xdgdatadirs.push_back(environment);
	return xdgdatadirs;
}
#endif

// Returning a vector rather than a set because animations need the indices
std::vector<std::string> FileSystem::get_sequential_files(const std::string& directory,
                                                          const std::string& basename,
                                                          const std::string& extension) const {
	std::vector<std::string> result;

	auto get_files = [this, directory, basename, extension](const std::string& number_template) {
		std::vector<std::string> files;
		const std::string filename_template =
		   directory + file_separator() + basename + number_template + "." + extension;

		NumberGlob glob(filename_template);
		std::string filename;
		while (glob.next(&filename)) {
			if (!file_exists(filename)) {
				break;
			}
			files.push_back(filename);
		}
		return files;
	};
	result = get_files("");
	if (result.empty()) {
		result = get_files("_?");
	}
	if (result.empty()) {
		result = get_files("_??");
	}
	if (result.empty()) {
		result = get_files("_???");
	}
	return result;
}

/**
 * Split a string into components separated by a certain character.
 *
 * \param path The path to parse
 * \param filesep The file path separator used by the native filesystem
 * \param components The output iterator to place the path nodes into
 */
template <typename Inserter>
static void fs_tokenize(const std::string& path, char const filesep, Inserter components) {
	std::string::size_type pos;   //  start of token
	std::string::size_type pos2;  //  next filesep character

	// Extract the first path component
	if (path.front() == filesep) {  // Is this an absolute path?
		pos = 1;
	} else {  // Relative path
		pos = 0;
	}
	pos2 = path.find(filesep, pos);
	// 'current' token is now between pos and pos2

	// Split path into it's components
	while (pos2 != std::string::npos) {
		if (pos != pos2) {
			std::string node = path.substr(pos, pos2 - pos);
			*components++ = node;
		}
		pos = pos2 + 1;
		pos2 = path.find(filesep, pos);
	}

	// Extract the last component (most probably a filename)
	std::string node = path.substr(pos);
	if (!node.empty()) {
		*components++ = node;
	}
}

/**
 * Transform any valid, unique pathname into a well-formed absolute path
 */
// TODO(unknown): Enable non-Unix paths
std::string FileSystem::canonicalize_name(const std::string& path) const {
	std::list<std::string> components;
	std::list<std::string>::iterator i;

#ifdef _WIN32
	std::string temp_path = path;
	// replace all slashes with backslashes so following can work.
	for (uint32_t j = 0; j < temp_path.size(); ++j) {
		if (temp_path[j] == '/') {
			temp_path[j] = '\\';
		}
	}
#else
	const std::string& temp_path = path;
#endif

	fs_tokenize(temp_path, file_separator(), std::inserter(components, components.begin()));

	// Tilde expansion
	if (!components.empty() && *components.begin() == "~") {
		components.erase(components.begin());
		fs_tokenize(get_homedir(), file_separator(), std::inserter(components, components.begin()));
	} else if (!is_path_absolute(temp_path)) {
		//  make relative paths absolute (so that "../../foo" can work)
		fs_tokenize(root_.empty() ? get_working_directory() : root_, file_separator(),
		            std::inserter(components, components.begin()));
	}

	// Clean up the path
	for (i = components.begin(); i != components.end();) {
		char const* str = i->c_str();
		if (*str == '.') {
			++str;

			// Remove single dot
			if (*str == '\0') {
				i = components.erase(i);
				continue;
			}
			// Remove double dot and the preceding component (if any)
			else if (*str == '.' && *(str + 1) == '\0') {
				if (i != components.begin()) {
#ifdef _WIN32
					// On windows don't remove driveletter in this error condition
					if (--i != components.begin())
						i = components.erase(i);
					else
						++i;
#else
					i = components.erase(--i);
#endif
				}
				i = components.erase(i);
				continue;
			}
		}

		++i;
	}

	std::string canonpath;
	canonpath.reserve(temp_path.length());
#ifndef _WIN32
	for (i = components.begin(); i != components.end(); ++i) {
		canonpath.push_back('/');
		canonpath += *i;
	}
#else
	for (i = components.begin(); i != components.end(); ++i) {
		canonpath += *i;
		canonpath += '\\';
	}

	// Remove trailing slash
	if (canonpath.size() > 1)
		canonpath.erase(canonpath.end() - 1);
#endif

	return canonpath;
}

/**
 * Returns the filename of this path, everything after the last
 * / or \  (or the whole string)
 */
const char* FileSystem::fs_filename(const char* p) {
	const char* result = p;

	while (*p != '\0') {
		if (*p == '/' || *p == '\\') {
			result = p + 1;
		}
		++p;
	}

	return result;
}

std::string FileSystem::fs_dirname(const std::string& full_path) {
	std::string filename = fs_filename(full_path.c_str());
	filename = full_path.substr(0, full_path.size() - filename.size());
#ifdef _WIN32
	std::replace(filename.begin(), filename.end(), '\\', '/');
#endif
	return filename;
}

std::string FileSystem::filename_ext(const std::string& f) {
	// Find last '.' - denotes start of extension
	size_t ext_start = f.rfind('.');

	if (std::string::npos == ext_start) {
		return "";
	} else {
		return f.substr(ext_start);
	}
}

std::string FileSystem::filename_without_ext(const char* const p) {
	std::string fname(p ? FileSystem::fs_filename(p) : "");
	std::string ext(FileSystem::filename_ext(fname));
	return fname.substr(0, fname.length() - ext.length());
}

/// Create a filesystem from a zipfile or a real directory
/// \throw FileNotFoundError if root does not exist, is some kind of special
/// file, loops around (via symlinks) or is too long for the OS/filesystem.
/// \throw FileAccessDeniedError if the OS denies access (of course ;-)
/// \throw FileTypeError if root is neither a directory or regular file
// TODO(unknown): Catch FileTypeError in all users
// TODO(unknown): Check for existence before doing anything with the file/dir
// TODO(unknown): Catch FileNotFoundError in all users
// TODO(unknown): throw FileTypeError if root is not a zipfile (exception from
// ZipFilesystem)
FileSystem& FileSystem::create(const std::string& root) {
	struct stat statinfo;

	if (stat(root.c_str(), &statinfo) == -1) {
		if (errno == EBADF || errno == ENOENT || errno == ENOTDIR ||
#ifdef ELOOP
		    errno == ELOOP ||  // MinGW does not support ELOOP (yet)
#endif
		    errno == ENAMETOOLONG) {
			throw FileNotFoundError("FileSystem::create", root);
		}
		if (errno == EACCES) {
			throw FileAccessDeniedError("FileSystem::create", root);
		}
	}

	if (S_ISDIR(statinfo.st_mode)) {
		return *new RealFSImpl(root);
	}
	if (S_ISREG(statinfo.st_mode)) {  // TODO(unknown): ensure root is a zipfile
		return *new ZipFilesystem(root);
	}

	throw FileTypeError(
	   "FileSystem::create", root, "cannot create virtual filesystem from file or directory");
}

#ifdef _WIN32
/// hack that is unfortunately needed for windows to check whether Widelands
/// can write in the directory
bool FileSystem::check_writeable_for_data(char const* const path) {
	RealFSImpl fs(path);

	if (fs.is_directory(".widelands"))
		return true;
	try {
		// throws an exception if not writable
		fs.ensure_directory_exists(".widelands");
		fs.fs_unlink(".widelands");
		return true;
	} catch (const FileError& e) {
		log_warn("Directory %s is not writeable - next try: %s\n", path, e.what());
	}

	return false;
}
#endif
