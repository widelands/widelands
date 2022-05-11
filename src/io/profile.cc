/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "io/profile.h"

#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include "base/i18n.h"
#include "base/log.h"
#include "base/string.h"
#include "base/wexception.h"
#include "build_info.h"
#include "io/fileread.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filewrite.h"

static char const* trueWords[] = {
   "true",
   /** TRANSLATORS: A variant of the commandline parameter "true" value */
   /** TRANSLATORS: Needs to be consistent with the translations in widelands-console */
   _("true"), "yes",
   /** TRANSLATORS: A variant of the commandline parameter "true" value */
   /** TRANSLATORS: Needs to be consistent with the translations in widelands-console */
   _("yes"), "on",
   /** TRANSLATORS: A variant of the commandline parameter "true" value */
   /** TRANSLATORS: Needs to be consistent with the translations in widelands-console */
   _("on"), "1"};

static char const* falseWords[] = {
   "false",
   /** TRANSLATORS: A variant of the commandline parameter "false" value */
   /** TRANSLATORS: Needs to be consistent with the translations in widelands-console */
   _("false"), "no",
   /** TRANSLATORS: A variant of the commandline parameter "false" value */
   /** TRANSLATORS: Needs to be consistent with the translations in widelands-console */
   _("no"), "off",
   /** TRANSLATORS: A variant of the commandline parameter "false" value */
   /** TRANSLATORS: Needs to be consistent with the translations in widelands-console */
   _("off"), "0"};

Section::Value::Value(const std::string& nname, const char* const nval)
   : used_(false), translate_(false), name_(nname) {
	set_string(nval);
}

Section::Value::Value(const Section::Value& o)
   : used_(o.used_), translate_(o.translate_), name_(o.name_) {
	set_string(o.value_.get());
}

Section::Value::Value(Section::Value&& o) noexcept : Value() {
	using std::swap;
	swap(*this, o);
}

Section::Value& Section::Value::operator=(Section::Value other) noexcept {
	using std::swap;
	swap(*this, other);
	return *this;
}

Section::Value& Section::Value::operator=(Section::Value&& other) noexcept {
	using std::swap;
	swap(*this, other);
	return *this;
}

bool Section::Value::is_used() const {
	return used_;
}

void Section::Value::mark_used() {
	used_ = true;
}

int32_t Section::Value::get_int() const {
	char* endp;
	int64_t const i = strtol(value_.get(), &endp, 0);
	if (*endp != 0) {
		throw wexception("%s: '%s' is not an integer", get_name(), get_untranslated_string());
	}
	int32_t const result = static_cast<int32_t>(i);
	if (i != result) {
		throw wexception("%s: '%s' is out of range", get_name(), get_untranslated_string());
	}

	return result;
}

uint32_t Section::Value::get_natural() const {
	char* endp;
	int64_t i = strtoll(value_.get(), &endp, 0);
	if ((*endp != 0) || i < 0) {
		throw wexception("%s: '%s' is not natural", get_name(), get_untranslated_string());
	}
	return i;
}

uint32_t Section::Value::get_positive() const {
	char* endp;
	int64_t i = strtoll(value_.get(), &endp, 0);
	if ((*endp != 0) || i < 1) {
		throw wexception("%s: '%s' is not positive", get_name(), get_untranslated_string());
	}
	return i;
}

bool Section::Value::get_bool() const {
	for (char const* word : trueWords) {
		if (iequals(value_.get(), word)) {
			return true;
		}
	}
	for (char const* word : falseWords) {
		if (iequals(value_.get(), word)) {
			return false;
		}
	}

	throw wexception("%s: '%s' is not a boolean value", get_name(), get_untranslated_string());
}

char const* Section::Value::get_string() const {
	return translate_ ? i18n::translate(value_.get()) : value_.get();
}

Vector2i Section::Value::get_point() const {
	char* endp = value_.get();
	int64_t const x = strtol(endp, &endp, 0);
	int64_t const y = strtol(endp, &endp, 0);
	if (*endp != 0) {
		throw wexception("%s: '%s' is not a Vector2i", get_name(), get_untranslated_string());
	}
	if (x > std::numeric_limits<int32_t>::max()) {
		throw wexception("%s: '%s' x coordinate too large (> %d)", get_name(),
		                 get_untranslated_string(), std::numeric_limits<int32_t>::max());
	}
	if (y > std::numeric_limits<int32_t>::max()) {
		throw wexception("%s: '%s' y coordinate too large (> %d)", get_name(),
		                 get_untranslated_string(), std::numeric_limits<int32_t>::max());
	}
	return Vector2i(x, y);
}

void Section::Value::set_string(char const* const value) {
	using std::copy;

	const auto len = strlen(value) + 1;
	value_.reset(new char[len]);
	copy(value, value + len, value_.get());
}

void swap(Section::Value& first, Section::Value& second) {
	using std::swap;

	swap(first.name_, second.name_);
	swap(first.value_, second.value_);
	swap(first.used_, second.used_);
	swap(first.translate_, second.translate_);
}

/*
==============================================================================

Section

==============================================================================
*/

char const* Section::get_name() const {
	return section_name_.c_str();
}
void Section::set_name(const std::string& name) {
	section_name_ = name;
}

Section::Section(Profile* const prof, const std::string& name)
   : profile_(prof), used_(false), section_name_(name) {
}

/** Section::is_used()
 *
 */
bool Section::is_used() const {
	return used_;
}

/** Section::mark_used()
 *
 */
void Section::mark_used() {
	used_ = true;
}

/** Section::check_used()
 *
 * Print a warning for every unused value.
 */
void Section::check_used() const {
	for (const Value& temp_value : values_) {
		if (!temp_value.is_used()) {
			profile_->error("Section [%s], key '%s' not used (did you spell the name "
			                "correctly?)",
			                get_name(), temp_value.get_name());
		}
	}
}

bool Section::has_val(char const* const name) const {
	for (const Value& temp_value : values_) {
		if (iequals(temp_value.get_name(), name)) {
			return true;
		}
	}
	return false;
}

/**
 * Returns the Value associated with the given keyname.
 *
 * Args: name  name of the key
 *
 * Returns: Pointer to the Value struct; 0 if the key doesn't exist.
 */
Section::Value* Section::get_val(char const* const name) {
	for (Value& value : values_) {
		if (iequals(value.get_name(), name)) {
			value.mark_used();
			return &value;
		}
	}
	return nullptr;
}

/**
 * Returns the first unused value associated with the given keyname.
 *
 * Args: name  name of the key; can be 0 to find any key
 *
 * Returns: Pointer to the Value struct; 0 if no more key-value pairs are found
 */
Section::Value* Section::get_next_val(char const* const name) {
	for (Value& value : values_) {
		if (!value.is_used()) {
			if ((name == nullptr) || iequals(value.get_name(), name)) {
				value.mark_used();
				return &value;
			}
		}
	}
	return nullptr;
}

Section::Value& Section::create_val(char const* const name, char const* const value) {
	for (Value& temp_value : values_) {
		if (iequals(temp_value.get_name(), name)) {
			temp_value.set_string(value);
			return temp_value;
		}
	}
	return create_val_duplicate(name, value);
}

Section::Value& Section::create_val_duplicate(char const* const name, char const* const value) {
	values_.emplace_back(name, value);
	return values_.back();
}

/**
 * Return the integer value of the given key or throw an exception if a
 * problem arises.
 */
int32_t Section::get_safe_int(char const* const name) {
	Value* const v = get_val(name);
	if (v == nullptr) {
		throw wexception("[%s]: missing integer key '%s'", get_name(), name);
	}
	return v->get_int();
}

/// Return the natural value of the given key or throw an exception.
uint32_t Section::get_safe_natural(char const* const name) {
	if (Value* const v = get_val(name)) {
		return v->get_natural();
	}
	throw wexception("[%s]: missing natural key '%s'", get_name(), name);
}

/// Return the positive value of the given key or throw an exception.
uint32_t Section::get_safe_positive(char const* const name) {
	if (Value* const v = get_val(name)) {
		return v->get_positive();
	}
	throw wexception("[%s]: missing positive key '%s'", get_name(), name);
}

/**
 * Return the boolean value of the given key or throw an exception if a
 * problem arises.
 */
bool Section::get_safe_bool(char const* const name) {
	Value* const v = get_val(name);
	if (v == nullptr) {
		throw wexception("[%s]: missing boolean key '%s'", get_name(), name);
	}
	return v->get_bool();
}

/**
 * Return the key value as a plain string or throw an exception if the key
 * doesn't exist
 */
char const* Section::get_safe_string(char const* const name) {
	Value* const v = get_val(name);
	if (v == nullptr) {
		throw wexception("[%s]: missing key '%s'", get_name(), name);
	}
	return v->get_string();
}

char const* Section::get_safe_untranslated_string(char const* const name) {
	Value* const v = get_val(name);
	if (v == nullptr) {
		throw wexception("[%s]: missing key '%s'", get_name(), name);
	}
	return v->get_untranslated_string();
}

/**
 * Return the key value as a plain string or throw an exception if the key
 * does not exist.
 */
const char* Section::get_safe_string(const std::string& name) {
	return get_safe_string(name.c_str());
}

/**
 * Returns the integer value of the given key. Falls back to a default value
 * if the key is not found.
 *
 * Args: name  name of the key
 *       def   fallback value
 *
 * Returns: the integer value of the key
 */
int32_t Section::get_int(char const* const name, int32_t const def) {
	Value* const v = get_val(name);
	if (v == nullptr) {
		return def;
	}

	try {
		return v->get_int();
	} catch (const std::exception& e) {
		profile_->error("%s", e.what());
	}

	return def;
}

uint32_t Section::get_natural(char const* const name, uint32_t const def) {
	if (Value* const v = get_val(name)) {
		try {
			return v->get_natural();
		} catch (const std::exception& e) {
			profile_->error("%s", e.what());
			return def;
		}
	} else {
		return def;
	}
}

uint32_t Section::get_positive(char const* const name, uint32_t const def) {
	if (Value* const v = get_val(name)) {
		try {
			return v->get_positive();
		} catch (const std::exception& e) {
			profile_->error("%s", e.what());
			return def;
		}
	}

	return def;
}

/**
 * Returns the boolean value of the given key. Falls back to a default value
 * if the key is not found.
 *
 * Args: name  name of the key
 *       def   fallback value
 *
 * Returns: the boolean value of the key
 */
bool Section::get_bool(char const* const name, bool const def) {
	Value* const v = get_val(name);
	if (v == nullptr) {
		return def;
	}

	try {
		return v->get_bool();
	} catch (const std::exception& e) {
		profile_->error("%s", e.what());
	}

	return def;
}

/**
 * Returns the value of the given key. Falls back to a default value if the
 * key is not found.
 *
 * Args: name  name of the key
 *       def   fallback value
 *
 * Returns: the string associated with the key; never returns 0 if the key
 *          has been found
 */
char const* Section::get_string(char const* const name, char const* const def) {
	Value const* const v = get_val(name);
	return v != nullptr ? v->get_string() : def;
}

Vector2i Section::get_point(const char* const name, const Vector2i def) {
	Value const* const v = get_val(name);
	return v != nullptr ? v->get_point() : def;
}

/**
 * Retrieve the next unused key with the given name as a boolean value.
 *
 * Args: name   name of the key, can be 0 to find all unused keys
 *       value  value of the key is stored here
 *
 * Returns: the name of the key, or 0 if none has been found
 */
char const* Section::get_next_bool(char const* const name, bool* const value) {
	Value* const v = get_next_val(name);
	if (v == nullptr) {
		return nullptr;
	}

	if (value != nullptr) {
		*value = v->get_bool();
	}
	return v->get_name();
}

/**
 * Modifies/Creates the given key.
 */
void Section::set_int(char const* const name, int32_t const value) {
	set_string(name, std::to_string(value));
}

void Section::set_natural(char const* const name, uint32_t const value) {
	set_string(name, std::to_string(static_cast<int64_t>(value)));
}

void Section::set_string(char const* const name, char const* string) {
	create_val(name, string).mark_used();
}

void Section::set_string_duplicate(char const* const name, char const* const string) {
	create_val_duplicate(name, string).mark_used();
}

void Section::set_translated_string(char const* const name, char const* string) {
	Value& v = create_val(name, string);
	v.mark_used();
	v.set_translate(true);
}
void Section::set_translated_string_duplicate(char const* const name, char const* const string) {
	Value& v = create_val_duplicate(name, string);
	v.mark_used();
	v.set_translate(true);
}

/*
==============================================================================

Profile

==============================================================================
*/

/** Profile::Profile(int32_t error_level = err_throw)
 *
 * Create an empty profile
 */
Profile::Profile(int32_t error_level) {
	error_level_ = error_level;
}

/**
 * Parses an ini-style file into sections and key-value pairs.
 * If global_section is not null, keys outside of sections are put into a
 * section
 * of that name.
 */
Profile::Profile(char const* const filename,
                 char const* const global_section,
                 int32_t const error_level)
   : filename_(filename), error_level_(error_level) {
	read(filename, global_section);
}

Profile::Profile(char const* const filename,
                 char const* const global_section,
                 const std::string& textdomain,
                 int32_t const error_level)
   : filename_(filename), error_level_(error_level) {
	i18n::Textdomain td(textdomain);
	read(filename, global_section);
}

/**
 * Output an error message.
 * Depending on the error level, it is thrown as a wexception, logged or
 * ignored.
 */
void Profile::error(char const* const fmt, ...) const {
	if (error_level_ == err_ignore) {
		return;
	}

	char buffer[256];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (error_level_ == err_log) {
		log_err("[%s] %s\n", filename_.c_str(), buffer);
	} else {
		throw wexception("[%s] %s", filename_.c_str(), buffer);
	}
}

/** Profile::check_used()
 *
 * Signal an error if a section or key hasn't been used.
 */
void Profile::check_used() const {
	for (const Section& temp_section : sections_) {
		if (!temp_section.is_used()) {
			error(
			   "Section [%s] not used (did you spell the name correctly?)", temp_section.get_name());
		} else {
			temp_section.check_used();
		}
	}
}

/**
 * Retrieve the first section of the given name and mark it used.
 *
 * @param name name of the section
 *
 * @return pointer to the section (or 0 if the section doesn't exist)
 */
Section* Profile::get_section(const std::string& name) {
	for (Section& temp_section : sections_) {
		if (iequals(temp_section.get_name(), name)) {
			temp_section.mark_used();
			return &temp_section;
		}
	}
	return nullptr;
}

/**
 * Safely get a section of the given name.
 * If the section doesn't exist, an exception is thrown.
 */
Section& Profile::get_safe_section(const std::string& name) {
	if (Section* const s = get_section(name)) {
		return *s;
	}
	throw wexception("in \"%s\" section [%s] not found", filename_.c_str(), name.c_str());
}

/**
 * Safely get a section of the given name.
 * If the section doesn't exist, it is created.
 * Similar to create_section(), but the section is marked as used.
 */
Section& Profile::pull_section(char const* const name) {
	Section& s = create_section(name);
	s.mark_used();
	return s;
}

/**
 * Retrieve the next unused section of the given name and mark it used.
 *
 * Args: name  name of the section; can be 0 to retrieve any unused section
 *
 * Returns: pointer to the section (or 0 if the section doesn't exist)
 */
Section* Profile::get_next_section(char const* const name) {
	for (Section& section : sections_) {
		if (!section.is_used()) {
			if ((name == nullptr) || iequals(section.get_name(), name)) {
				section.mark_used();
				return &section;
			}
		}
	}
	return nullptr;
}

Section& Profile::create_section(char const* const name) {
	for (Section& section : sections_) {
		if (iequals(section.get_name(), name)) {
			return section;
		}
	}
	return create_section_duplicate(name);
}

Section& Profile::create_section_duplicate(char const* const name) {
	sections_.push_back(Section(this, name));
	return sections_.back();
}

inline char* skipwhite(char* p) {
	while ((*p != 0) && (isspace(*p) != 0)) {
		++p;
	}
	return p;
}

inline void rtrim(char* const str) {
	for (char* p = strchr(str, '\0'); str < p; --p) {
		if (isspace(p[-1]) == 0) {
			*p = 0;
			break;
		}
	}
}

inline void killcomments(char* p) {
	while (*p != 0) {
		if (p[0] == '#') {
			p[0] = '\0';
			break;
		}
		++p;
	}
}

/**
 * Parses an ini-style file into sections and key values. If a section or
 * key name occurs multiple times, an additional entry is created.
 *
 * Args: filename  name of the source file
 */
void Profile::read(char const* const filename, char const* const global_section, FileSystem& fs) {
	uint32_t linenr = 0;
	try {
		FileRead fr;
		fr.open(fs, filename);

		char* p = nullptr;
		Section* s = nullptr;

		bool reading_multiline = false;
		std::string data;
		char* key = nullptr;
		bool translate_line = false;
		while (char* line = fr.read_line()) {
			++linenr;

			if (!reading_multiline) {
				p = line;
			}

			p = skipwhite(p);
			if ((p[0] == 0) || p[0] == '#') {
				continue;
			}

			if (p[0] == '[') {
				++p;
				if (char* const closing = strchr(p, ']')) {
					*closing = '\0';
				} else {
					throw wexception("missing ']' after \"%s\"", p);
				}
				s = &create_section_duplicate(p);
			} else {
				char* tail = nullptr;
				translate_line = false;
				if (reading_multiline) {
					// Note: comments are killed by walking backwards into the string
					rtrim(p);
					while (*line != '\'' && *line != '"') {
						if (*line == 0) {
							throw wexception("runaway multiline string");
						}
						if (*line == '_') {
							translate_line = true;
						}
						++line;
					}

					// skip " or '
					++line;

					for (char* eot = line + strlen(line) - 1; *eot != '"' && *eot != '\''; --eot) {
						*eot = 0;
					}
					// NOTE: we leave the last '"' and do not remove them
					tail = line;
				} else {
					tail = strchr(p, '=');
					if (tail == nullptr) {
						throw wexception("invalid syntax: %s", line);
					}
					*tail++ = '\0';
					key = p;
					if (*tail == '_') {
						++tail;  // skip =_, which is only used for translations
						translate_line = true;
					}
					tail = skipwhite(tail);
					killcomments(tail);
					rtrim(tail);
					rtrim(p);

					// first, check for multiline string
					if ((tail[0] == '\'' || tail[0] == '"') && (tail[1] == '\'' || tail[1] == '"')) {
						reading_multiline = true;
						tail += 2;
					}

					// then remove surrounding '' or ""
					if (tail[0] == '\'' || tail[0] == '"') {
						++tail;
					}
				}
				if (tail != nullptr) {
					char* const eot = tail + strlen(tail) - 1;
					if (*eot == '\'' || *eot == '"') {
						*eot = '\0';
						if (*tail != 0) {
							char* const eot2 = tail + strlen(tail) - 1;
							if (*eot2 == '\'' || *eot2 == '"') {
								reading_multiline = false;
								*eot2 = '\0';
							}
						}
					}

					// ready to insert
					if (s == nullptr) {
						if (global_section != nullptr) {
							s = &create_section_duplicate(global_section);
						} else {
							throw wexception("key %s outside section", p);
						}
					}
					assert(s);
					data += tail;

					if (!reading_multiline) {
						Section::Value& result = s->create_val_duplicate(key, data.c_str());
						result.set_translate(translate_line);
						data.clear();
					}
				} else {
					throw wexception("syntax error");
				}
			}
		}
	} catch (const FileNotFoundError&) {
		// It's no problem if the config file does not exist. (It'll get
		// written on exit anyway)
		log_warn("There's no configuration file, using default values.\n");
	} catch (const std::exception& e) {
		error("%s:%u: %s", filename, linenr, e.what());
	}

	//  Make sure that the requested global section exists, even if it is empty.
	if ((global_section != nullptr) && (get_section(global_section) == nullptr)) {
		create_section_duplicate(global_section);
	}
}

/**
 * Writes all sections out to the given file.
 * If used_only is true, only used sections and keys are written to the file.
 * comment is optional text to be put to second line
 */
void Profile::write(char const* const filename,
                    bool const used_only,
                    FileSystem& fs,
                    char const* const comment) {
	FileWrite fw;

	fw.print_f(
	   "# Automatically created by Widelands %s (%s)\n", build_id().c_str(), build_type().c_str());

	if (comment != nullptr) {
		fw.print_f("# %s\n", comment);
	}

	for (const Section& temp_section : sections_) {
		if (used_only && !temp_section.is_used()) {
			continue;
		}

		fw.print_f("\n[%s]\n", temp_section.get_name());

		for (const Section::Value& temp_value : temp_section.values_) {
			if (used_only && !temp_value.is_used()) {
				continue;
			}

			char const* const str = temp_value.get_untranslated_string();

			if (*str != 0) {
				uint32_t spaces = strlen(temp_value.get_name());
				bool multiline = false;

				for (uint32_t i = 0; i < strlen(str); ++i) {
					if (str[i] == '\n') {
						multiline = true;
					}
				}

				// Try to avoid _every_ possible way of
				// getting inconsistent data
				std::string tempstr;

				if (multiline) {
					// Show WL that a multilined text starts
					tempstr += "\"";
				}

				for (char const* it = str; *it != 0; ++it) {
					// No speach marks - they would break the format
					switch (*it) {
					case '"':
						tempstr += "''";
						break;
					case '\n':
						// Convert the newlines to WL format.
						tempstr += " \"\n";
						for (uint32_t j = 0; j < spaces + 1; ++j) {
							tempstr += ' ';
						}
						tempstr += " \"";
						break;
					default:
						tempstr += *it;
						break;
					}
				}

				if (multiline) {
					// End of multilined text.
					tempstr += '"';
				}

				fw.print_f("%s=%s\"%s\"\n", temp_value.get_name(),
				           temp_value.get_translate() ? "_" : "", tempstr.c_str());
			} else {
				fw.print_f("%s=\n", temp_value.get_name());
			}
		}
	}

	fw.write(fs, filename);
}
