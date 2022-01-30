/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/tribes/requirements.h"

#include "base/i18n.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/map_object.h"

namespace Widelands {

bool Requirements::check(const MapObject& obj) const {
	return !m || m->check(obj);
}

constexpr uint16_t kCurrentPacketVersion = 4;

/**
 * Read this requirement from a file
 */
void Requirements::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			*this = RequirementsStorage::read(fr, egbase, mol);
		} else {
			throw UnhandledVersionError("Requirements", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("requirements: %s", e.what());
	}
}

void Requirements::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);

	if (!m) {
		fw.unsigned_16(0);
	} else {
		fw.unsigned_16(m->storage().id());
		m->write(fw, egbase, mos);
	}
}

RequirementsStorage::RequirementsStorage(uint32_t const init_id, Reader const init_reader)
   : id_(init_id), reader_(init_reader) {
	StorageMap& s = storageMap();

	assert(0 < init_id);
	assert(init_id < 65535);
	assert(s.find(init_id) == s.end());

	s.insert(std::make_pair(init_id, this));
}

uint32_t RequirementsStorage::id() const {
	return id_;
}

Requirements RequirementsStorage::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	uint32_t const id = fr.unsigned_16();

	if (id == 0) {
		return Requirements();
	}

	StorageMap& s = storageMap();
	StorageMap::iterator it = s.find(id);

	if (it == s.end()) {
		throw GameDataError("unknown requirement id %u", id);
	}

	return it->second->reader_(fr, egbase, mol);
}

RequirementsStorage::StorageMap& RequirementsStorage::storageMap() {
	static StorageMap map;
	return map;
}

void RequireOr::add(const Requirements& req) {
	m.push_back(req);
}

bool RequireOr::check(const MapObject& obj) const {
	for (const Requirements& req : m) {
		if (req.check(obj)) {
			return true;
		}
	}

	return false;
}

void RequireOr::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) const {
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.unsigned_16(m.size());

	for (const Requirements& req : m) {
		req.write(fw, egbase, mos);
	}
}

static Requirements read_or(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
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

void RequireAnd::add(const Requirements& req) {
	m.push_back(req);
}

bool RequireAnd::check(const MapObject& obj) const {
	for (const Requirements& req : m) {
		if (!req.check(obj)) {
			return false;
		}
	}
	return true;
}

void RequireAnd::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) const {
	assert(m.size() == static_cast<uint16_t>(m.size()));
	fw.unsigned_16(m.size());

	for (const Requirements& req : m) {
		req.write(fw, egbase, mos);
	}
}

static Requirements read_and(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
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

bool RequireAttribute::check(const MapObject& obj) const {
	if (TrainingAttribute::kTotal != at) {
		int32_t const value = obj.get_training_attribute(at);

		return value >= min && value <= max;
	}
	int32_t value = 0;
	value += obj.get_training_attribute(TrainingAttribute::kHealth);
	value += obj.get_training_attribute(TrainingAttribute::kAttack);
	value += obj.get_training_attribute(TrainingAttribute::kDefense);
	value += obj.get_training_attribute(TrainingAttribute::kEvade);
	return value >= min && value <= max;
}

void RequireAttribute::write(FileWrite& fw, EditorGameBase&, MapObjectSaver&) const {
	fw.unsigned_8(static_cast<uint8_t>(at));
	fw.signed_32(min);
	fw.signed_32(max);
}

static Requirements read_attribute(FileRead& fr, EditorGameBase&, MapObjectLoader&) {
	// Get the training attribute and check if it is a valid enum member
	// We use a temp value, because the static_cast to the enum might be undefined.
	uint8_t temp_at = fr.unsigned_8();
	switch (temp_at) {
	case static_cast<uint8_t>(TrainingAttribute::kHealth):
	case static_cast<uint8_t>(TrainingAttribute::kAttack):
	case static_cast<uint8_t>(TrainingAttribute::kDefense):
	case static_cast<uint8_t>(TrainingAttribute::kEvade):
	case static_cast<uint8_t>(TrainingAttribute::kTotal):
		break;
	default:
		throw GameDataError("expected kHealth (%u), kAttack (%u), kDefense (%u), kEvade "
		                    "(%u) or kTotal (%u) but found unknown attribute value (%u)",
		                    static_cast<unsigned int>(TrainingAttribute::kHealth),
		                    static_cast<unsigned int>(TrainingAttribute::kAttack),
		                    static_cast<unsigned int>(TrainingAttribute::kDefense),
		                    static_cast<unsigned int>(TrainingAttribute::kEvade),
		                    static_cast<unsigned int>(TrainingAttribute::kTotal), temp_at);
	}
	TrainingAttribute const at = static_cast<TrainingAttribute>(temp_at);

	int32_t const min = fr.signed_32();
	int32_t const max = fr.signed_32();

	return RequireAttribute(at, min, max);
}

const RequirementsStorage RequireAttribute::storage(requirementIdAttribute, read_attribute);
}  // namespace Widelands
