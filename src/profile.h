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

#ifndef PROFILE_H
#define PROFILE_H

#include "growablearray.h"

#include <iostream>

class Profile;

/*
Represents one section inside the .ini-style file, basically as a list of
key-value pairs.

get_string:
Returns the value of the first key with the given name. If the key isn't found,
def is returned.

get_int, get_boolean convert the value string to the desired type.

get_next_*:
Returns the value of the next key of the given name. Value::used is used to
determine which key is next.
The value of the key is stored in the second parameter. Functions return the
name of the key: you can pass name == 0 to retrieve any unused keys from the
section.
*/
class Section {
	friend Profile;

	std::ostream &err;
	bool used;
	char *sname;

	Growable_Array values;
	struct Value {
		bool used;
		char *name;
		char *val;

		Value(const char *nname, const char *nval);
		~Value();
	};

	Section(std::ostream &errstream, const char *name);
	~Section();

	void check_used();

	Value *get_val(const char *name);
	Value *get_next_val(const char *name);

	void add_val(const char *key, const char *value);

public:
	inline const char *get_name() const { return sname; }

	int get_int(const char *name, int def = 0);
	bool get_boolean(const char *name, bool def = false);
	const char *get_string(const char *name, const char *def = 0);

	const char *get_next_int(const char *name, int *value);
	const char *get_next_boolean(const char *name, bool *value);
	const char *get_next_string(const char *name, const char **value);
};

/*
Parses an .ini-style file into sections and key-value pairs.
The destructor prints out warnings if a section or value hasn't been used.

get_section:
Returns the first section of the given name (or 0 if none exist).

get_next_section:
Returns the next unused section of the given name, or 0 if all sections
have been used. name can be 0 to retrieve any remaining sections.
*/
class Profile {
	std::ostream &err;
	Growable_Array sections;

	void parse(const char *filename);
public:
	Profile(std::ostream &errstream, const char* filename);
	~Profile();

	Section *get_section(const char *name);
	Section *get_next_section(const char *name);
};

#endif /* PROFILE_H */
