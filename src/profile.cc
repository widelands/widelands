/*
 * Copyright (C) 2002 by Florian Bluemel
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <iostream>

#include "os.h"
#include "profile.h"
#include "myfile.h"

#define TRUE_WORDS 3
const char* trueWords[TRUE_WORDS] =
{
	"true",
	"yes",
	"on"
};

#define FALSE_WORDS 3
const char* falseWords[FALSE_WORDS] =
{
	"false",
	"no",
	"off"
};

/*
==============================================================================

Section::Value

==============================================================================
*/

Section::Value::Value(const char *nname, const char *nval)
{
	used = false;
	name = strdup(nname);
	val = strdup(nval);
}

Section::Value::~Value()
{
	free(name);
	free(val);
}

/*
==============================================================================

Section

==============================================================================
*/

Section::Section(std::ostream &errstream, const char *name, bool supress_error)
	: err(errstream), values(8, 8)
{
   supr_err=supress_error;
	used = false;
	sname = strdup(name);
}

Section::~Section()
{
	for(int i = values.elements()-1; i >= 0; i--)
		delete (Value *)values.element_at(i);
	free(sname);
}

/** Section::check_used()
 *
 * Print a warning for every unused value.
 * This should only be called from the Profile destructor
 */
void Section::check_used()
{
	for(int i = 0; i < values.elements(); i++) {
		Value *v = (Value *)values.element_at(i);
		if (!v->used) {
			if(!supr_err) err << "Section [" << sname << "], key '" << v->name << "' not used (did you spell the name correctly?)" << std::endl;
      }
	}
}

/** Section::get_val(const char *name)
 *
 * Returns the value associated with the given keyname.
 *
 * Args: name	name of the key
 *
 * Returns: Pointer to the Value struct; 0 if the key doesn't exist.
 */
Section::Value *Section::get_val(const char *name)
{
	for(int i = 0; i < values.elements(); i++) {
		Value *v = (Value *)values.element_at(i);
		if (!strcmpi(v->name, name)) {
			v->used = true;
			return v;
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
	for(int i = 0; i < values.elements(); i++) {
		Value *v = (Value *)values.element_at(i);
		if (v->used)
			continue;
		if (!name || !strcmpi(v->name, name)) {
			v->used = true;
			return v;
		}
	}

	return 0;
}

/** Section::add_val(const char *key, const char *value)
 *
 * Adds the given key-value pair to the section.
 *
 * Args: key	name of the key
 *       value	string associated with the key
 */
void Section::add_val(const char *key, const char *value)
{
	Value *v = new Value(key, value);
	values.add(v);
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
	return atoi(v->val);
}

/** Section::get_boolean(const char *name, bool def)
 *
 * Returns the boolean value of the given key. Falls back to a default value
 * if the key is not found.
 *
 * Args: name	name of the key
 *       def	fallback value
 *
 * Returns: the boolean value of the key
 */
bool Section::get_boolean(const char *name, bool def)
{
	Value *v = get_val(name);
	if (!v)
		return def;

	int i;
	for (i = 0; i < TRUE_WORDS; i++)
		if (!strcmpi(v->val, trueWords[i]))
			return true;
	for (i = 0; i < FALSE_WORDS; i++)
		if (!strcmpi(v->val, falseWords[i]))
			return false;

	if(!supr_err) err << "[" << sname << "], key '" << name << "' is not a boolean value" << std::endl;
	return def;
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
const char *Section::get_string(const char *name, const char *def = 0)
{
	Value *v = get_val(name);
	if (!v)
		return def;
   char* retval=v->val;
   if(retval[0]=='\'' || retval[0]=='\"') {
      retval++;
   }
   if(retval[strlen(retval)-1]=='\'' || retval[strlen(retval)-1]=='\"') {
      retval[strlen(retval)-1]='\0'; // well, we change the buffer, but this doesn't matter
   }  
	return retval;
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
	*value = atoi(v->val);
	return v->name;
}

/** Section::get_next_boolean(const char *name, int *value)
 *
 * Retrieve the next unused key with the given name as a boolean value.
 *
 * Args: name	name of the key, can be 0 to find all unused keys
 *       value	value of the key is stored here
 *
 * Returns: the name of the key, or 0 if none has been found
 */
const char *Section::get_next_boolean(const char *name, bool *value)
{
	for(;;) {
		Value *v = get_next_val(name);
		if (!v)
			return 0;

		int i;
		for (i = 0; i < TRUE_WORDS; i++)
			if (!strcmpi(v->val, trueWords[i])) {
				*value = true;
				return v->name;
			}
		for (i = 0; i < FALSE_WORDS; i++)
			if (!strcmpi(v->val, falseWords[i])) {
				*value = false;
				return v->name;
			}

		if(!supr_err) err << "[" << sname << "], key '" << v->name << "' is not a boolean value" << std::endl;
		// we can't really return anything, so just get the next value
		// I guess a goto would be more logical in this rare situation ;p
	}
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
	*value = v->val;
	return v->name;
}


/*
==============================================================================

Profile

==============================================================================
*/

/** Profile::Profile(std::ostream &errstream, const char* filename)
 *
 * Parses an ini-style file into sections and key-value pairs.
 *
 * Args: errstream	all syntax errors etc.. are written to this stream
 *       filename	name of the .ini file
 */
Profile::Profile(std::ostream &errstream, const char* filename, bool section_less_file,  bool suppress_error_msg )
	: err(errstream), sections(8, 8)
{
   supr_err=suppress_error_msg;
   if(section_less_file) {
      sections.add(new Section(errstream, "[__NO_SEC__]", supr_err));
   }
   parse(filename, section_less_file);
}

Profile::~Profile()
{
	for(int i = sections.elements()-1; i >= 0; i--) {
		Section *s = (Section *)sections.element_at(i);
		if (!s->used) {
			if(!supr_err) err << "Section [" << s->get_name() << "] not used (did you spell the name correctly?)" << std::endl;
      } else {
			s->check_used();
      }
		delete s;
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
	for(int i = 0; i < sections.elements(); i++) {
		Section *s = (Section *)sections.element_at(i);
		if (!strcmpi(s->get_name(), name)) {
			s->used = true;
			return s;
		}
	}

	return 0;
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
	for(int i = 0; i < sections.elements(); i++) {
		Section *s = (Section *)sections.element_at(i);
		if (s->used)
			continue;
		if (!name || !strcmpi(s->get_name(), name)) {
			s->used = true;
			return s;
		}
	}

	return 0;
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

/** Profile::parse(const char *filename)
 *
 * Parses an ini-style file into sections and key values. If a section or
 * key name occurs multiple times, an additional entry is created.
 *
 * Args: filename	name of the source file
 */
void Profile::parse(const char *filename, bool section_less_file )
{
   Ascii_file file;
   file.open(filename, File::READ);

   char line[1024];
   char *p;
   uint linenr = 0;
   Section *s = 0;

   while(file.read_line(line, sizeof(line)) >= 0)
   {
      linenr++;
      p = line;
      p = skipwhite(p);
      if (!p[0] || p[0] == '#' || (p[0] == '/' && p[1] == '/'))
         continue;

      if (p[0] == '[') {
         if(section_less_file) {
            if(!supr_err) err << filename<<", line "<<linenr<<": Section "<<p<<" in sectionless file!" << std::endl;
         } else {
            p++;
            setEndAt(p, ']');
            s = new Section(err, p, supr_err);
            sections.add(s);
         }
      } else {
         char *tail = strchr(p, '=');
         if (tail) {
            *tail++ = 0;
            tail = skipwhite(tail);
            killcomments(tail);
            rtrim(tail);
            rtrim(p);

            if(!section_less_file) {
               if (s) {
                  s->add_val(p, tail);
               } else {
                  if(!supr_err) err << filename<<", line "<<linenr<<": key "<<p<<" outside section" << std::endl;
               }
            } else {
               ((Section*) sections.element_at(0))->add_val(p, tail);
               // add to default section
            }

         } else {
            if(!supr_err) err << filename<<", line "<<linenr<<": syntax error" << std::endl;
         }
      }
   }
}

#ifdef TEST_PROFILE // needs profile.cc, myfile.cc growablearray.cc
int main(int argc, char **argv)
{
	if (argc != 2) {
		cerr << "Usage: "<<argv[0]<<" <ini-file>" << std::endl;
		return 3;
	}

	Profile p(cerr, argv[1]);
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
		while((key = s->get_next_boolean(0, &v)))
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
