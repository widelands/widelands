/*
 * Copyright (C) 2008-2013 by the Widelands Development Team
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

#ifndef FINDIMMOVABLE_H
#define FINDIMMOVABLE_H

#include <stdint.h>

namespace Widelands {

struct BaseImmovable;
struct Immovable_Descr;
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
		virtual bool accept(const BaseImmovable &) const = 0;

		int refcount;
	};
	template<typename T>
	struct Capsule : public BaseCapsule {
		Capsule(const T & _op) : op(_op) {}
		bool accept(const BaseImmovable & imm) const override {return op.accept(imm);}

		const T op;
	};

	BaseCapsule * capsule;

public:
	FindImmovable(const FindImmovable & o) {
		capsule = o.capsule;
		capsule->addref();
	}
	~FindImmovable() {
		capsule->deref();
		capsule = nullptr;
	}
	FindImmovable & operator= (const FindImmovable & o) {
		capsule->deref();
		capsule = o.capsule;
		capsule->addref();
		return *this;
	}

	template<typename T>
	FindImmovable(const T & op) {
		capsule = new Capsule<T>(op);
	}

	// Return true if this node should be returned by find_fields()
	bool accept(const BaseImmovable & imm) const {
		return capsule->accept(imm);
	}
};

// FindImmovable functor
struct FindImmovableSize {
	FindImmovableSize(int32_t const min, int32_t const max)
		: m_min(min), m_max(max)
	{}

	bool accept(const BaseImmovable &) const;

private:
	int32_t m_min, m_max;
};
struct FindImmovableType {
	FindImmovableType(int32_t const type) : m_type(type) {}

	bool accept(const BaseImmovable &) const;

private:
	int32_t m_type;
};
struct FindImmovableAttribute {
	FindImmovableAttribute(uint32_t const attrib) : m_attrib(attrib) {}

	bool accept(const BaseImmovable &) const;

private:
	int32_t m_attrib;
};
struct FindImmovablePlayerImmovable {
	FindImmovablePlayerImmovable() {}

	bool accept(const BaseImmovable &) const;
};
struct FindImmovablePlayerMilitarySite {
	FindImmovablePlayerMilitarySite(const Player & _player) : player(_player) {}

	bool accept(const BaseImmovable &) const;

	const Player & player;
};
struct FindImmovableAttackable {
	FindImmovableAttackable()  {}

	bool accept(const BaseImmovable &) const;
};
struct FindImmovableByDescr {
	FindImmovableByDescr(const Immovable_Descr & _descr) : descr(_descr) {}

	bool accept(const BaseImmovable &) const;

	const Immovable_Descr & descr;
};
struct FindFlagOf {
	FindFlagOf(const FindImmovable & finder) : finder_(finder) {}

	bool accept(const BaseImmovable &) const;

	const FindImmovable finder_;
};


} // namespace Widelands

#endif
