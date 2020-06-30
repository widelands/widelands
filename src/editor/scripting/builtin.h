/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_EDITOR_SCRIPTING_BUILTIN_H
#define WL_EDITOR_SCRIPTING_BUILTIN_H

#include <functional>
#include <map>
#include <memory>

#include "editor/scripting/function.h"
#include "editor/scripting/scripting.h"

/************************************************************
                      Builtin functions
************************************************************/

// Wrapper for a (static) FunctionBase object, for use in kBuiltinFunctions
struct BuiltinFunctionInfo {
	BuiltinFunctionInfo(std::function<std::string()> d, FunctionBase* f, std::string i = "")
	   : function(f), description(d), included_from(i) {
	}
	BuiltinFunctionInfo(const BuiltinFunctionInfo&) = default;
	BuiltinFunctionInfo& operator=(const BuiltinFunctionInfo&) = default;
	~BuiltinFunctionInfo() {
	}

	const std::shared_ptr<FunctionBase> function;
	// Implemented as a function to make it translatable
	const std::function<std::string()> description;

	const std::string included_from;
};

// All supported builtin functions.
const extern std::map<std::string, BuiltinFunctionInfo> kBuiltinFunctions;
// Quick access to a builtin by its unique name
const BuiltinFunctionInfo& builtin_f(const std::string&);
std::string builtin_f(const FunctionBase&);

/************************************************************
                       Property
************************************************************/

// Used to access a member variable of a variable of builtin type. Since only builtins
// have properties, you do not instantiate this class. Use kBuiltinProperties instead.
// Property needs to be embedded in GetProperty or FS_SetProperty to be used for anything.
class Property {
public:
	enum Access { RO, RW };

	Property(
	   const std::string& n, Access a, const VariableType& c, const VariableType& t, bool ais = true)
	   : call_on_(c), type_(t), name_(n), access_(a), available_in_subclasses_(ais) {
		assert(type_.id() != VariableTypeID::Nil);
	}

	~Property() {
	}

	const VariableType& get_type() const {
		return type_;
	}
	const VariableType& get_class() const {
		return call_on_;
	}
	const std::string& get_name() const {
		return name_;
	}
	Access get_access() const {
		return access_;
	}
	bool available_in_subclasses() const {
		return available_in_subclasses_;
	}

private:
	VariableType call_on_;
	VariableType type_;
	std::string name_;
	Access access_;
	bool available_in_subclasses_;

	DISALLOW_COPY_AND_ASSIGN(Property);
};

/************************************************************
                      Builtin properties
************************************************************/

// Wrapper for a (static) Property object, for use in kBuiltinProperties
struct BuiltinPropertyInfo {
	BuiltinPropertyInfo(std::function<std::string()> d, Property* p) : property(p), description(d) {
	}
	BuiltinPropertyInfo(const BuiltinPropertyInfo&) = default;
	BuiltinPropertyInfo& operator=(const BuiltinPropertyInfo&) = default;
	~BuiltinPropertyInfo() {
	}

	const std::shared_ptr<Property> property;
	// Implemented as a function to make it translatable
	const std::function<std::string()> description;
};

// All supported builtin properties.
const extern std::map<std::string, BuiltinPropertyInfo> kBuiltinProperties;
// Quick access to a builtin by its unique name
const BuiltinPropertyInfo& builtin_p(const std::string&);
std::string builtin_p(const Property&);

#endif  // end of include guard: WL_EDITOR_SCRIPTING_BUILTIN_H
