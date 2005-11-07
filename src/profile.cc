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

#include <cstdarg>
#include <cctype>
#include <stdio.h>
#include <string>

#include "constants.h"
#include "error.h"
#include "filesystem.h"
#include "profile.h"
#include "system.h"
#include "wexception.h"
#include "system.h"

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

/*
==============================================================================

Section::Value

==============================================================================
*/

Section::Value::Value(const char *nname, const char *nval)
{
	m_used = false;
	m_name = strdup(nname);
	m_value = strdup(nval);
}

Section::Value::Value(const Section::Value &o)
{
	m_used = o.m_used;
	m_name = strdup(o.m_name);
	m_value = strdup(o.m_value);
}

Section::Value::~Value()
{
	free(m_name);
	free(m_value);
}

Section::Value &Section::Value::operator=(const Section::Value &o)
{
	free(m_name);
	free(m_value);
	m_used = o.m_used;
	m_name = strdup(o.m_name);
	m_value = strdup(o.m_value);

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

int Section::Value::get_int() const
{
	char *endp;
	int i;

	i = strtol(m_value, &endp, 0);

	if (*endp)
		throw wexception("%s: '%s' is not an integer", get_name(), m_value);

	return i;
}

float Section::Value::get_float() const
{
	char *endp;
	float f;

	f = strtod(m_value, &endp);

	if (*endp)
		throw wexception("%s: '%s' is not a float", get_name(), m_value);

	return f;
}

bool Section::Value::get_bool() const
{
	int i;
	for (i = 0; i < TRUE_WORDS; i++)
		if (!strcasecmp(m_value, trueWords[i]))
			return true;
	for (i = 0; i < FALSE_WORDS; i++)
		if (!strcasecmp(m_value, falseWords[i]))
			return false;

	throw wexception("%s: '%s' is not a boolean value", get_name(), m_value);
}

const char *Section::Value::get_string() const
{
	return m_value;
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

Section::Section(Profile *prof, const char *name)
{
	m_profile = prof;
	m_used = false;
	m_section_name = strdup(name);
}

Section::Section(const Section &o)
{
	m_section_name = 0;
	*this = o;
}

Section::~Section()
{
	if (m_section_name)
		free(m_section_name);
}

Section &Section::operator=(const Section &o)
{
	if (m_section_name)
		free(m_section_name);

	m_profile = o.m_profile;
	m_used = o.m_used;
	m_section_name = strdup(o.m_section_name);
	m_values = o.m_values;

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
	for(Value_list::iterator v = m_values.begin(); v != m_values.end(); v++) {
		if (!v->is_used())
			m_profile->error("Section [%s], key '%s' not used (did you spell the name correctly?)",
				              get_name(), v->get_name());
	}
}

/** Section::get_val(const char *name)
 *
 * Returns the Value associated with the given keyname.
 *
 * Args: name	name of the key
 *
 * Returns: Pointer to the Value struct; 0 if the key doesn't exist.
 */
Section::Value *Section::get_val(const char *name)
{
	for(Value_list::iterator v = m_values.begin(); v != m_values.end(); v++) {
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
 * Args: name	name of the key; can be 0 to find any key
 *
 * Returns: Pointer to the Value struct; 0 if no more key-value pairs are found
 */
Section::Value *Section::get_next_val(const char *name)
{
	for(Value_list::iterator v = m_values.begin(); v != m_values.end(); v++) {
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
		for(Value_list::iterator old = m_values.begin(); old != m_values.end(); old++) {
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
int Section::get_safe_int(const char *name)
{
	Value *v = get_val(name);
	if (!v)
		throw wexception("[%s]: missing integer key '%s'", get_name(), name);
	return v->get_int();
}

/** Section::get_safe_float(const char *name)
 *
 * Return the float value of the given key or throw an exception if a
 * problem arises.
 */
float Section::get_safe_float(const char *name)
{
	Value *v = get_val(name);
	if (!v)
		throw wexception("[%s]: missing float key '%s'", get_name(), name);
	return v->get_float();
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

/** Section::get_safe_translated_string
 *
 * Returns the value of the given key, translated in the set local
 * using gettext.
 *
 * Note: the textdomain must be set through the system
 */
const char* Section::get_safe_translated_string( const char* name ) {
   return Sys_Translate( get_safe_string( name ) );
}
 
/** Section::get_int(const char *name, int def)
 *
 * Returns the integer value of the given key. Falls back to a default value
 * if the key is not found.
 *
 * Args: name	name of the key
 *       def	fallback value
 *
 * Returns: the integer value of the key
 */
int Section::get_int(const char *name, int def)
{
	Value *v = get_val(name);
	if (!v)
		return def;

	try {
		return v->get_int();
	} catch(std::exception &e) {
		m_profile->error("%s", e.what());
		return def;
	}
}

/** Section::get_float(const char *name, float def)
 *
 * Returns the float value of the given key. Falls back to a default value
 * if the key is not found.
 */
float Section::get_float(const char *name, float def)
{
	Value *v = get_val(name);
	if (!v)
		return def;

	try {
		return v->get_float();
	} catch(std::exception &e) {
		m_profile->error("%s", e.what());
		return def;
	}
}

/** Section::get_bool(const char *name, bool def)
 *
 * Returns the boolean value of the given key. Falls back to a default value
 * if the key is not found.
 *
 * Args: name	name of the key
 *       def	fallback value
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
	} catch(std::exception &e) {
		m_profile->error("%s", e.what());
		return def;
	}
}

/** Section::get_string(const char *name, const char *def)
 *
 * Returns the value of the given key. Falls back to a default value if the
 * key is not found.
 *
 * Args: name	name of the key
 *       def	fallback value
 *
 * Returns: the string associated with the key; never returns 0 if the key
 *          has been found
 */
const char *Section::get_string(const char *name, const char *def)
{
	Value *v = get_val(name);
	if (!v)
		return def;
	return v->get_string();
}

/** Section::get_translated_string
 *
 * Returns the value of the given key, translated in the set local
 * using gettext.
 *
 * Note: the textdomain must be set through the system
 */
const char* Section::get_translated_string(const char* name, const char* def ) {
   const char* retval = get_string( name, def );
   if( !retval ) 
      return 0;

   return Sys_Translate( retval );
}
   
/** Section::get_next_int(const char *name, int *value)
 *
 * Retrieve the next unused key with the given name as an integer.
 *
 * Args: name	name of the key, can be 0 to find all unused keys
 *       value	value of the key is stored here
 *
 * Returns: the name of the key, or 0 if none has been found
 */
const char *Section::get_next_int(const char *name, int *value)
{
	Value *v = get_next_val(name);
	if (!v)
		return 0;

	*value = v->get_int();
	return v->get_name();
}

/** Section::get_next_float(const char *name, float *value)
 *
 * Retrieve the next unused key with the given name as a float.
 */
const char *Section::get_next_float(const char *name, float *value)
{
	Value *v = get_next_val(name);
	if (!v)
		return 0;

	*value = v->get_float();
	return v->get_name();
}

/** Section::get_next_bool(const char *name, int *value)
 *
 * Retrieve the next unused key with the given name as a boolean value.
 *
 * Args: name	name of the key, can be 0 to find all unused keys
 *       value	value of the key is stored here
 *
 * Returns: the name of the key, or 0 if none has been found
 */
const char *Section::get_next_bool(const char *name, bool *value)
{
	Value *v = get_next_val(name);
	if (!v)
		return 0;

	*value = v->get_bool();
	return v->get_name();
}

/** Section::get_next_string(const char *name, int *value)
 *
 * Retrieve the next unused key with the given name.
 *
 * Args: name	name of the key, can be 0 to find all unused keys
 *       value	value of the key is stored here
 *
 * Returns: the name of the key, or 0 if none has been found
 */
const char *Section::get_next_string(const char *name, const char **value)
{
	Value *v = get_next_val(name);
	if (!v)
		return 0;

	*value = v->get_string();
	return v->get_name();
}

/** Section::set_int(const char *name, int value, bool duplicate = false)
 *
 * Modifies/Creates the given key.
 * If duplicate is true, a duplicate key will be created if the key already
 * exists.
 */
void Section::set_int(const char *name, int value, bool duplicate)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%i", value);
	create_val(name, buf, duplicate)->mark_used();
}

/** Section::set_float(const char *name, float value, bool duplicate = false)
 *
 * Modifies/Creates the given key.
 * If duplicate is true, a duplicate key will be created if the key already
 * exists.
 */
void Section::set_float(const char *name, float value, bool duplicate)
{
	char buf[64];

	snprintf(buf, sizeof(buf), "%f", value);
	create_val(name, buf, duplicate)->mark_used();
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


/*
==============================================================================

Profile

==============================================================================
*/

/** Profile::Profile(int error_level = err_throw)
 *
 * Create an empty profile
 */
Profile::Profile(int error_level)
{
	m_error_level = error_level;
}

/** Profile::Profile(const char* filename, const char *global_section = 0, int error_level = err_throw)
 *
 * Parses an ini-style file into sections and key-value pairs.
 * If global_section is not null, keys outside of sections are put into a section
 * of that name.
 */
Profile::Profile(const char* filename, const char *global_section, int error_level)
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

	char buf[256];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	if (m_error_level == err_log)
		log("%s\n", buf);
	else
		throw wexception("%s", buf);
}

/** Profile::check_used()
 *
 * Signal an error if a section or key hasn't been used.
 */
void Profile::check_used()
{
	for(Section_list::iterator s = m_sections.begin(); s != m_sections.end(); s++) {
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
 * Args: name	name of the section
 *
 * Returns: pointer to the section (or 0 if the section doesn't exist)
 */
Section *Profile::get_section(const char *name)
{
	for(Section_list::iterator s = m_sections.begin(); s != m_sections.end(); s++) {
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
 * Args: name	name of the section; can be 0 to retrieve any unused section
 *
 * Returns: pointer to the section (or 0 if the section doesn't exist)
 */
Section *Profile::get_next_section(const char *name)
{
	for(Section_list::iterator s = m_sections.begin(); s != m_sections.end(); s++) {
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
		for(Section_list::iterator s = m_sections.begin(); s != m_sections.end(); s++) {
			if (!strcasecmp(s->get_name(), name))
				return &*s;
		}
	}

	m_sections.push_back(Section(this, name));
	return &m_sections.back();
}

inline char *skipwhite(char *p)
{
	while(*p && isspace(*p))
		p++;
	return p;
}

inline void rtrim(char *str)
{
	char *p;
	for(p = strchr(str, 0); p > str; p--) {
		if (!isspace(*(p-1)))
			break;
	}
	*p = 0;
}

inline void killcomments(char *p)
{
	while(*p) {
		if (p[0] == '#' || (p[0] == '/' && p[1] == '/')) {
			p[0] = 0;
			break;
		}
		p++;
	}
}

inline char *setEndAt(char *str, char c)
{
	char* s = strchr(str, c);
	if (s)
		s[0] = 0;
	return str;
}

/** Profile::read(const char *filename, const char *global_section = 0)
 *
 * Parses an ini-style file into sections and key values. If a section or
 * key name occurs multiple times, an additional entry is created.
 *
 * Args: filename	name of the source file
 */
void Profile::read(const char *filename, const char *global_section, FileSystem* fs)
{
	try
	{
		FileRead fr;

      if( !fs ) 
         fr.Open(g_fs, filename);
      else 
         fr.Open(fs, filename);

		char line[1024];
		char *p = 0;
		uint linenr = 0;
		Section *s = 0;

      bool reading_multiline = 0;
      std::string data;
      std::string key;
      while(fr.ReadLine(line, sizeof(line)))
		{
			linenr++;
			
         if( !reading_multiline ) 
            p = line;

			p = skipwhite(p);
			if (!p[0] || p[0] == '#' || (p[0] == '/' && p[1] == '/'))
				continue;

			if (p[0] == '[') {
				p++;
				setEndAt(p, ']');
				s = create_section(p, true); // may create duplicate
			} else {
            char* tail = 0;
            if( reading_multiline ) {
               data += '\n';
               tail = line;
            } else {
               tail = strchr(p, '=');
               *tail++ = 0;
               key = p;
					if(*tail == '_' ) 
                  tail+= 1; // skip =_, which is only used for translations
               tail = skipwhite(tail);
					killcomments(tail);
					rtrim(tail);
					rtrim(p);
            
               // remove surrounding '' or ""
					if (tail[0] == '\'' || tail[0] == '\"') {
                  reading_multiline = true;
                  tail++;
					}
            }
				if (tail) {
               char *eot = tail+strlen(tail)-1;
               if (*eot == '\'' || *eot == '\"') {
                  *eot = 0;
                  reading_multiline = false;
               }
					
               // ready to insert
					if (!s) {
						if (global_section)
							s = create_section(global_section, true);
						else
							throw wexception("line %i: key %s outside section", linenr, p);
					}
           
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
	catch(std::exception &e) {
		error("%s: %s", filename, e.what());
	}
}

/** Profile::write(const char *filename, bool used_only = true)
 *
 * Writes all sections out to the given file.
 * If used_only is true, only used sections and keys are written to the file.
 */
void Profile::write(const char *filename, bool used_only, FileSystem* fs)
{
	FileWrite fw;

	fw.Printf("# Automatically created by Widelands " VERSION "\n\n");

	for(Section_list::iterator s = m_sections.begin(); s != m_sections.end(); s++) {
		if (used_only && !s->is_used())
			continue;

		fw.Printf("[%s]\n", s->get_name());

		for(Section::Value_list::iterator v = s->m_values.begin(); v != s->m_values.end(); v++) {
			if (used_only && !v->is_used())
				continue;
         
         const char* str = v->get_string();
         bool multiline = false;
         for( uint i = 0; i < strlen( str ); i++)
            if( str[i] == '\n' )
               multiline = true;

         if( !multiline )
            fw.Printf("%s=%s\n", v->get_name(), v->get_string());
         else
            fw.Printf("%s='%s'\n", v->get_name(), v->get_string());
		}

		fw.Printf("\n");
	}

   if( !fs ) 
      fw.Write(g_fs, filename);
   else 
      fw.Write(fs, filename);
}


#ifdef TEST_PROFILE // this is out of date
int main(int argc, char **argv)
{
	if (argc != 2) {
		cerr << _("Usage: ")<<argv[0]<<" <ini-file>" << std::endl;
		return 3;
	}

	Profile p(argv[1]);
	Section *s;

	while((s = p.get_next_section("test"))) {
		const char *value;

		cout << "["<<s->get_name()<<"]" << std::endl;

		while(s->get_next_string("test", &value))
			cout << "test: \""<<value<<"\"" << std::endl;
	}

	if ((s = p.get_section("bool"))) {
		const char *key;
		bool v;

		cout << "[bool]" << std::endl;
		while((key = s->get_next_bool(0, &v)))
			cout << key<<"="<<v << std::endl;
	}

	if ((s = p.get_section("int"))) {
		const char *key;
		int v;

		cout << "[int]" << std::endl;
		while((key = s->get_next_int(0, &v)))
			cout << key<<"="<<v << std::endl;
	}
}
#endif /* TEST_PROFILE */
