/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#ifndef PROFILE_H
#define PROFILE_H

#include <cstring>
#include <vector>

#include <boost/noncopyable.hpp>

//TODO: as soon as g_fs is not needed anymore, next include can be changed
//to ..../filesystem.h
#include "io/filesystem/layered_filesystem.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"
#include "point.h"
#include "port.h"


namespace Widelands {
struct Building_Descr;
class Editor_Game_Base;
struct Immovable_Descr;
};

extern struct Profile g_options;
class FileSystem;

/**
 * Represents one section inside the .ini-style file, basically as a list of
 * key-value pairs.
 *
 * get_string:
 * Returns the value of the first key with the given name. If the key isn't
 * found, def is returned.
 *
 * get_int, get_bool, convert the value string to the desired type.
 *
 * get_safe_*:
 * Like above, but throw an exception if the key doesn't exist.
 *
 * get_next_*:
 * Returns the name of the next key of the given name. You can pass name == 0 to
 * retrieve any unused key. Value::used is used to determine which key is next.
 * The value of the key is stored in the second parameter.
 */
struct Section {
	friend struct Profile;

	struct Value {
		bool   m_used;
		char * m_name;
		char * m_value;

		Value(char const * nname, char const * nval);
		Value(const Value &);
		~Value();

		Value & operator= (const Value &);

		char const * get_name() const {return m_name;}

		bool is_used() const;
		void mark_used();

		int32_t get_int() const;
		uint32_t get_natural () const;
		uint32_t get_positive() const;
		bool get_bool() const;
		char const * get_string() const {return m_value;}
		char       * get_string()       {return m_value;}
		Point  get_Point () const;
		Widelands::Coords get_Coords(Widelands::Extent) const;

		void set_string(char const *);
	};

	typedef std::vector<Value> Value_list;

	Section(Profile *, char const * name);
	Section(const Section &);

	Section & operator= (const Section &);

	/// \returns whether a value with the given name exists.
	/// Does not mark the value as used.
	bool has_val(char const * name) const;

	Value * get_val     (char const * name);
	Value * get_next_val(char const * name = nullptr);
	uint32_t get_num_values() const {return m_values.size();}

	char const * get_name() const;
	void set_name(const std::string&);

	bool is_used() const;
	void mark_used();

	void check_used() const;

	int32_t                  get_int
		(char             const * name,
		 int32_t                  def = 0);
	uint32_t                 get_natural
		(char             const * name,
		 uint32_t                 def = 0);
	uint32_t                 get_positive
		(char             const * name,
		 uint32_t                 def = 1);
	bool                     get_bool
		(char             const * name,
		 bool                     def = false);
	const char *             get_string
		(char             const * name,
		 char             const * def = nullptr);
	Point                    get_Point
		(char             const * name,
		 Point                    def = Point (0, 0));
	Widelands::Coords        get_Coords
		(char             const * name, Widelands::Extent,
		 Widelands::Coords        def);
	Widelands::Player_Number get_Player_Number
		(char             const * name,
		 Widelands::Player_Number nr_players,
		 Widelands::Player_Number def = 1);

	int32_t                   get_safe_int
		(const char * name);
	uint32_t                  get_safe_natural
		(char const * name);
	uint32_t                  get_safe_positive
		(char const * name);
	bool                      get_safe_bool
		(const char * name);
	const char *              get_safe_string
		(const char * name);
	const char * get_safe_string(const std::string & name);
	Widelands::Coords         get_safe_Coords
		(const char * name, Widelands::Extent);
	Widelands::Player_Number  get_safe_Player_Number
		(char const * name,
		 Widelands::Player_Number nr_players);
	const Widelands::Immovable_Descr & get_safe_Immovable_Type
		(char const * tribe, char const * name,
		 Widelands::Editor_Game_Base &);
	Widelands::Building_Index get_safe_Building_Index
		(char const * name,
		 Widelands::Editor_Game_Base &, Widelands::Player_Number);
	const Widelands::Building_Descr & get_safe_Building_Type
		(char const * name,
		 Widelands::Editor_Game_Base &, Widelands::Player_Number);

	char const * get_next_bool(char const * name, bool * value);

	void set_int
		(char const *       name, int32_t                   value);
	void set_bool
		(char const * const name, bool                const value)
	{
		set_string(name, value ? "true" : "false");
	}
	void set_string
		(char const *       name, char        const *       value);
	void set_string_duplicate
		(char const *       name, char        const *       value);
	void set_string
		(char const * const name, const std::string &       value)
	{
		set_string(name, value.c_str());
	}
	void set_string_duplicate
		(char const * const name, const std::string &       value)
	{
		set_string_duplicate(name, value.c_str());
	}
	void set_Coords
		(char const * name, Widelands::Coords value);
	void set_Immovable_Type
		(char const * tribe, char const * name,
		 const Widelands::Immovable_Descr &);
	void set_Building_Index
		(char const * name,
		 Widelands::Building_Index value,
		 Widelands::Editor_Game_Base &, Widelands::Player_Number);

	/// If a Value with this name already exists, update it with the given
	/// value. Otherwise create a new Value with the given name and value.
	Value & create_val          (char const * name, char const * value);

	/// Create a new Value with the given name and value.
	Value & create_val_duplicate(char const * name, char const * value);

private:
	Profile  * m_profile;
	bool       m_used;
	std::string m_section_name;
	Value_list m_values;
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
struct Profile : boost::noncopyable {
	enum {
		err_ignore = 0,
		err_log,
		err_throw
	};

	Profile(int32_t error_level = err_throw);
	Profile
		(char const * filename,
		 char const * global_section = nullptr,
		 int32_t      error_level    = err_throw);
	Profile
		(char const * filename,
		 char const * global_section,
		 const std::string & textdomain,
		 int32_t      error_level    = err_throw);

	void error(char const *, ...) const __attribute__((format(printf, 2, 3)));
	void check_used() const;

	void read
		(const char * const filename,
		 const char * const global_section = nullptr,
		 FileSystem & = *g_fs);
	void write
		(const char * const filename,
		 bool used_only = true,
		 FileSystem & = *g_fs);

	Section * get_section     (const std::string & name);
	Section & get_safe_section(const std::string & name);
	Section & pull_section    (char const * name);
	Section * get_next_section(char const * name = nullptr);

	/// If a section with the given name already exists, return a reference to
	/// it. Otherwise create a new section with the given name and return a
	/// reference to it.
	Section & create_section          (char const * name);

	/// Create a new section with the given name and return a reference to it.
	Section & create_section_duplicate(char const * name);

private:
	std::string m_filename;
	typedef std::vector<Section> Section_list;
	Section_list m_sections;
	int32_t m_error_level;
};

#endif
