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
#include "growablearray.h"
#include "myfile.h"
#include "os.h"
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
	char* s = strchr(str, c);
	if (s)
		s[0] = 0;
	return str;
}

Profile::Profile(const char* filename)
{
	this->values = new Growable_Array(32, 8);
	Ascii_file* file = new Ascii_file();
	file->open(filename, File::READ);
	char line[1024];
	char inSection[MAX_NAME_LEN];
	inSection[0] = 0;
	uint lineNr = 0;
	while (file->read_line(line, 1024) >= 0)
	{
		lineNr++;
		ltrim(line);
		if (line[0] == '#' || line[0] == 0)
			continue;
		else if (line[0] == '[')
		{
			strncpy(inSection, setEndAt(line, ']') + 1, MAX_NAME_LEN);
			inSection[MAX_NAME_LEN-1] = 0;
		}
		else
		{
			char* tail = strchr(line, '=');
			if (tail)
			{
				tail[0] = 0;
				tail++;
				Value* val = new Value();
				strcpy(val->section, inSection);
				strncpy(val->name, line, MAX_NAME_LEN);
				val->name[MAX_NAME_LEN-1] = 0;
				strncpy(val->val, tail, MAX_VAL_LEN);
				val->val[MAX_VAL_LEN-1] = 0;
				values->add(val);
			}
			else
				printf("profile %s: error in line #%i\n", filename, lineNr);
		}
	}
	
}

Profile::~Profile()
{
	for (int i=this->values->elements()-1; i>=0; i--)
		//delete (Value*)values->remove(i);
		// ich loesch den ja eh gleich, also bin ich mal boese und mach:
		delete (Value*)values->element_at(i);
	delete this->values;
}

Profile::Value* Profile::get_val(const char* section, const char* name)
{
	for (int i=0; i<values->elements(); i++)
	{
		Value* v = (Value*)values->element_at(i);
		if (strcmpi(v->name, name) == 0)
			if (strcmpi(v->section, section) == 0)
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
	for (int i=0; i<TRUE_WORDS; i++)
		if (strcmpi(v->val, trueWords[i]))
			return true;
	for (int j=0; j<FALSE_WORDS; j++)
		if (strcmpi(v->val, falseWords[j]))
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
