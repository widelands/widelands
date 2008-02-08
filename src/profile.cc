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

#include "build_id.h"
#include "fileread.h"
#include "filewrite.h"
#include "i18n.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "wexception.h"
#include "wlapplication.h"

#include "log.h"

#include <cstdarg>
#include <cctype>
#include <limits>
#include <string>

#include <stdio.h>

#define TRUE_WORDS 4
const char* trueWords[TRUE_WORDS] =
{
	"true",
	"yes",
	"on",
	"1"
};

#define FALSE_WORDS 4
const char* falseWords[FALSE_WORDS] =
{
	"false",
	"no",
	"off",
	"0"
};

Profile g_options(Profile::err_log);

/*
==============================================================================

Section::Value

==============================================================================
*/

Section::Value::Value(const char * const nname, const char * const nval) :
m_used(false), m_name(strdup(nname)), m_value(strdup(nval)) {}

Section::Value::Value(const Section::Value &o) :
m_used(o.m_used), m_name(strdup(o.m_name)), m_value(strdup(o.m_value)) {}

Section::Value::~Value()
{
	free(m_name);
	free(m_value);
}

Section::Value &Section::Value::operator=(const Section::Value &o)
{
	if (this != &o) {
		free(m_name);
		free(m_value);
		m_used = o.m_used;
		m_name = strdup(o.m_name);
		m_value = strdup(o.m_value);
	}
	return *this;
}

bool Section::Value::is_used() const
{
	return m_used;
}

void Section::Value::mark_used()
{
	m_used = true;
}

int32_t Section::Value::get_int() const
{
	char *endp;
	int32_t i;

	i = strtol(m_value, &endp, 0);

	if (*endp)
		throw wexception("%s: '%s' is not an integer", get_name(), m_value);

	return i;
}


bool Section::Value::get_bool() const
{
	for (int32_t i = 0; i < TRUE_WORDS; ++i)
		if (!strcasecmp(m_value, trueWords[i]))
			return true;
	for (int32_t i = 0; i < FALSE_WORDS; ++i)
		if (!strcasecmp(m_value, falseWords[i]))
			return false;

	throw wexception("%s: '%s' is not a boolean value", get_name(), m_value);
}

const char *Section::Value::get_string() const
{
	return m_value;
}

Point Section::Value::get_Point() const
{
	char * endp = m_value;
	long int const x = strtol(endp, &endp, 0);
	long int const y = strtol(endp, &endp, 0);
	if (*endp) throw wexception("%s: '%s' is not a Point", get_name(), m_value);

	return Point(x, y);
}

Widelands::Coords Section::Value::get_Coords
(Widelands::Extent const extent) const
{
	char * endp = m_value;
	long int const x = strtol(endp, &endp, 0);
	long int const y = strtol(endp, &endp, 0);
	if (x <  0 or extent.w <= x or y < 0 or extent.h <= y or *endp)
		throw wexception
			("%s: \"%s\" is not a Coords on a map with size (%u, %u)",
			 get_name(), m_value, extent.w, extent.h);

	return Widelands::Coords(x, y);
}

void Section::Value::set_string(const char *value)
{
	free(m_value);
	m_value = strdup(value);
}


/*
==============================================================================

Section

==============================================================================
*/

const char *Section::get_name() const {
	return m_section_name;
}

Section::Section(Profile * const prof, const char * const name) :
m_profile(prof), m_used(false), m_section_name(strdup(name)) {}

Section::Section(const Section & o) :
m_profile     (o.m_profile),
m_used        (o.m_used),
m_section_name(strdup(o.m_section_name)),
m_values      (o.m_values)
{assert(this != &o);}

Section::~Section() {free(m_section_name);}

Section &Section::operator=(const Section & o) {
	if (this != &o) {
		free(m_section_name);

		m_profile      = o.m_profile;
		m_used         = o.m_used;
		m_section_name = strdup(o.m_section_name);
		m_values       = o.m_values;
	}

	return *this;
}

/** Section::is_used()
 *
 */
bool Section::is_used() const
{
	return m_used;
}

/** Section::mark_used()
 *
 */
void Section::mark_used()
{
	m_used = true;
}

/** Section::check_used()
 *
 * Print a warning for every unused value.
 */
void Section::check_used()
{
	const Value_list::const_iterator values_end = m_values.end();
	for
		(Value_list::const_iterator it = m_values.begin(); it != values_end; ++it)
		if (not it->is_used())
			m_profile->error
				("Section [%s], key '%s' not used (did you spell the name "
				 "correctly?)",
				 get_name(),
				 it->get_name());
}

/** Section::get_val(const char *name)
 *
 * Returns the Value associated with the given keyname.
 *
 * Args: name  name of the key
 *
 * Returns: Pointer to the Value struct; 0 if the key doesn't exist.
 */
Section::Value *Section::get_val(const char *name)
{
	for (Value_list::iterator v = m_values.begin(); v != m_values.end(); ++v) {
		if (!strcasecmp(v->get_name(), name)) {
			v->mark_used();
			return &*v;
		}
	}

	return 0;
}

/** Section::get_next_val(const char *name)
 *
 * Returns the first unused value associated with the given keyname.
 *
 * Args: name  name of the key; can be 0 to find any key
 *
 * Returns: Pointer to the Value struct; 0 if no more key-value pairs are found
 */
Section::Value *Section::get_next_val(const char *name)
{
	for (Value_list::iterator v = m_values.begin(); v != m_values.end(); ++v) {
		if (v->is_used())
			continue;
		if (!name || !strcasecmp(v->get_name(), name)) {
			v->mark_used();
			return &*v;
		}
	}

	return 0;
}

/** Section::create_val(const char *name, const char *value, bool duplicate = false)
 *
 * Set the given key. If duplicate is false, an old key with the given name is
 * replaced if possible.
 *
 * Unlike the set_*() class functions, it doesn't mark the key as used.
 */
Section::Value *Section::create_val(const char *name, const char *value, bool duplicate)
{
	if (!duplicate) {
		for
			(Value_list::iterator old = m_values.begin();
			 old != m_values.end();
			 ++old)
		{
			if (!strcasecmp(old->get_name(), name)) {
				old->set_string(value);
				return &*old;
			}
		}
	}

	Value v(name, value);
	m_values.push_back(v);
	return &m_values.back();
}

/** Section::get_safe_int(const char *name)
 *
 * Return the integer value of the given key or throw an exception if a
 * problem arises.
 */
int32_t Section::get_safe_int(const char *name)
{
	Value *v = get_val(name);
	if (!v)
		throw wexception("[%s]: missing integer key '%s'", get_name(), name);
	return v->get_int();
}


/** Section::get_safe_bool(const char *name)
 *
 * Return the boolean value of the given key or throw an exception if a
 * problem arises.
 */
bool Section::get_safe_bool(const char *name)
{
	Value *v = get_val(name);
	if (!v)
		throw wexception("[%s]: missing boolean key '%s'", get_name(), name);
	return v->get_bool();
}

/** Section::get_safe_string(const char *name)
 *
 * Return the key value as a plain string or throw an exception if the key
 * doesn't exist
 */
const char *Section::get_safe_string(const char *name)
{
	Value *v = get_val(name);
	if (!v)
		throw wexception("[%s]: missing key '%s'", get_name(), name);
	return v->get_string();
}

/** Section::get_safe_Coords(const char * const name)
 *
 * Return the key value as a Coords or throw an exception if the key
 * doesn't exist
 */
Widelands::Coords Section::get_safe_Coords
(const char * const name, Widelands::Extent const extent)
{
	if (const Value * const v = get_val(name))
		return v->get_Coords(extent);
	else
		throw wexception("[%s]: missing key '%s'", get_name(), name);
}


Widelands::Immovable_Descr const & Section::get_safe_Immovable_Type
(char const * tribe, char const * name,
 Widelands::Editor_Game_Base & egbase)
{
	char const * const immname = get_safe_string(name);
	if (char const * const tribename = get_string(tribe, 0)) {
		Widelands::Tribe_Descr const & tridescr =
			egbase.manually_load_tribe(tribename);
		if
			(Widelands::Immovable_Descr const * const result =
			 tridescr.get_immovable_descr(tridescr.get_immovable_index(immname)))
			return *result;
		else
			throw wexception
				("tribe %s does not define immovable type \"%s\"",
				 tribename,        immname);
	} else {
		Widelands::World       const & world    =
			egbase.map().world();
		if
			(Widelands::Immovable_Descr const * const result =
			 world   .get_immovable_descr(world   .get_immovable_index(immname)))
			return *result;
		else
			throw wexception
				("world %s does not define immovable type \"%s\"",
				 world.get_name(), immname);
	}
}



Widelands::Building_Index Section::get_safe_Building_Index
(const char * const name,
 Widelands::Editor_Game_Base &       egbase,
 Widelands::Player_Number      const player)
{
	Widelands::Tribe_Descr const & tribe =
		egbase.manually_load_tribe
		(egbase.map().get_scenario_player_tribe(player).c_str());
	char const * const b = get_safe_string(name);
	if (Widelands::Building_Index const idx = tribe.get_building_index(b))
		return idx;
	else
		throw wexception
			("building type \"%s\" does not exist in player %u's tribe %s",
			 b, player, tribe.name().c_str());
}


Widelands::Building_Descr const & Section::get_safe_Building_Type
(const char * const name,
 Widelands::Editor_Game_Base &       egbase,
 Widelands::Player_Number      const player)
{
	Widelands::Tribe_Descr const & tribe =
		egbase.manually_load_tribe
		(egbase.map().get_scenario_player_tribe(player).c_str());
	char const * const b = get_safe_string(name);
	if
		(Widelands::Building_Descr const * const result =
		 tribe.get_building_descr(tribe.building_index(b)))
		return *result;
	else
		throw wexception
			("building type \"%s\" does not exist in player %u's tribe %s",
			 b, player, tribe.name().c_str());
}


/** Section::get_int(const char *name, int32_t def)
 *
 * Returns the integer value of the given key. Falls back to a default value
 * if the key is not found.
 *
 * Args: name  name of the key
 *       def   fallback value
 *
 * Returns: the integer value of the key
 */
int32_t Section::get_int(const char *name, int32_t def)
{
	Value *v = get_val(name);
	if (!v)
		return def;

	try {
		return v->get_int();
	} catch (std::exception &e) {
		m_profile->error("%s", e.what());
		return def;
	}
}


/** Section::get_bool(const char *name, bool def)
 *
 * Returns the boolean value of the given key. Falls back to a default value
 * if the key is not found.
 *
 * Args: name  name of the key
 *       def   fallback value
 *
 * Returns: the boolean value of the key
 */
bool Section::get_bool(const char *name, bool def)
{
	Value *v = get_val(name);
	if (!v)
		return def;

	try {
		return v->get_bool();
	} catch (std::exception &e) {
		m_profile->error("%s", e.what());
		return def;
	}
}

/** Section::get_string(const char *name, const char *def)
 *
 * Returns the value of the given key. Falls back to a default value if the
 * key is not found.
 *
 * Args: name  name of the key
 *       def   fallback value
 *
 * Returns: the string associated with the key; never returns 0 if the key
 *          has been found
 */
const char *Section::get_string(const char *name, const char *def)
{
	Value const * const v = get_val(name);
	return v ? v->get_string() : def;
}

Point Section::get_Point(const char * const name, const Point def)
{const Value * const v = get_val(name); return v ? v->get_Point() : def;}


Widelands::Player_Number Section::get_Player_Number
(const char * const name,
 Widelands::Player_Number const nr_players,
 Widelands::Player_Number const def)
{
	int32_t const value = get_int(name, def);
	if (1 <= value and value <= nr_players)
		return value;
	else
		throw wexception
			("player number is %i but there are only %u players on the map",
			 value, nr_players);
}


/** Section::get_next_int(const char *name, int32_t *value)
 *
 * Retrieve the next unused key with the given name as an integer.
 *
 * Args: name   name of the key, can be 0 to find all unused keys
 *       value  value of the key is stored here
 *
 * Returns: the name of the key, or 0 if none has been found
 */
const char *Section::get_next_int(const char *name, int32_t *value)
{
	if (Value const * const v = get_next_val(name)) {
		if (value)
			*value = v->get_int();
		return v->get_name();
	} else
		return 0;
}


/** Section::get_next_bool(const char *name, int32_t *value)
 *
 * Retrieve the next unused key with the given name as a boolean value.
 *
 * Args: name   name of the key, can be 0 to find all unused keys
 *       value  value of the key is stored here
 *
 * Returns: the name of the key, or 0 if none has been found
 */
const char *Section::get_next_bool(const char *name, bool *value)
{
	Value *v = get_next_val(name);
	if (!v)
		return 0;

	if (value)
      *value = v->get_bool();
	return v->get_name();
}

/** Section::get_next_string(const char *name, int32_t *value)
 *
 * Retrieve the next unused key with the given name.
 *
 * \param name   name of the key, can be 0 to find all unused keys
 * \param value  value of the key is stored here
 *
 * \return the name of the key, or 0 if none has been found
 */
const char *Section::get_next_string(const char *name, const char **value)
{
	Value *v = get_next_val(name);
	if (!v)
		return 0;

	if (value)
      *value = v->get_string();
	return v->get_name();
}


/** Section::set_int(const char *name, int32_t value, bool duplicate = false)
 *
 * Modifies/Creates the given key.
 * If duplicate is true, a duplicate key will be created if the key already
 * exists.
 */
void Section::set_int(const char *name, int32_t value, bool duplicate)
{
	char buffer[32];

	snprintf(buffer, sizeof(buffer), "%i", value);
	create_val(name, buffer, duplicate)->mark_used();
}


/** Section::set_bool(const char *name, bool value, bool duplicate = false)
 *
 * Modifies/Creates the given key.
 * If duplicate is true, a duplicate key will be created if the key already
 * exists.
 */
void Section::set_bool(const char *name, bool value, bool duplicate)
{
	const char *string;

	string = value ? "true" : "false";
	create_val(name, string, duplicate)->mark_used();
}

/** Section::set_string(const char *name, const char *string, bool duplicate = false)
 *
 * Modifies/Creates the given key.
 * If duplicate is true, a duplicate key will be created if the key already
 * exists.
 */
void Section::set_string(const char *name, const char *string, bool duplicate)
{
	create_val(name, string, duplicate)->mark_used();
}

/** Section::set_Coords(const char * const name, const Coords value, const bool duplicate = false)
 *
 * Modifies/Creates the given key.
 * If duplicate is true, a duplicate key will be created if the key already
 * exists.
 */
void Section::set_Coords
(const char * const name, const Widelands::Coords value, const bool duplicate)
{
	char buffer[64];

	snprintf(buffer, sizeof(buffer), "%i %i", value.x, value.y);
	create_val(name, buffer, duplicate)->mark_used();
}


void Section::set_Immovable_Type
(char const * const tribe, char const * const name,
 Widelands::Immovable_Descr const & descr)
{
	if (Widelands::Tribe_Descr const * const tridescr = descr.get_owner_tribe())
		create_val(tribe, tridescr->name().c_str(), false)->mark_used();
	create_val   (name,     descr. name().c_str(), false)->mark_used();
}


void Section::set_Building_Index
(char const * const name,
 Widelands::Building_Index           const value,
 Widelands::Editor_Game_Base const &       egbase,
 Widelands::Player_Number            const player,
 bool                                const duplicate)
{
	create_val
		(name,
		 egbase.player(player).tribe().get_building_descr(value)->name().c_str(),
		 duplicate)
		->mark_used();
}


void Section::set_Building_Type
(char const * const name,
 Widelands::Building_Descr const &       value,
 bool                              const duplicate)
{
	create_val(name, value.name().c_str(), duplicate)->mark_used();
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
Profile::Profile(int32_t error_level)
{
	m_error_level = error_level;
}

/** Profile::Profile(const char* filename, const char *global_section = 0, int32_t error_level = err_throw)
 *
 * Parses an ini-style file into sections and key-value pairs.
 * If global_section is not null, keys outside of sections are put into a section
 * of that name.
 */
Profile::Profile(const char* filename, const char *global_section, int32_t error_level)
{
	m_error_level = error_level;
	read(filename, global_section);
}

/** Profile::~Profile()
 *
 * Free allocated resources
 */
Profile::~Profile()
{
}

/** Profile::error(const char *fmt, ...)
 *
 * Output an error message.
 * Depending on the error level, it is thrown as a wexception, logged or ignored.
 */
void Profile::error(const char *fmt, ...) const
{
	if (m_error_level == err_ignore)
		return;

	char buffer[256];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (m_error_level == err_log) log("%s\n", buffer);
	else throw wexception("%s", buffer);
}

/** Profile::check_used()
 *
 * Signal an error if a section or key hasn't been used.
 */
void Profile::check_used()
{
	for
		(Section_list::iterator s = m_sections.begin();
		 s != m_sections.end();
		 ++s)
	{
		if (!s->is_used())
			error("Section [%s] not used (did you spell the name correctly?)", s->get_name());
		else
			s->check_used();
	}
}

/** Profile::get_section(const char *name)
 *
 * Retrieve the first section of the given name and mark it used.
 *
 * Args: name  name of the section
 *
 * Returns: pointer to the section (or 0 if the section doesn't exist)
 */
Section *Profile::get_section(const char *name)
{
	for
		(Section_list::iterator s = m_sections.begin();
		 s != m_sections.end();
		 ++s)
	{
		if (!strcasecmp(s->get_name(), name)) {
			s->mark_used();
			return &*s;
		}
	}

	return 0;
}

/** Profile::get_safe_section(const char *name)
 *
 * Safely get a section of the given name.
 * If the section doesn't exist, an exception is thrown.
 */
Section *Profile::get_safe_section(const char *name)
{
	Section *s = get_section(name);
	if (!s)
		throw wexception("Section [%s] not found", name);
	return s;
}

/** Profile::pull_section(const char *name)
 *
 * Safely get a section of the given name.
 * If the section doesn't exist, it is created.
 * Similar to create_section(), but the section is marked as used.
 */
Section *Profile::pull_section(const char *name)
{
	Section *s = create_section(name);
	s->mark_used();
	return s;
}

/** Profile::get_next_section(const char *name)
 *
 * Retrieve the next unused section of the given name and mark it used.
 *
 * Args: name  name of the section; can be 0 to retrieve any unused section
 *
 * Returns: pointer to the section (or 0 if the section doesn't exist)
 */
Section *Profile::get_next_section(const char *name)
{
	for
		(Section_list::iterator s = m_sections.begin();
		 s != m_sections.end();
		 ++s)
	{
		if (s->is_used())
			continue;
		if (!name || !strcasecmp(s->get_name(), name)) {
			s->mark_used();
			return &*s;
		}
	}

	return 0;
}

/** Profile::create_section(const char *name, bool duplicate = false)
 *
 * Create a section of the given name.
 * If duplicate is true, a duplicate section may be created. Otherwise, a
 * pointer to an existing section is returned.
 */
Section *Profile::create_section(const char *name, bool duplicate)
{
	if (!duplicate) {
		for
			(Section_list::iterator s = m_sections.begin();
			 s != m_sections.end();
			 ++s)
		{
			if (!strcasecmp(s->get_name(), name))
				return &*s;
		}
	}

	m_sections.push_back(Section(this, name));
	return &m_sections.back();
}

inline char *skipwhite(char *p)
{
	while (*p && isspace(*p))
		++p;
	return p;
}

inline void rtrim(char *str)
{
	char *p;
	for (p = strchr(str, 0); p > str; --p) {
		if (!isspace(*(p-1)))
			break;
	}
	*p = 0;
}

inline void killcomments(char *p)
{
	while (*p) {
		if (p[0] == '#' || (p[0] == '/' && p[1] == '/')) {
			p[0] = 0;
			break;
		}
		++p;
	}
}

inline char *setEndAt(char *str, char c)
{
	char* s = strchr(str, c);
	if (s)
		s[0] = 0;
	return str;
}

/**
 * Parses an ini-style file into sections and key values. If a section or
 * key name occurs multiple times, an additional entry is created.
 *
 * Args: filename  name of the source file
 */
void Profile::read
(const char * const filename,
 const char * const global_section,
 FileSystem & fs)
{
	try {
		FileRead fr;
		fr.Open(fs, filename);

		// line can become quite big. But this should be enough
		//  Previously this was allocated with new and then simply leaked.
		//  We (Valgrind and I) did not like that.
		char buffer[1024 * 30];
		const char * const buf_end = buffer + sizeof(buffer);
		char * line = buffer;
		char *p = 0;
		uint32_t linenr = 0;
		Section *s = 0;

		bool reading_multiline = 0;
		std::string data;
		std::string key;
		bool translate_line = false;
		while (fr.ReadLine(line, buf_end)) {
			++linenr;

			if (!reading_multiline)
				p = line;

			p = skipwhite(p);
			if (!p[0] || p[0] == '#' || (p[0] == '/' && p[1] == '/'))
				continue;

			if (p[0] == '[') {
				++p;
				setEndAt(p, ']');
				s = create_section(p, true); // may create duplicate
			} else {
				char* tail = 0;
				translate_line = false;
				if (reading_multiline) {
					// Note: comments are killed by walking backwards into the string
					rtrim(p);
					while (*line != '\'' && *line != '"') {
						if (*line == 0)
							throw wexception
								("line %i: runaway multiline string", linenr);
						if (*line == '_')
							translate_line = true;
						++line;
					}

					// skip " or '
					++line;

					char *eot = line+strlen(line)-1;
					while (*eot != '"' && *eot != '\'') {
						*eot = 0;
						--eot;
					}
					// NOTE: we leave the last '"' and do not remove them
					tail = line;
				} else {
					tail = strchr(p, '=');
					if (!tail)
						throw wexception("line %i: invalid syntax: %s", linenr, line);
					*tail++ = 0;
					key = p;
					if (*tail == '_') {
						tail+= 1; // skip =_, which is only used for translations
						translate_line = true;
					}
					tail = skipwhite(tail);
					killcomments(tail);
					rtrim(tail);
					rtrim(p);

					// first, check for multiline string
					if
						(strlen(tail) >= 2
						 and
						 ((tail[0] == '\'' or tail[0] == '\"')
						  and
						  (tail[1] == '\'' or tail[1] == '\"')))
					{
						reading_multiline = true;
						tail += 2;
					}

					// then remove surrounding '' or ""
					if (tail[0] == '\'' || tail[0] == '\"') {
						++tail;
					}
				}
				if (tail) {
					char *eot = tail+strlen(tail)-1;
					if (*eot == '\'' || *eot == '\"') {
						*eot = 0;
						if (strlen(tail)) {
							char *eot2 = tail+strlen(tail)-1;
							if (*eot2 == '\'' || *eot2 == '\"') {
								reading_multiline = false;
								*eot2 = 0;
							}
						}
					}

					// ready to insert
					if (!s) {
						if (global_section)
							s = create_section(global_section, true);
						else
							throw wexception("line %i: key %s outside section", linenr, p);
					}

					if (translate_line && strlen(tail))
						data += i18n::translate(tail);
					else
						data += tail;
					if (s && ! reading_multiline) { // error() may or may not throw
						s->create_val(key.c_str(), data.c_str(), true); // may create duplicate
						data = "";
					}
				} else {
					throw wexception("line %i: syntax error", linenr);
				}
			}
		}
	}
	catch (FileNotFound_error &e) {
		//It's no problem if the config file does not exist. (It'll get
		//written on exit anyway)
		log("There's no configuration file, using default values.\n");
	}
	catch (std::exception &e) {
		error("%s: %s", filename, e.what());
	}
}

/**
 * Writes all sections out to the given file.
 * If used_only is true, only used sections and keys are written to the file.
 */
void Profile::write
(const char * const filename, const bool used_only, FileSystem & fs)
{
	FileWrite fw;

	fw.Printf("# Automatically created by Widelands " BUILD_ID "\n");

	for
		(Section_list::iterator s = m_sections.begin();
		 s != m_sections.end();
		 ++s)
	{
		if (used_only && !s->is_used())
			continue;

		fw.Printf("\n[%s]\n", s->get_name());

		for
			(Section::Value_list::iterator v = s->m_values.begin();
			 v != s->m_values.end();
			 ++v)
		{
			if (used_only && !v->is_used())
				continue;

			const char* str = v->get_string();

			if (strlen(str)>=1) {
				uint32_t spaces = strlen(v->get_name());
				bool multiline = false;

				for (uint32_t i = 0; i < strlen(str); ++i) {
					if (str[i] == '\n') {
						multiline = true;
					}
				}

				// Try to avoid _every_ possible way of
				// getting inconsistent data
				std::string tempstr("");

				if (multiline)
					// Show WL that a multilined text starts
					tempstr += "\"";

				for (uint32_t i = 0; i < strlen(str); i++) {//  FIXME
					// No speach marks - they would break the format
					if (str[i] == '"')
						tempstr += "''";
					else {
						// Convert the newlines to WL format.
						if (str[i] == '\n') {
							tempstr += " \"\n";
							for (uint32_t j = 0; j <= spaces; j++)//  FIXME
								tempstr += " ";
							tempstr += " \"";
						} else
							tempstr += str[i];
					}
				}

				if (multiline)
					// End of multilined text.
					tempstr += "\"";

				fw.Printf("%s=\"%s\"\n", v->get_name(), tempstr.c_str());
			} else
				fw.Printf("%s=\n", v->get_name());
		}
	}

	fw.Write(fs, filename);
}

