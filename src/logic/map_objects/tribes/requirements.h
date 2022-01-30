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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_REQUIREMENTS_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_REQUIREMENTS_H

#include <climits>
#include <map>
#include <memory>
#include <vector>

#include "logic/map_objects/tribes/training_attribute.h"

class FileRead;
class FileWrite;

namespace Widelands {

class MapObject;
class EditorGameBase;
class MapObjectLoader;
struct MapObjectSaver;

struct RequirementsStorage;

/**
 * Requirements can be attached to Requests.
 *
 * Requirements are matched to a \ref MapObject 's \ref TrainingAttribute as
 * returned by \ref MapObject::get_training_attribute .
 */
struct Requirements {
private:
	struct BaseCapsule {
		virtual ~BaseCapsule() {
		}

		virtual bool check(const MapObject&) const = 0;
		virtual void write(FileWrite&, EditorGameBase&, MapObjectSaver&) const = 0;
		virtual const RequirementsStorage& storage() const = 0;
	};

	template <typename T> struct Capsule : public BaseCapsule {
		explicit Capsule(const T& init_m) : m(init_m) {
		}

		bool check(const MapObject& obj) const override {
			return m.check(obj);
		}

		void write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) const override {
			m.write(fw, egbase, mos);
		}

		const RequirementsStorage& storage() const override {
			return T::storage;
		}

		T m;
	};

public:
	Requirements() {
	}

	template <typename T> Requirements(const T& req) : m(new Capsule<T>(req)) {
	}

	/**
	 * \return \c true if the object satisfies the requirements.
	 */
	bool check(const MapObject&) const;

	// For Save/Load Games
	void read(FileRead&, EditorGameBase&, MapObjectLoader&);
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) const;

private:
	std::shared_ptr<BaseCapsule> m;
};

/**
 * On-disk IDs for certain requirements.
 *
 * Only add enums at the end, and make their value explicit.
 */
enum {
	requirementIdOr = 1,
	requirementIdAnd = 2,
	requirementIdAttribute = 3,
};

/**
 * Factory-like system for requirement loading from files.
 */
struct RequirementsStorage {
	using Reader = Requirements (*)(FileRead&, EditorGameBase&, MapObjectLoader&);

	RequirementsStorage(uint32_t id, Reader reader);
	uint32_t id() const;

	static Requirements read(FileRead&, EditorGameBase&, MapObjectLoader&);

private:
	using StorageMap = std::map<uint32_t, RequirementsStorage*>;

	uint32_t id_;
	Reader reader_;

	static StorageMap& storageMap();
};

/**
 * Require that at least one of the sub-requirements added with \ref add()
 * is met. Defaults to \c false if no sub-requirement is added.
 */
struct RequireOr {
	void add(const Requirements&);

	bool check(const MapObject&) const;
	void write(FileWrite&, EditorGameBase& egbase, MapObjectSaver&) const;

	static const RequirementsStorage storage;

private:
	std::vector<Requirements> m;
};

/**
 * Require that all sub-requirements added \ref add() are met.
 * Defaults to \c true if no sub-requirement is added.
 */
struct RequireAnd {
	void add(const Requirements&);

	bool check(const MapObject&) const;
	void write(FileWrite&, EditorGameBase& egbase, MapObjectSaver&) const;

	static const RequirementsStorage storage;

private:
	std::vector<Requirements> m;
};

/**
 * Require that a \ref TrainingAttribute lies in the given, inclusive, range.
 */
struct RequireAttribute {
	RequireAttribute(TrainingAttribute const init_at, int32_t const init_min, int32_t const init_max)
	   : at(init_at), min(init_min), max(init_max) {
	}

	RequireAttribute() : at(TrainingAttribute::kTotal), min(SHRT_MIN), max(SHRT_MAX) {
	}
	bool check(const MapObject&) const;
	void write(FileWrite&, EditorGameBase& egbase, MapObjectSaver&) const;

	static const RequirementsStorage storage;

	int32_t get_min() const {
		return min;
	}
	int32_t get_max() const {
		return max;
	}

private:
	TrainingAttribute at;
	int32_t min;
	int32_t max;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_REQUIREMENTS_H
