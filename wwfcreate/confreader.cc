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

#include "confreader.h"
#include "../src/growablearray.h"
#include "../src/profile.h"
#include "../src/worldfiletypes.h"
#include <string.h>
#include <stdio.h>

inline char* ltrim(char* str)
{
	int i = 0;
	while (str[i] == ' ' || str[i] == '\t')
		i++;
	return strcpy(str, str + i);
}

Conf_Reader::Conf_Reader(const char* file)
{
	this->conf = new Profile(file);
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
	this->pics->flush();
	delete this->pics;
	delete this->resource;
	delete this->terrain;
}

void Conf_Reader::read_header()
{
	memset(&header, 0, sizeof(WorldFileHeader));
	strcpy(header.magic, WLWF_MAGIC);
	header.version = WLWF_VERSION;
	header.bobs = conf->get_int("world", "bobs");
	header.resources = conf->get_int("world", "resources");
	header.terrains = conf->get_int("world", "terrains");
	header.pictures = 0;
	header.checksum = 0;
	conf->get_string("world", "author", header.author, "(unknown)");
	conf->get_string("world", "name", header.name, "(no name)");
}

void Conf_Reader::read_resources()
{
	resource = new ResourceDesc[header.resources];
	for (uint i=0; i<header.resources; i++)
	{
		memset(&resource[i], 0, sizeof(ResourceDesc));
		char sectionName[MAX_NAME_LEN];
		sprintf(sectionName, "resource_%i", i);
		conf->get_string(sectionName, "name", resource[i].name, "");
		resource[i].occurence = conf->get_int(sectionName, "occurence");
	}
}

void Conf_Reader::read_terrains()
{
	this->terrain = new TerrainType[header.terrains];
	for (uint i=0; i<header.terrains; i++)
	{
		char sectionName[MAX_NAME_LEN];
		sprintf(sectionName, "terrain_%i", i);
		
		memset(&terrain[i], 0, sizeof(TerrainType));
		conf->get_string(sectionName, "name", terrain[i].name, sectionName);
		terrain[i].attributes = 0;
		terrain[i].heightMax = conf->get_int(sectionName, "heightMax");
		terrain[i].heightMin = conf->get_int(sectionName, "heightMin");
		terrain[i].resources = 0;
		char resList[MAX_VAL_LEN];
		conf->get_string(sectionName, "resources", resList, "");
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
		conf->get_string(sectionName, "texture", picName, "");
		terrain[i].texture = pics->elements();
		pics->add(picName);
	}
}

void Conf_Reader::read_bobs()
{
	this->bob = new BobDesc[header.bobs];
	for (uint i=0; i<header.bobs; i++)
	{
		char sectionName[MAX_NAME_LEN];
		sprintf(sectionName, "bob_%i", i);

		memset(&bob[i], 0, sizeof(BobDesc));
		conf->get_string(sectionName, "name", bob[i].name, sectionName);
		bob[i].heir = conf->get_int(sectionName, "heir", -1);
		bob[i].stock = conf->get_int(sectionName, "stock", 1);
		bob[i].resource = conf->get_int(sectionName, "resource", -1);
		//bob[i].anim = 0;
		//bob[i].animFactor = conf->get_int(sectionName, "animFactor", -1);
		//bob[i].animKey = ...
		//bob[i].attributes = ...
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
