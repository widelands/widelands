/*
 * Copyright (C) 2018-2022 by the Widelands Development Team
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

#include "website/lua/lua_tree.h"

#include <memory>

#include "io/filewrite.h"

// ########################## LuaTree Element #############################

namespace LuaTree {
const std::string LuaTree::Element::tab_ = "   ";

LuaTree::Object* Element::add_object(const std::string& key) {
	children_.push_back(std::unique_ptr<LuaTree::Object>(new LuaTree::Object(key, level_ + 1)));
	return dynamic_cast<LuaTree::Object*>(children_.back().get());
}

void Element::add_bool(const std::string& key, bool value) {
	if (key.empty()) {
		keyless_values_.push_back(std::unique_ptr<LuaTree::Value>(new LuaTree::Boolean(value)));
	} else {
		values_.push_back(
		   std::unique_ptr<KeyValuePair>(new KeyValuePair(key, new LuaTree::Boolean(value))));
	}
}

void Element::add_double(const std::string& key, double value) {
	if (key.empty()) {
		keyless_values_.push_back(
		   std::unique_ptr<LuaTree::Value>(new LuaTree::Boolean(value != 0.0)));
	} else {
		values_.push_back(
		   std::unique_ptr<KeyValuePair>(new KeyValuePair(key, new LuaTree::Double(value))));
	}
}
void Element::add_int(const std::string& key, int value) {
	if (key.empty()) {
		keyless_values_.push_back(std::unique_ptr<LuaTree::Value>(new LuaTree::Int(value)));
	} else {
		values_.push_back(
		   std::unique_ptr<KeyValuePair>(new KeyValuePair(key, new LuaTree::Int(value))));
	}
}
void Element::add_empty(const std::string& key) {
	if (key.empty()) {
		keyless_values_.push_back(std::unique_ptr<LuaTree::Value>(new LuaTree::Empty()));
	} else {
		values_.push_back(std::unique_ptr<KeyValuePair>(new KeyValuePair(key, new LuaTree::Empty())));
	}
}
void Element::add_string(const std::string& key, const std::string& value) {
	if (key.empty()) {
		keyless_values_.push_back(std::unique_ptr<LuaTree::Value>(new LuaTree::String(value)));
	} else {
		values_.push_back(
		   std::unique_ptr<KeyValuePair>(new KeyValuePair(key, new LuaTree::String(value))));
	}
}

void Element::add_raw(const std::string& key, const std::string& value) {
	if (key.empty()) {
		keyless_values_.push_back(std::unique_ptr<LuaTree::Value>(new LuaTree::Raw(value)));
	} else {
		values_.push_back(
		   std::unique_ptr<KeyValuePair>(new KeyValuePair(key, new LuaTree::Raw(value))));
	}
}

void Element::write_to_file(FileSystem& fs, const std::string& filename) const {
	FileWrite file_writer;
	file_writer.text(as_string());
	file_writer.write(fs, filename);
}

std::string Element::as_string() const {
	return children_as_string();
}

std::string Element::keyless_values_as_string() const {
	std::string result = "{";
	if (!keyless_values_.empty()) {
		for (size_t i = 0; i < keyless_values_.size() - 1; ++i) {
			const auto& element = keyless_values_.at(i);
			const std::string element_as_string = element->as_string();
			result += " " + element_as_string + ",";
		}
		const auto& element = keyless_values_.at(keyless_values_.size() - 1);
		const std::string element_as_string = element->as_string();
		result += " " + element_as_string;
	}
	result += (children_.empty() && values_.empty()) ? " }" : " },";
	return result;
}

std::string Element::values_as_string(const std::string& tabs) const {
	std::string result;
	if (!values_.empty()) {
		for (size_t i = 0; i < values_.size() - 1; ++i) {
			const auto& element = values_.at(i);
			const std::string element_as_string = element->value->as_string();
			result.append(tabs)
			   .append(tab_)
			   .append(key_to_string(element->key, element_as_string.empty()))
			   .append(element_as_string)
			   .append(",\n");
		}
		const auto& element = values_.at(values_.size() - 1);
		const std::string element_as_string = element->value->as_string();
		result += tabs + tab_ + key_to_string(element->key, element_as_string.empty()) +
		          element_as_string + (children_.empty() ? "\n" : ",\n");
	}
	return result;
}

std::string Element::children_as_string() const {
	std::string result;
	if (!children_.empty()) {
		for (size_t i = 0; i < children_.size() - 1; ++i) {
			result += children_.at(i)->as_string() + ",\n";
		}
		result += children_.at(children_.size() - 1)->as_string() + "\n";
	}
	return result;
}

std::string Element::key_to_string(const std::string& value, bool value_is_empty) {
	return value + (value_is_empty ? "" : " = ");
}

// ########################## LuaTree Table #############################

Object::Object(const std::string& key, int level) : LuaTree::Element(key, level) {
}

std::string Object::as_string() const {
	std::string result;
	std::string tabs;
	for (int i = 0; i < level_; ++i) {
		tabs += tab_;
	}

	if (!keyless_values_.empty()) {
		result += tabs + (key_.empty() ? "" : key_to_string(key_));
		result += keyless_values_as_string();
	} else {
		result += tabs + (key_.empty() ? "" : key_to_string(key_)) + "{\n";
	}
	result += values_as_string(tabs);
	result += children_as_string();
	if (keyless_values_.empty()) {
		result += tabs + "}";
	}
	return result;
}

}  // namespace LuaTree
