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

#ifndef FINDIMMOVABLE_H
#define FINDIMMOVABLE_H

#include <stdint.h>

namespace Widelands {

class BaseImmovable;
class Player;

struct FindImmovable {
private:
	struct BaseCapsule {
		BaseCapsule() : refcount(1) {}
		virtual ~BaseCapsule() {}

		void addref() {++refcount;}
		void deref() {
			if (--refcount == 0)
				delete this;
		}
		virtual bool accept(BaseImmovable const &) const = 0;

		int refcount;
	};
	template<typename T>
	struct Capsule : public BaseCapsule {
		Capsule(T const & _op) : op(_op) {}
		bool accept(BaseImmovable const & imm) const {return op.accept(imm);}

		const T op;
	};

	BaseCapsule * capsule;

public:
	FindImmovable(FindImmovable const & o) {
		capsule = o.capsule;
		capsule->addref();
	}
	~FindImmovable() {
		capsule->deref();
		capsule = 0;
	}
	FindImmovable & operator= (FindImmovable const & o) {
		capsule->deref();
		capsule = o.capsule;
		capsule->addref();
		return *this;
	}

	template<typename T>
	FindImmovable(T const & op) {
		capsule = new Capsule<T>(op);
	}

	// Return true if this node should be returned by find_fields()
	bool accept(BaseImmovable const & imm) const {
		return capsule->accept(imm);
	}
};

// FindImmovable functor
struct FindImmovableSize {
	FindImmovableSize(int32_t const min, int32_t const max)
		: m_min(min), m_max(max)
	{}

	bool accept(BaseImmovable const &) const;

private:
	int32_t m_min, m_max;
};
struct FindImmovableType {
	FindImmovableType(int32_t const type) : m_type(type) {}

	bool accept(BaseImmovable const &) const;

private:
	int32_t m_type;
};
struct FindImmovableAttribute {
	FindImmovableAttribute(uint32_t const attrib) : m_attrib(attrib) {}

	bool accept(BaseImmovable const &) const;

private:
	int32_t m_attrib;
};
struct FindImmovablePlayerImmovable {
	FindImmovablePlayerImmovable() {}

	bool accept(BaseImmovable const &) const;
};
struct FindImmovablePlayerMilitarySite {
	FindImmovablePlayerMilitarySite(Player const & _player) : player(_player) {}

	bool accept(BaseImmovable const &) const;

	Player const & player;
};
struct FindImmovableAttackable {
	FindImmovableAttackable()  {}

	bool accept(BaseImmovable const &) const;
};


}

#endif
