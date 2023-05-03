/*
 * Copyright (C) 2008-2023 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_FINDIMMOVABLE_H
#define WL_LOGIC_MAP_OBJECTS_FINDIMMOVABLE_H

#include "logic/map_objects/map_object_type.h"

namespace Widelands {

struct BaseImmovable;
class ImmovableDescr;
class Player;

struct FindImmovable {
private:
	struct BaseCapsule {
		BaseCapsule() = default;
		virtual ~BaseCapsule() = default;

		void addref() {
			++refcount;
		}
		void deref() {
			if (--refcount == 0) {
				delete this;
			}
		}
		[[nodiscard]] virtual bool accept(const BaseImmovable&) const = 0;

		int refcount{1};
	};
	template <typename T> struct Capsule : public BaseCapsule {
		Capsule(const T& init_op) : op(init_op) {  // NOLINT allow implicit conversion
		}
		[[nodiscard]] bool accept(const BaseImmovable& imm) const override {
			return op.accept(imm);
		}

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
		capsule = nullptr;
	}
	FindImmovable& operator=(const FindImmovable& o) {
		if (&o == this) {
			return *this;
		}
		capsule->deref();
		capsule = o.capsule;
		capsule->addref();
		return *this;
	}

	template <typename T> FindImmovable(const T& op) {  // NOLINT allow implicit conversion
		capsule = new Capsule<T>(op);
	}

	// Return true if this node should be returned by find_fields()
	[[nodiscard]] bool accept(const BaseImmovable& imm) const {
		return capsule->accept(imm);
	}
};

const FindImmovable& find_immovable_always_true();

// FindImmovable functor
struct FindImmovableSize {
	FindImmovableSize(int32_t const init_min, int32_t const init_max)
	   : min(init_min), max(init_max) {
	}

	[[nodiscard]] bool accept(const BaseImmovable&) const;

private:
	int32_t min, max;
};

struct FindImmovableType {
	explicit FindImmovableType(MapObjectType const init_type) : type(init_type) {
	}

	[[nodiscard]] bool accept(const BaseImmovable&) const;

private:
	MapObjectType type;
};

struct FindImmovableAttribute {
	explicit FindImmovableAttribute(uint32_t const init_attrib) : attrib(init_attrib) {
	}

	[[nodiscard]] bool accept(const BaseImmovable&) const;

private:
	int32_t attrib;
};

struct FindImmovablePlayerImmovable {
	FindImmovablePlayerImmovable() = default;

	[[nodiscard]] bool accept(const BaseImmovable&) const;
};

struct FindImmovablePlayerMilitarySite {
	explicit FindImmovablePlayerMilitarySite(const Player& init_player) : player(init_player) {
	}

	[[nodiscard]] bool accept(const BaseImmovable&) const;

private:
	const Player& player;
};

struct FindImmovableAttackTarget {
	FindImmovableAttackTarget() = default;

	[[nodiscard]] bool accept(const BaseImmovable&) const;
};

struct FindForeignMilitarysite {
	explicit FindForeignMilitarysite(const Player& init_player) : player(init_player) {
	}

	[[nodiscard]] bool accept(const BaseImmovable&) const;

private:
	const Player& player;
};

struct FindImmovableByDescr {
	explicit FindImmovableByDescr(const ImmovableDescr& init_descr) : descr(init_descr) {
	}

	[[nodiscard]] bool accept(const BaseImmovable&) const;

private:
	const ImmovableDescr& descr;
};

struct FindFlagOf {
	explicit FindFlagOf(const FindImmovable& init_finder) : finder(init_finder) {
	}

	[[nodiscard]] bool accept(const BaseImmovable&) const;

private:
	const FindImmovable finder;
};

struct FindFlagWithPlayersWarehouse {
	explicit FindFlagWithPlayersWarehouse(const Player& owner) : owner_(owner) {
	}

	[[nodiscard]] bool accept(const BaseImmovable& imm) const;

private:
	const Player& owner_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_FINDIMMOVABLE_H
