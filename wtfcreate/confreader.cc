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
//#include "../src/worldfiletypes.h"
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
	this->read_wares();
	this->read_creatures();
	this->read_buildings();
	delete this->conf;
	this->conf = NULL;
}

Conf_Reader::~Conf_Reader()
{
	for (int i=this->pics->elements()-1; i>=0; i--)
		delete[] (char*)this->pics->remove(i);
	delete this->pics;
}

void Conf_Reader::read_header()
{
	memset(&header, 0, sizeof(TribeFileHeader));
	strcpy(header.magic, WLTF_MAGIC);
	header.version = WLTF_VERSION;
//	header.pictures = 0;
//	header.checksum = 0;
	header.buildings = conf->get_int("tribe", "buildings");
	header.wares = conf->get_int("tribe", "wares");
	header.creatures = conf->get_int("tribe", "creatures");
	conf->get_string("tribe", "author", header.author, "(unknown)");
	conf->get_string("tribe", "name", header.name, "(no name)");
}

void Conf_Reader::read_buildings()
{
	if (!header.buildings)
		return;
	building = new BuildingDesc[header.buildings];
	for (uint i=0; i<header.buildings; i++)
	{
		memset(&building[i], 0, sizeof(BuildingDesc));
		char sectionName[MAX_NAME_LEN];
		sprintf(sectionName, "building_%i", i);
		conf->get_string(sectionName, "name", building[i].name, "(no name)");
		char profession[128];
		conf->get_string(sectionName, "profession", profession, "");
		//building[i].profession = 0xFFFFFFFF;
		for (uint j=0; j<header.creatures; j++)
			if (strcmpi(creature[j].name, profession))
			{
				building[i].profession = j;
				break;
			}
		char type[128];
		conf->get_string(sectionName, "type", type, "");
		if (strcmpi(type, "search") == 0)
			building[i].professionType = PROFESSION_SEARCH;
		else if (strcmpi(type, "sit") == 0)
			building[i].professionType = PROFESSION_SIT;
	}
}

void Conf_Reader::read_creatures()
{
	if (!header.creatures)
		return;
	creature = new CreatureDesc[header.creatures];
	memset(creature, 0, header.creatures * sizeof(CreatureDesc));
	for (uint i=0; i<header.creatures; i++)
	{
		char sectionName[MAX_NAME_LEN];
		sprintf(sectionName, "creature_%i", i);
		conf->get_string(sectionName, "name", creature[i].name, "(no name)");
		//TODO
		//produce
		creature[i].prodTime = conf->get_int(sectionName, "prodtime");
		//consume
		//consumetype
		//animation
	}
}

void Conf_Reader::read_wares()
{
	if (!header.wares)
		return;
	ware = new Ware[header.wares];
	memset(ware, 0, header.wares * sizeof(Ware));
	for (uint i=0; i<header.wares; i++)
	{
		char sectionName[MAX_NAME_LEN];
		sprintf(sectionName, "ware_%i", i);
		conf->get_string(sectionName, "name", ware[i].name);

		char* picName = new char[1024];
		conf->get_string(sectionName, "pic", picName, "");
		ware[i].pic = header.pictures++;
		pics->add(picName);

		picName = new char[1024];
		conf->get_string(sectionName, "menupic", picName, "");
		ware[i].menupic = header.pictures++;
		pics->add(picName);
	}
}

const TribeFileHeader* Conf_Reader::get_header()
{
	return &header;
}

const BuildingDesc* Conf_Reader::get_building(int n)
{
	return &building[n];
}

const CreatureDesc* Conf_Reader::get_creature(int n)
{
	return &creature[n];
}

const Ware* Conf_Reader::get_ware(int n)
{
	return &ware[n];
}

uint Conf_Reader::textures()
{
	return pics->elements();
}

const char* Conf_Reader::get_texture(int n)
{
	return (char*)pics->element_at(n);
}