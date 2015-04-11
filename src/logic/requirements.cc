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

#include "base/i18n.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/instances.h"

namespace Widelands {

bool Requirements::check(const MapObject & obj) const
{
	return !m || m->check(obj);
}

constexpr uint16_t kCurrentPacketVersion = 3;

/**
 * Read this requirement from a file
 */
void Requirements::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			*this = RequirementsStorage::read(fr, egbase, mol);
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw wexception("requirements: %s", e.what());
	}
}

void Requirements::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
	const
{
	fw.unsigned_16(kCurrentPacketVersion);

	if (!m) {
		fw.unsigned_16(0);
	} else {
		fw.unsigned_16(m->storage().id());
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
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	uint32_t const id = fr.unsigned_16();

	if (id == 0)
		return Requirements();

	StorageMap & s = storageMap();
	StorageMap::iterator it = s.find(id);

	if (it == s.end())
		throw GameDataError("unknown requirement id %u", id);

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

bool RequireOr::check(const MapObject & obj) const
{
	for (const Requirements& req : m) {
		if (req.check(obj)) {
			return true;
		}
	}

	return false;
}

void RequireOr::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
	const
{
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.unsigned_16(m.size());

	for (const Requirements& req : m) {
		req.write(fw, egbase, mos);
	}
}

static Requirements read_or
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	uint32_t const count = fr.unsigned_16();
	RequireOr req;

	for (uint32_t i = 0; i < count; ++i) {
		Requirements sub;
		sub.read(fr, egbase, mol);
		req.add(sub);
	}

	return req;
}

const RequirementsStorage RequireOr::storage(requirementIdOr, read_or);


void RequireAnd::add(const Requirements & req)
{
	m.push_back(req);
}

bool RequireAnd::check(const MapObject & obj) const
{
	for (const Requirements& req : m) {
		if (!req.check(obj)) {
			return false;
		}
	}
	return true;
}

void RequireAnd::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
	const
{
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.unsigned_16(m.size());

	for (const Requirements& req : m) {
		req.write(fw, egbase, mos);
	}
}

static Requirements read_and
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	uint32_t const count = fr.unsigned_16();
	RequireAnd req;

	for (uint32_t i = 0; i < count; ++i) {
		Requirements sub;
		sub.read(fr, egbase, mol);
		req.add(sub);
	}

	return req;
}

const RequirementsStorage RequireAnd::storage(requirementIdAnd, read_and);


bool RequireAttribute::check(const MapObject & obj) const
{
	if (atrTotal != at)
	{
		int32_t const value = obj.get_training_attribute(at);

		return value >= min && value <= max;
	}
	else
	{
		int32_t value = 0;
		value += obj.get_training_attribute(atrHP);
		value += obj.get_training_attribute(atrAttack);
		value += obj.get_training_attribute(atrDefense);
		value += obj.get_training_attribute(atrEvade);
		return value >= min && value <= max;
	}
}

void RequireAttribute::write
	(FileWrite & fw, EditorGameBase &, MapObjectSaver &) const
{
	fw.unsigned_32(at);
	fw.signed_32(min);
	fw.signed_32(max);
}

static Requirements read_attribute
	(FileRead & fr, EditorGameBase &, MapObjectLoader &)
{
	TrainingAttribute const at  = static_cast<TrainingAttribute>(fr.unsigned_32());
	if
		(at != atrHP && at != atrAttack && at != atrDefense && at != atrEvade
		 &&
		 at != atrTotal)
		throw GameDataError
			(
			 "expected atrHP (%u), atrAttack (%u), atrDefense (%u), atrEvade "
			 "(%u) or atrTotal (%u) but found unknown attribute value (%u)",
			 atrHP, atrAttack, atrDefense, atrEvade, atrTotal, at);
	int32_t const min = fr.signed_32();
	int32_t const max = fr.signed_32();

	return RequireAttribute(at, min, max);
}

const RequirementsStorage RequireAttribute::
	storage(requirementIdAttribute, read_attribute);
}
