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

#ifndef WL_WEBSITE_JSON_VALUE_H
#define WL_WEBSITE_JSON_VALUE_H

#include <string>

namespace JSON {

/// Value types for JSON
struct Value {
	Value() = default;
	virtual ~Value() = default;
	virtual std::string as_string() const = 0;
};

struct Boolean : Value {
	explicit Boolean(bool value);
	std::string as_string() const override;

private:
	const bool bool_value;
};

struct Double : Value {
	explicit Double(double value);
	std::string as_string() const override;

private:
	const double double_value;
};

struct Empty : Value {
	Empty() = default;
	std::string as_string() const override;
};

struct Int : Value {
	explicit Int(int value);
	std::string as_string() const override;

private:
	const int int_value;
};

struct String : Value {
	explicit String(const std::string& value);
	std::string as_string() const override;

private:
	const std::string string_value;
};

}  // namespace JSON
#endif  // end of include guard: WL_WEBSITE_JSON_VALUE_H
