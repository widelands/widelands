/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef PROFILE_H
#define PROFILE_H

#include "layered_filesystem.h" //TODO: as soon as g_fs is not needed anymore, this can be changed to filesystem.h
#include "widelands_geometry.h"

#include "point.h"

#include <vector>

using Widelands::Coords;

extern class Profile g_options;
class FileSystem;

/**
 * Represents one section inside the .ini-style file, basically as a list of
 * key-value pairs.
 *
 * get_string:
 * Returns the value of the first key with the given name. If the key isn't found,
 * def is returned.
 *
 * get_int, get_bool, get_float convert the value string to the desired type.
 *
 * get_safe_*:
 * Like above, but throw an exception if the key doesn't exist.
 *
 * get_next_*:
 * Returns the name of the next key of the given name. You can pass name == 0 to
 * retrieve any unused key. Value::used is used to determine which key is next.
 * The value of the key is stored in the second parameter.
 */
class Section {
	friend class Profile;

public:
	struct Value {
		bool   m_used;
		char * m_name;
		char * m_value;

		Value(const char * const nname, const char * const nval);
		Value(const Value &o);
		~Value();

		Value &operator=(const Value &o);

		const char *get_name() const {return m_name;}

		bool is_used() const;
		void mark_used();

		int32_t get_int() const;
		float get_float() const;
		bool get_bool() const;
		const char *get_string() const;
		Point  get_Point () const;
		Coords get_Coords() const;

		void set_string(const char *value);
	};

	typedef std::vector<Value> Value_list;

private:
	Profile  * m_profile;
	bool       m_used;
	char     * m_section_name;
	Value_list m_values;

public:
	Section(Profile * const, const char * const name);
	Section(const Section &);
	~Section();

	Section & operator=(const Section &);

	Value *get_val(const char *name);
	Value *get_next_val(const char *name);
	uint32_t get_num_values() const {return m_values.size();}

	const char *get_name() const;

	bool is_used() const;
	void mark_used();

	void check_used();

	int32_t get_int(const char *name, int32_t def = 0);
	float get_float(const char *name, float def = 0);
	bool get_bool(const char *name, bool def = false);
	const char *get_string(const char *name, const char *def = 0);
	Point  get_Point (const char * const name, const Point  def = Point (0, 0));
	Coords get_Coords(const char * const name, const Coords def = Coords(-1, -1));

	int32_t get_safe_int(const char *name);
	float get_safe_float(const char *name);
	bool get_safe_bool(const char *name);
	const char *get_safe_string(const char *name);
	Coords get_safe_Coords(const char * const name);

	const char *get_next_int(const char *name, int32_t *value);
	const char *get_next_float(const char *name, float *value);
	const char *get_next_bool(const char *name, bool *value);
	const char *get_next_string(const char *name, const char **value);
	const char *get_next_Coords(const char * const name, Coords * const value);

	void set_int
		(char const * name, int32_t      value, bool duplicate = false);
	void set_float
		(char const * name, float        value, bool duplicate = false);
	void set_bool
		(char const * name, bool         value, bool duplicate = false);
	void set_string
		(char const * name, char const * value, bool duplicate = false);
	void set_string
		(char        const * const name,
		 std::string const &       value,
		 bool                const duplicate = false)
	{
		set_string(name, value.c_str(), duplicate);
	}
	void set_Coords
		(char const * name, Coords       value, bool duplicate = false);

	Value *create_val(const char *name, const char *value, bool duplicate = false);
};

/**
 * Parses an .ini-style file into sections and key-value pairs.
 * The destructor prints out warnings if a section or value hasn't been used.
 *
 * get_section:
 * Returns the first section of the given name (or 0 if none exist).
 *
 * get_safe_section:
 * Like get_section, but throws an exception if the section doesn't exist.
 *
 * get_next_section:
 * Returns the next unused section of the given name, or 0 if all sections
 * have been used. name can be 0 to retrieve any remaining sections.
 */
class Profile {
	typedef std::vector<Section> Section_list;

	Section_list m_sections;
	int32_t m_error_level;

public:
	enum {
		err_ignore = 0,
		err_log,
		err_throw
	};

	Profile(int32_t error_level = err_throw);
	Profile(const char* filename, const char *global_section = 0, int32_t error_level = err_throw);
	~Profile();

	void error(const char *fmt, ...) const __attribute__((format(printf, 2, 3)));
	void check_used();

	void read
		(const char * const filename,
		 const char * const global_section = 0,
		 FileSystem & = *g_fs);
	void write
		(const char * const filename,
		 bool used_only = true,
		 FileSystem & = *g_fs);

	Section *get_section(const char *name);
	Section *get_safe_section(const char *name);
	Section *pull_section(const char *name);
	Section *get_next_section(const char *name);

	Section *create_section(const char *name, bool duplicate = false);
};

#endif /* PROFILE_H */
