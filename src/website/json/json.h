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

#include "io/filesystem/filesystem.h"
#include "website/json/value.h"

namespace JSON {

class Array;
class Object;

class Element {
protected:
	// Constructor for child node
	explicit Element(int level) : level_(level) {
	}

public:
	// Constructor for root node
	explicit Element() : JSON::Element(0) {
	}

	JSON::Object* add_object();
	JSON::Array* add_array(const std::string& key);
	void add_bool(const std::string& key, bool value);
	void add_double(const std::string& key, double value);
	void add_int(const std::string& key, int value);
	void add_empty(const std::string& key);
	void add_string(const std::string& key, const std::string& value);

	void write_to_file(FileSystem& fs, const std::string& filename) const;

	virtual std::string as_string() const;

protected:
	static const std::string tab_;

	std::string values_as_string(const std::string& tabs) const;
	std::string children_as_string() const;
	static std::string key_to_string(const std::string& value, bool value_is_empty = false);

	const int level_;
	std::vector<std::unique_ptr<JSON::Element>> children_;
	std::vector<std::pair<std::string, std::unique_ptr<JSON::Value>>> values_;
};

class Object : public Element {
	friend class JSON::Element;

protected:
	// Constructor for child node
	explicit Object(int level);

public:
	std::string as_string() const override;
};

class Array : public Element {
	friend class JSON::Element;
protected:
	// Constructor for child node
	explicit Array(const std::string key, int level);

public:
	std::string as_string() const override;

private:
	std::string key_;
};
}  // namespace JSON
#endif  // end of include guard: WL_WEBSITE_JSON_JSON_H
