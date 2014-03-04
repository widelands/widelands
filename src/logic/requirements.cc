/*
 * Copyright (C) 2008-2010 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/requirements.h"

#include "container_iterate.h"
#include "i18n.h"
#include "logic/game_data_error.h"
#include "logic/instances.h"

namespace Widelands {

bool Requirements::check(const Map_Object & obj) const
{
	return !m or m->check(obj);
}

#define REQUIREMENTS_VERSION 3

/**
 * Read this requirement from a file
 */
void Requirements::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == REQUIREMENTS_VERSION) {
			*this = RequirementsStorage::read(fr, egbase, mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw wexception("requirements: %s", e.what());
	}
}

void Requirements::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
	const
{
	fw.Unsigned16(REQUIREMENTS_VERSION);

	if (!m) {
		fw.Unsigned16(0);
	} else {
		fw.Unsigned16(m->storage().id());
		m->write(fw, egbase, mos);
	}
}

RequirementsStorage::RequirementsStorage
	(uint32_t const _id, Reader const reader)
	: m_id(_id), m_reader(reader)
{
	StorageMap & s = storageMap();

	assert(0 < _id);
	assert    (_id < 65535);
	assert(s.find(_id) == s.end());

	s.insert(std::make_pair(_id, this));
}

uint32_t RequirementsStorage::id() const
{
	return m_id;
}

Requirements RequirementsStorage::read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	uint32_t const id = fr.Unsigned16();

	if (id == 0)
		return Requirements();

	StorageMap & s = storageMap();
	StorageMap::iterator it = s.find(id);

	if (it == s.end())
		throw game_data_error("unknown requirement id %u", id);

	return it->second->m_reader(fr, egbase, mol);
}

RequirementsStorage::StorageMap & RequirementsStorage::storageMap()
{
	static StorageMap map;
	return map;
}


void RequireOr::add(const Requirements & req)
{
	m.push_back(req);
}

bool RequireOr::check(const Map_Object & obj) const
{
	container_iterate_const(std::vector<Requirements>, m, i)
		if (i.current->check(obj))
			return true;

	return false;
}

void RequireOr::write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
	const
{
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.Unsigned16(m.size());

	container_iterate_const(std::vector<Requirements>, m, i)
		i.current->Write(fw, egbase, mos);
}

static Requirements readOr
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	uint32_t const count = fr.Unsigned16();
	RequireOr req;

	for (uint32_t i = 0; i < count; ++i) {
		Requirements sub;
		sub.Read(fr, egbase, mol);
		req.add(sub);
	}

	return req;
}

const RequirementsStorage RequireOr::storage(requirementIdOr, readOr);


void RequireAnd::add(const Requirements & req)
{
	m.push_back(req);
}

bool RequireAnd::check(const Map_Object & obj) const
{
	container_iterate_const(std::vector<Requirements>, m, i)
		if (!i.current->check(obj))
			return false;

	return true;
}

void RequireAnd::write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
	const
{
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.Unsigned16(m.size());

	container_iterate_const(std::vector<Requirements>, m, i)
		i.current->Write(fw, egbase, mos);
}

static Requirements readAnd
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	uint32_t const count = fr.Unsigned16();
	RequireAnd req;

	for (uint32_t i = 0; i < count; ++i) {
		Requirements sub;
		sub.Read(fr, egbase, mol);
		req.add(sub);
	}

	return req;
}

const RequirementsStorage RequireAnd::storage(requirementIdAnd, readAnd);


bool RequireAttribute::check(const Map_Object & obj) const
{
	if (atrTotal != at)
	{
		int32_t const value = obj.get_tattribute(at);

		return value >= min && value <= max;
	}
	else
	{
		int32_t value = 0;
		value += obj.get_tattribute(atrHP);
		value += obj.get_tattribute(atrAttack);
		value += obj.get_tattribute(atrDefense);
		value += obj.get_tattribute(atrEvade);
		return value >= min && value <= max;
	}
}

void RequireAttribute::write
	(FileWrite & fw, Editor_Game_Base &, Map_Map_Object_Saver &) const
{
	fw.Unsigned32(at);
	fw.Signed32(min);
	fw.Signed32(max);
}

static Requirements readAttribute
	(FileRead & fr, Editor_Game_Base &, Map_Map_Object_Loader &)
{
	tAttribute const at  = static_cast<tAttribute>(fr.Unsigned32());
	if
		(at != atrHP and at != atrAttack and at != atrDefense and at != atrEvade
		 and
		 at != atrTotal)
		throw game_data_error
			(
			 "expected atrHP (%u), atrAttack (%u), atrDefense (%u), atrEvade "
			 "(%u) or atrTotal (%u) but found unknown attribute value (%u)",
			 atrHP, atrAttack, atrDefense, atrEvade, atrTotal, at);
	int32_t const min = fr.Signed32();
	int32_t const max = fr.Signed32();

	return RequireAttribute(at, min, max);
}

const RequirementsStorage RequireAttribute::
	storage(requirementIdAttribute, readAttribute);
}

