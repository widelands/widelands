/*
 * Copyright (C) 2018 by the Widelands Development Team
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

#ifndef WL_WEBSITE_JSON_JSON_H
#define WL_WEBSITE_JSON_JSON_H

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "website/json/value.h"

namespace JSON {

class Element {
protected:
	// Constructor for child node
	explicit Element(int level) : level_(level) {
	}

public:
	// Constructor for root node
	explicit Element() : JSON::Element(0) {
	}

	template <typename ObjectType> ObjectType* add_object() {
		objects_.push_back(std::unique_ptr<ObjectType>(new ObjectType(level_ + 1)));
		return dynamic_cast<ObjectType*>(objects_.back().get());
	}

	virtual std::string as_string() const;

protected:
	static const std::string tab_;

	std::string children_as_string() const;
	std::string key_to_string(const std::string& value) const;

	const int level_;
	std::vector<std::unique_ptr<JSON::Element>> objects_;
};

class Object : public Element {
	friend class JSON::Element;

protected:
	// Constructor for child node
	explicit Object(int level);

public:
	void add_bool(const std::string& key, bool value);
	void add_double(const std::string& key, double value);
	void add_int(const std::string& key, int value);
	void add_null(const std::string& key);
	void add_string(const std::string& key, const std::string& value);
	std::string as_string() const override;

private:
	std::vector<std::pair<std::string, std::unique_ptr<JSON::Value>>> values_;
};
}  // namespace JSON
#endif  // end of include guard: WL_WEBSITE_JSON_JSON_H
