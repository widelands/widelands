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
#include "../src/os.h"
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
	this->conf = new Profile(cerr, file);
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
	Section *s = conf->get_section("tribe");
	if (!s) {
		strcpy(header.author, "(unknown)");
		strcpy(header.name, "(no name)");
	} else {
		header.buildings = s->get_int("buildings");
		header.wares = s->get_int("wares");
		header.creatures = s->get_int("creatures");
		strcpy(header.author, s->get_string("author", "(unknown)"));
		strcpy(header.name, s->get_string("name", "(no name)"));
	}
}

int Conf_Reader::get_ware(const char* name)
{
	for (uint i=0; i<header.wares; i++)
		if (strcmpi(ware[i].name, name) == 0)
			return i;
	return 0xFFFFFFFF;
}

int Conf_Reader::get_creature(const char* name)
{
	for (uint j=0; j<header.creatures; j++)
		if (strcmpi(creature[j].name, name) == 0)
			return j;
	return 0xFFFFFFFF;
}

void Conf_Reader::read_buildings()
{
	if (!header.buildings)
		return;
	building = new BuildingDesc[header.buildings];
	for (uint i=0; i<header.buildings; i++)
	{
		memset(&building[i], 0, sizeof(BuildingDesc));
		char sectionName[32];
		sprintf(sectionName, "building_%i", i);

		Section *s = conf->get_section(sectionName);
		if (!s)
			continue;
		strcpy(building[i].name, s->get_string("name", sectionName));

		char profession[128];
		strcpy(profession, s->get_string("profession", ""));
		building[i].profession = this->get_creature(profession);

		char type[128];
		strcpy(type, s->get_string("type", ""));
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
		char sectionName[32];
		sprintf(sectionName, "creature_%i", i);

		Section *s = conf->get_section(sectionName);
		if (!s)
			continue;

		strcpy(creature[i].name, s->get_string("name", sectionName));

		char produce[128];
		strcpy(produce, s->get_string("produce", ""));
		creature[i].produce = this->get_ware(produce);

		creature[i].prodTime = s->get_int("prodtime");

		char ctype[128];
		strcpy(ctype, s->get_string("consume", ""));
		char* consume = strchr(ctype, ':');
		consume[0] = 0;
		consume++;
		if (strcmpi(ctype, "bob") == 0)
			creature[i].consumeType = CONSUME_BOB;
		else if (strcmpi(ctype, "res") == 0)
			creature[i].consumeType = CONSUME_RESOURCE;
		strcpy(creature[i].consume, consume);
		//TODO
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
		char sectionName[32];
		sprintf(sectionName, "ware_%i", i);

		Section *s = conf->get_section(sectionName);
		if (!s)
			continue;

		strcpy(ware[i].name, s->get_string("name", sectionName));

		char* picName = new char[1024];
		strcpy(picName, s->get_string("pic", ""));
		ware[i].pic = header.pictures++;
		pics->add(picName);

		picName = new char[1024];
		strcpy(picName, s->get_string("menupic", ""));
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
