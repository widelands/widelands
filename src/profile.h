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

class Growable_Array;

#define MAX_NAME_LEN	64
#define MAX_VAL_LEN		256

class Profile
{
	Growable_Array* values;
	struct Value
	{
		char section[MAX_NAME_LEN];
		char name[MAX_NAME_LEN];
		char val[MAX_VAL_LEN];
	};
	Value*	get_val(const char* section, const char* name);
public:
			Profile(const char* filename);
			~Profile();
	int		get_int(const char* section, const char* name, int def = 0);
	bool	get_boolean(const char* section, const char* name, bool def = false);
	void	get_string(const char* section, const char* name, char* buffer, const char* def = "");
};