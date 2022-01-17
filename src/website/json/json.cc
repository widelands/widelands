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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "website/json/json.h"

#include <memory>

#include "io/filewrite.h"

// ########################## JSON Element #############################

namespace JSON {
const std::string JSON::Element::tab_ = "   ";

JSON::Object* Element::add_object(const std::string& key) {
	children_.push_back(std::unique_ptr<JSON::Object>(new JSON::Object(key, level_ + 1)));
	return dynamic_cast<JSON::Object*>(children_.back().get());
}

JSON::Array* Element::add_array(const std::string& key) {
	children_.push_back(std::unique_ptr<JSON::Array>(new JSON::Array(key, level_ + 1)));
	return dynamic_cast<JSON::Array*>(children_.back().get());
}

void Element::add_bool(const std::string& key, bool value) {
	values_.push_back(std::make_pair(key, std::unique_ptr<JSON::Value>(new JSON::Boolean(value))));
}

void Element::add_double(const std::string& key, double value) {
	values_.push_back(std::make_pair(key, std::unique_ptr<JSON::Value>(new JSON::Double(value))));
}
void Element::add_int(const std::string& key, int value) {
	values_.push_back(std::make_pair(key, std::unique_ptr<JSON::Value>(new JSON::Int(value))));
}
void Element::add_empty(const std::string& key) {
	values_.push_back(std::make_pair(key, std::unique_ptr<JSON::Value>(new JSON::Empty())));
}
void Element::add_string(const std::string& key, const std::string& value) {
	values_.push_back(std::make_pair(key, std::unique_ptr<JSON::Value>(new JSON::String(value))));
}

void Element::write_to_file(FileSystem& fs, const std::string& filename) const {
	FileWrite file_writer;
	file_writer.text(as_string());
	file_writer.write(fs, filename);
}

std::string Element::as_string() const {
	return "{\n" + children_as_string() + "}\n";
}

std::string Element::values_as_string(const std::string& tabs) const {
	std::string result;
	if (!values_.empty()) {
		for (size_t i = 0; i < values_.size() - 1; ++i) {
			const auto& element = values_.at(i);
			const std::string element_as_string = element.second->as_string();
			result.append(tabs)
			   .append(tab_)
			   .append(key_to_string(element.first, element_as_string.empty()))
			   .append(element_as_string)
			   .append(",\n");
		}
		const auto& element = values_.at(values_.size() - 1);
		const std::string element_as_string = element.second->as_string();
		result += tabs + tab_ + key_to_string(element.first, element_as_string.empty()) +
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
	return "\"" + value + (value_is_empty ? "\"" : "\": ");
}

// ########################## JSON Object #############################

Object::Object(const std::string& key, int level) : JSON::Element(key, level) {
}

std::string Object::as_string() const {
	std::string result;
	std::string tabs;
	for (int i = 0; i < level_; ++i) {
		tabs += tab_;
	}

	result += tabs + (key_.empty() ? "" : key_to_string(key_)) + "{\n";
	result += values_as_string(tabs);
	result += children_as_string();
	result += tabs + "}";
	return result;
}

// ########################## JSON Array #############################

Array::Array(const std::string& key, int level) : JSON::Element(key, level) {
}

std::string Array::as_string() const {
	std::string result;
	std::string tabs;
	for (int i = 0; i < level_; ++i) {
		tabs += tab_;
	}

	result += tabs + key_to_string(key_) + "[\n";
	result += values_as_string(tabs);
	result += children_as_string();
	result += tabs + "]";
	return result;
}

}  // namespace JSON
