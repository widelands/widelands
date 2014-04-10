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

#ifndef REQUIREMENTS_H
#define REQUIREMENTS_H

#include <climits>
#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "logic/tattribute.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

namespace Widelands {

class Map_Object;
class Editor_Game_Base;
class Map_Map_Object_Loader;
struct Map_Map_Object_Saver;

struct RequirementsStorage;

/**
 * Requirements can be attached to Requests.
 *
 * Requirements are matched to a \ref Map_Object 's \ref tAttribute as
 * returned by \ref get_tattribute .
 */
struct Requirements {
private:
	struct BaseCapsule {
		virtual ~BaseCapsule() {}

		virtual bool check(const Map_Object &) const = 0;
		virtual void write
			(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver &) const
			= 0;
		virtual const RequirementsStorage & storage() const = 0;
	};

	template<typename T>
	struct Capsule : public BaseCapsule {
		Capsule(const T & _m) : m(_m) {}

		bool check(const Map_Object & obj) const override {return m.check(obj);}

		void write
			(FileWrite            & fw,
			 Editor_Game_Base     & egbase,
			 Map_Map_Object_Saver & mos)
			const override
		{
			m.write(fw, egbase, mos);
		}

		const RequirementsStorage & storage() const override {return T::storage;}

		T m;
	};

public:
	Requirements() {}

	template<typename T> Requirements(const T & req) : m(new Capsule<T>(req)) {}

	/**
	 * \return \c true if the object satisfies the requirements.
	 */
	bool check(const Map_Object &) const;

	// For Save/Load Games
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) const;

private:
	boost::shared_ptr<BaseCapsule> m;
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
	typedef
		Requirements (*Reader)
			(FileRead &, Editor_Game_Base &, Map_Map_Object_Loader &);

	RequirementsStorage(uint32_t _id, Reader reader);
	uint32_t id() const;

	static Requirements read
		(FileRead &, Editor_Game_Base &, Map_Map_Object_Loader &);

private:
	typedef std::map<uint32_t, RequirementsStorage *> StorageMap;

	uint32_t m_id;
	Reader m_reader;

	static StorageMap & storageMap();
};


/**
 * Require that at least one of the sub-requirements added with \ref add()
 * is met. Defaults to \c false if no sub-requirement is added.
 */
struct RequireOr {
	void add(const Requirements &);

	bool check(const Map_Object &) const;
	void write
		(FileWrite &, Editor_Game_Base & egbase, Map_Map_Object_Saver &) const;

	static const RequirementsStorage storage;

private:
	std::vector<Requirements> m;
};


/**
 * Require that all sub-requirements added \ref add() are met.
 * Defaults to \c true if no sub-requirement is added.
 */
struct RequireAnd {
	void add(const Requirements &);

	bool check(const Map_Object &) const;
	void write
		(FileWrite &, Editor_Game_Base & egbase, Map_Map_Object_Saver &) const;

	static const RequirementsStorage storage;

private:
	std::vector<Requirements> m;
};


/**
 * Require that a \ref tAttribute lies in the given, inclusive, range.
 */
struct RequireAttribute {
	RequireAttribute
		(tAttribute const _at, int32_t const _min, int32_t const _max)
		: at(_at), min(_min), max(_max) {}

	RequireAttribute() : at(atrTotal), min(SHRT_MIN), max(SHRT_MAX) {}
	bool check(const Map_Object &) const;
	void write
		(FileWrite &, Editor_Game_Base & egbase, Map_Map_Object_Saver &) const;

	static const RequirementsStorage storage;

	int32_t getMin() const {return min; }
	int32_t getMax() const {return max; }

private:
	tAttribute at;
	int32_t min;
	int32_t max;
};

}

#endif // REQUIREMENTS_H
