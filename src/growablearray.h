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
#ifndef __GROWABLE_ARRAY_H
#define __GROWABLE_ARRAY_H

class Growable_Array
{
	void** elementData;
	int elementCount;
	int capacity;
	int increase;
	void grow(int inc);
public:
	Growable_Array(int size = 10, int increase = 0);
	~Growable_Array();
	void flush();
	void ensure_capacity(int cap);
	void add(void*);
	void insert_at(int i, void*);
	void* remove(int i);
	void* element_at(int i);
	int elements();
};

#endif

