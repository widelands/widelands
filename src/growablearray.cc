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

#include "GrowableArray.h"
#include <string.h>

Growable_Array::Growable_Array(int size, int inc)
{
	this->elementData = new void*[size];
	this->capacity = size;
	this->increase = inc;
	this->elementCount = 0;
	memset(this->elementData, 0, this->capacity * sizeof(void*));
}

Growable_Array::~Growable_Array()
{
	delete this->elementData;
}

void Growable_Array::flush()
{
	delete[] this->elementData;
	this->elementData = new void*[this->increase];
	this->capacity = this->increase;
	this->elementCount = 0;
	memset(this->elementData, 0, this->capacity * sizeof(void*));
}

void Growable_Array::ensure_capacity(int cap)
{
	if (this->capacity >= cap)
		return;
	if (this->capacity + this->increase >= cap)
		this->grow(this->increase);
	else
		this->grow(cap - this->capacity);
}

void Growable_Array::grow(int inc)
{
	int newCapacity = this->capacity;
	if (inc > 0)
		newCapacity += inc;
	else
		newCapacity *= 2;
	void** newData = new void*[newCapacity];
	memset(newData, 0, newCapacity * sizeof(void*));
	memcpy(newData, this->elementData, this->capacity * sizeof(void*));
	delete this->elementData;
	this->elementData = newData;
	this->capacity = newCapacity;
}

void Growable_Array::add(void* data)
{
	this->ensure_capacity(this->elementCount + 1);
	this->elementData[this->elementCount++] = data;
}

void Growable_Array::insert_at(int i, void* data)
{
	this->ensure_capacity(this->elementCount + 1);
	int s = sizeof(void*);
	memmove(&elementData[i+1], &elementData[i], (elementCount - i) * s);
//	memmove(elementData + (i+1) * s, elementData + i * s, (elementCount - i) * s);
	elementData[i] = data;
	this->elementCount++;
}

void* Growable_Array::remove(int i)
{
	void* ret = this->elementData[i];
	int s = sizeof(void*);
	memmove(&elementData[i], &elementData[i+1], (elementCount - i) * s);
	this->elementCount--;
	return ret;
}

int Growable_Array::elements()
{
	return this->elementCount;
}

void* Growable_Array::element_at(int i)
{
	return this->elementData[i];
}