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

#ifndef WL_LOGIC_FINDNODE_H
#define WL_LOGIC_FINDNODE_H

#include <vector>

#include <stdint.h>

namespace Widelands {

struct FCoords;
class Map;

struct FindNode {
private:
	struct BaseCapsule {
		BaseCapsule() : refcount(1) {}
		virtual ~BaseCapsule() {}

		void addref() {++refcount;}
		void deref() {
			if (--refcount == 0)
				delete this;
		}
		virtual bool accept(const Map &, const FCoords & coord) const = 0;

		int refcount;
	};
	template<typename T>
	struct Capsule : public BaseCapsule {
		Capsule(const T & init_op) : op(init_op) {}
		bool accept(const Map & map, const FCoords & coord) const override {
			return op.accept(map, coord);
		}

		const T op;
	};

	BaseCapsule * capsule;

public:
	FindNode(const FindNode & o) {
		capsule = o.capsule;
		capsule->addref();
	}
	~FindNode() {
		capsule->deref();
		capsule = nullptr;
	}
	FindNode & operator= (const FindNode & o) {
		capsule->deref();
		capsule = o.capsule;
		capsule->addref();
		return *this;
	}

	template<typename T>
	FindNode(const T & op) {
		capsule = new Capsule<T>(op);
	}

	// Return true if this node should be returned by find_fields()
	bool accept(const Map & map, const FCoords & coord) const {
		return capsule->accept(map, coord);
	}
};

struct FindNodeCaps {
	FindNodeCaps(uint8_t init_mincaps) : mincaps(init_mincaps) {}

	bool accept(const Map &, const FCoords &) const;

private:
	uint8_t mincaps;
};

/// Accepts a node if it is accepted by all subfunctors.
struct FindNodeAnd {
	FindNodeAnd() {}

	void add(const FindNode &, bool negate = false);

	bool accept(const Map &, const FCoords &) const;

private:
	struct Subfunctor {
		bool negate;
		FindNode findfield;

		Subfunctor(const FindNode &, bool init_negate);
	};

	std::vector<Subfunctor> subfunctors;
};

/// Accepts a node based on what can be built there.
struct FindNodeSize {
	enum Size {
		sizeAny    = 0,   //  any field not occupied by a robust immovable
		sizeBuild,        //  any field we can build on (flag or building)
		sizeSmall,        //  at least small size
		sizeMedium,
		sizeBig,
		sizeMine,         //  can build a mine on this field
		sizePort,         //  can build a port on this field
	};

	FindNodeSize(Size init_size) : size(init_size) {}

	bool accept(const Map &, const FCoords &) const;

private:
	Size size;
};

/// Accepts a node based on the size of the immovable there (if any).
struct FindNodeImmovableSize {
	enum {
		sizeNone   = 1 << 0,
		sizeSmall  = 1 << 1,
		sizeMedium = 1 << 2,
		sizeBig    = 1 << 3
	};

	FindNodeImmovableSize(uint32_t init_sizes) : sizes(init_sizes) {}

	bool accept(const Map &, const FCoords &) const;

private:
	uint32_t sizes;
};

/// Accepts a node if it has an immovable with a given attribute.
struct FindNodeImmovableAttribute {
	FindNodeImmovableAttribute(uint32_t attrib) : attribute(attrib) {}

	bool accept(const Map &, const FCoords &) const;

private:
	uint32_t attribute;
};


/// Accepts a node if it has at least one of the given resource.
struct FindNodeResource {
	FindNodeResource(uint8_t res) : resource(res) {}

	bool accept(const Map &, const FCoords &) const;

private:
	uint8_t resource;
};


/// Accepts a node if it has the given resource type and remaining capacity.
struct FindNodeResourceBreedable {
	FindNodeResourceBreedable(uint8_t res) : resource(res) {}

	bool accept(const Map &, const FCoords &) const;

private:
	uint8_t resource;
};

/// Accepts a node if it is a shore node in the sense that it is walkable
/// and has a neighbouring field that is swimmable
struct FindNodeShore {
	FindNodeShore() {}

	bool accept(const Map &, const FCoords &) const;
};

}

#endif  // end of include guard: WL_LOGIC_FINDNODE_H
