/*
 * Copyright (C) 2008 by the Widelands Development Team
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
		virtual bool accept(BaseImmovable* imm) const = 0;

		int refcount;
	};
	template<typename T>
	struct Capsule : public BaseCapsule {
		Capsule(const T& _op) : op(_op) {}
		bool accept(BaseImmovable* imm) const {return op.accept(imm);}

		const T op;
	};

	BaseCapsule* capsule;

public:
	FindImmovable(const FindImmovable& o) {
		capsule = o.capsule;
		capsule->addref();
	}
	~FindImmovable() {
		capsule->deref();
		capsule = 0;
	}
	FindImmovable& operator=(const FindImmovable& o) {
		capsule->deref();
		capsule = o.capsule;
		capsule->addref();
		return *this;
	}

	template<typename T>
	FindImmovable(const T& op) {
		capsule = new Capsule<T>(op);
	}

	// Return true if this node should be returned by find_fields()
	bool accept(BaseImmovable* imm) const {
		return capsule->accept(imm);
	}
};

// FindImmovable functor
struct FindImmovableSize {
	FindImmovableSize(int32_t min, int32_t max) : m_min(min), m_max(max) {}

	bool accept(BaseImmovable *imm) const;

private:
	int32_t m_min, m_max;
};
struct FindImmovableType {
	FindImmovableType(int32_t type) : m_type(type) {}

	bool accept(BaseImmovable *imm) const;

private:
	int32_t m_type;
};
struct FindImmovableAttribute {
	FindImmovableAttribute(uint32_t attrib) : m_attrib(attrib) {}

	bool accept(BaseImmovable *imm) const;

private:
	int32_t m_attrib;
};
struct FindImmovablePlayerImmovable {
	FindImmovablePlayerImmovable() {}

	bool accept(BaseImmovable* imm) const;
};
struct FindImmovablePlayerMilitarySite {
	FindImmovablePlayerMilitarySite(Player* _player) : player(_player) {}

	bool accept(BaseImmovable* imm) const;

	Player* player;
};
struct FindImmovableAttackable {
	FindImmovableAttackable()  {}

	bool accept(BaseImmovable* imm) const;
};


}

#endif // FINDIMMOVABLE_H
