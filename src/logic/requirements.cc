/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#include "requirements.h"

#include "game_data_error.h"
#include "instances.h"

#include "i18n.h"

#include "container_iterate.h"

namespace Widelands {

bool Requirements::check(Map_Object const & obj) const
{
	return !m or m->check(obj);
}

#define REQUIREMENTS_VERSION 3

/**
 * Read this requirement from a file
 */
void Requirements::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader * mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == REQUIREMENTS_VERSION) {
			*this = RequirementsStorage::read(fr, egbase, mol);
		} else if (packet_version == 2) {
			RequireAnd req;

			for (;;) {
				uint32_t const at = fr.Unsigned32();

				if (at == 0xffffffff)
					break;

				int32_t const min = fr.Signed32();
				int32_t const max = fr.Signed32();

				req.add(RequireAttribute(static_cast<tAttribute>(at), min, max));
			}

			*this = req;
		} else if (packet_version == 1) {
			RequireAnd req;

			{ //  HitPoints Levels
				uint8_t const min = fr.Unsigned8();
				uint8_t const max = fr.Unsigned8();
				req.add(RequireAttribute(atrHP, min, max));
			}

			{ //  Attack Levels
				uint8_t const min = fr.Unsigned8();
				uint8_t const max = fr.Unsigned8();
				req.add(RequireAttribute(atrAttack, min, max));
			}

			{ //  Defense levels
				uint8_t const min = fr.Unsigned8();
				uint8_t const max = fr.Unsigned8();
				req.add(RequireAttribute(atrDefense, min, max));
			}

			{ //  Evade
				uint8_t const min = fr.Unsigned8();
				uint8_t const max = fr.Unsigned8();
				req.add(RequireAttribute(atrEvade, min, max));
			}

			*this = req;
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw wexception("requirements: %s", e.what());
	}
}

void Requirements::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver * mos)
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
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader * mol)
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


void RequireOr::add(Requirements const & req)
{
	m.push_back(req);
}

bool RequireOr::check(Map_Object const & obj) const
{
	container_iterate_const(std::vector<Requirements>, m, i)
		if (i.current->check(obj))
			return true;

	return false;
}

void RequireOr::write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver * mos)
	const
{
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.Unsigned16(m.size());

	container_iterate_const(std::vector<Requirements>, m, i)
		i.current->Write(fw, egbase, mos);
}

static Requirements readOr
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader * mol)
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


void RequireAnd::add(Requirements const & req)
{
	m.push_back(req);
}

bool RequireAnd::check(Map_Object const & obj) const
{
	container_iterate_const(std::vector<Requirements>, m, i)
		if (!i.current->check(obj))
			return false;

	return true;
}

void RequireAnd::write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver * mos)
	const
{
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.Unsigned16(m.size());

	container_iterate_const(std::vector<Requirements>, m, i)
		i.current->Write(fw, egbase, mos);
}

static Requirements readAnd
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader * mol)
{
	uint32_t count = fr.Unsigned16();
	RequireAnd req;

	for (uint32_t i = 0; i < count; ++i) {
		Requirements sub;
		sub.Read(fr, egbase, mol);
		req.add(sub);
	}

	return req;
}

const RequirementsStorage RequireAnd::storage(requirementIdAnd, readAnd);


bool RequireAttribute::check(Map_Object const & obj) const
{
	int32_t const value = obj.get_tattribute(at);

	return value >= min && value <= max;
}

void RequireAttribute::write
	(FileWrite & fw, Editor_Game_Base &, Map_Map_Object_Saver *) const
{
	fw.Unsigned32(at);
	fw.Signed32(min);
	fw.Signed32(max);
}

static Requirements readAttribute
	(FileRead & fr, Editor_Game_Base &, Map_Map_Object_Loader *)
{
	tAttribute at = static_cast<tAttribute>(fr.Unsigned32());
	int32_t min = fr.Signed32();
	int32_t max = fr.Signed32();

	return RequireAttribute(at, min, max);
}

const RequirementsStorage RequireAttribute::
	storage(requirementIdAttribute, readAttribute);
}

