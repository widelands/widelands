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

#include "profile.h"
#include "growableArray.h"
#include "myfile.h"
#include <string.h>
#include <stdlib.h>

#define TRUE_WORDS 3
char* trueWords[TRUE_WORDS] =
{
	"true",
	"yes",
	"on"
};

#define FALSE_WORDS 3
char* falseWords[FALSE_WORDS] =
{
	"false",
	"no",
	"off"
};

inline char* ltrim(char* str)
{
	int i = 0;
	while (str[i] == ' ' || str[i] == '\t')
		i++;
	return strcpy(str, str + i);
}

inline char* setEndAt(char* str, char c)
{
	int i = 0;
	while (str[i] != c && str[i])
		i++;
	str[i] = 0;
	return str;
}

inline char* setStartAfter(char* str, char c)
{
	int i = 0;
	while (str[i] != c && str[i])
		i++;
	return strcpy(str, str + i + 1);
}

Profile::Profile(const char* filename)
{
	this->values = new Growable_Array(32, 8);
	Ascii_file* file = new Ascii_file();
	file->open(filename, File::For::READ);
	char line[1024];
	char inSection[MAX_NAME_LEN];
	inSection[0] = 0;
	while (file->read_line(line, 1024) >= 0)
	{
		ltrim(line);
		if (line[0] == '#')
			continue;
		else if (line[0] == '[')
		{
			strncpy(inSection, setEndAt(line, ']'), MAX_NAME_LEN);
			inSection[MAX_NAME_LEN-1] = 0;
		}
		else
		{
			Value* val = new Value();
			//section:
			strcpy(val->section, inSection);
			//name:
			strncpy(val->name, line, MAX_NAME_LEN);
			val->name[MAX_NAME_LEN-1] = 0;
			setEndAt(val->name, '=');
			//value:
			setStartAfter(line, '=');
			strncpy(val->val, line, MAX_VAL_LEN);
			val->val[MAX_VAL_LEN-1] = 0;
			values->add(val);
		}
	}
	
}

Profile::~Profile()
{
	this->values->flush();
	delete this->values;
}

Profile::Value* Profile::get_val(const char* section, const char* name)
{
	for (int i=0; i<values->elements(); i++)
	{
		Value* v = (Value*)values->element_at(i);
		if (strcmp(v->name, name) == 0)
			if (strcmp(v->section, section) == 0)
				return v;
	}
	return NULL;
}

int Profile::get_int(const char* section, const char* name, int def)
{
	Value* v = this->get_val(section, name);
	if (!v)
		return def;
	return atoi(v->val);
}

bool Profile::get_boolean(const char* section, const char* name, bool def)
{
	Value* v = this->get_val(section, name);
	if (!v)
		return def;
//	if (atoi(v->val))
//		return true;		// things like "0001" are true
	strlwr(v->val);
	for (int i=0; i<TRUE_WORDS; i++)
		if (strcmp(v->val, trueWords[i]))
			return true;
	for (int j=0; j<FALSE_WORDS; j++)
		if (strcmp(v->val, falseWords[j]))
			return false;
	return def;
}

void Profile::get_string(const char* section, const char* name, char* buffer, const char* def)
{
	Value* v = this->get_val(section, name);
	if (!v)
		strcpy(buffer, def);
	else
		strcpy(buffer, v->val);
}