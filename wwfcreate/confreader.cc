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

#include "../src/widelands.h"
#include "confreader.h"
#include "../src/growablearray.h"
#include "../src/profile.h"
#include "../src/worldfiletypes.h"

inline char* ltrim(char* str)
{
	int i = 0;
	while (str[i] == ' ' || str[i] == '\t')
		i++;
	return strcpy(str, str + i);
}

Conf_Reader::Conf_Reader(const char* file)
{
	this->conf = new Profile(cerr, file);
	this->pics = new Growable_Array(32, 8);
	this->read_header();
	this->read_resources();
	this->read_terrains();
	this->read_bobs();
	delete this->conf;
	this->conf = NULL;
}

Conf_Reader::~Conf_Reader()
{
	for (int i=this->pics->elements()-1; i>=0; i--)
		delete[] (char*)this->pics->remove(i);
	delete this->pics;
	delete this->resource;
	delete this->terrain;
}

void Conf_Reader::read_header()
{
	memset(&header, 0, sizeof(WorldFileHeader));
	strcpy(header.magic, WLWF_MAGIC);
	header.version = WLWF_VERSION;

	Section *s = conf->get_section("world");
	if (!s) {
		strcpy(header.author, "(unknown");
		strcpy(header.name, "(no name)");
	} else {
		header.bobs = s->get_int("bobs");
		header.resources = s->get_int("resources");
		header.terrains = s->get_int("terrains");
		strcpy(header.author, s->get_string("author", "(unknown)"));
		strcpy(header.name, s->get_string("name", "(no name)"));
	}

	header.pictures = 0;
	header.checksum = 0;
}

void Conf_Reader::read_resources()
{
	resource = new ResourceDesc[header.resources];
	for (uint i=0; i<header.resources; i++)
	{
		memset(&resource[i], 0, sizeof(ResourceDesc));
		char sectionName[32];
		sprintf(sectionName, "resource_%i", i);
		Section *s = conf->get_section(sectionName);
		if (!s)
			continue;
		strcpy(resource[i].name, s->get_string("name", ""));
		resource[i].occurence = s->get_int("occurence");
	}
}

void Conf_Reader::read_terrains()
{
	this->terrain = new TerrainType[header.terrains];
	for (uint i=0; i<header.terrains; i++)
	{
		char sectionName[32];
		sprintf(sectionName, "terrain_%i", i);

		memset(&terrain[i], 0, sizeof(TerrainType));
		
		Section *s = conf->get_section(sectionName);
		if (!s)
			continue;
		strcpy(terrain[i].name, s->get_string("name", sectionName));
		//terrain[i].attributes = 0;
		terrain[i].heightMax = s->get_int("heightMax");
		terrain[i].heightMin = s->get_int("heightMin");
		//terrain[i].resources = 0;
		char resList[256];
		strcpy(resList, s->get_string("resources", ""));
		char* resName = strtok(resList, ",");
		while (resName)
		{
			ltrim(resName);
			for (uint i=0; i<header.resources; i++)
				if (strcmp(resName, resName) == 0)
				{
					terrain[i].resources |= 1 << i;
					break;
				}
			resName = strtok(NULL, ",");
		};
		char* picName = new char[1024];
		strcpy(picName, s->get_string("texture", ""));
		terrain[i].texture = header.pictures++;
		pics->add(picName);
	}
}

void Conf_Reader::read_bobs()
{
	this->bob = new BobDesc[header.bobs];
	for (uint i=0; i<header.bobs; i++)
	{
		char sectionName[32];
		sprintf(sectionName, "bob_%i", i);

		memset(&bob[i], 0, sizeof(BobDesc));

		Section *s = conf->get_section(sectionName);
		if (!s)
			continue;
		strcpy(bob[i].name, s->get_string("name", sectionName));
		bob[i].heir = s->get_int("heir", -1);
		bob[i].stock = s->get_int("stock", 1);
		bob[i].resource = s->get_int("resource", -1);
		bob[i].lifetime = s->get_int("lifetime", 0);
		//bob[i].anim = 0;
		//bob[i].animFactor = conf->get_int(sectionName, "animFactor", -1);
		//bob[i].animKey = ...
		//**bob[i].attributes = ...
	}
}

const WorldFileHeader* Conf_Reader::get_header()
{
	return &header;
}

const ResourceDesc* Conf_Reader::get_resource(int n)
{
	return &resource[n];
}

const TerrainType* Conf_Reader::get_terrain(int n)
{
	return &terrain[n];
}

const BobDesc* Conf_Reader::get_bob(int n)
{
	return &bob[n];
}

uint Conf_Reader::textures()
{
	return pics->elements();
}

const char* Conf_Reader::get_texture(int n)
{
	return (char*)pics->element_at(n);
}
