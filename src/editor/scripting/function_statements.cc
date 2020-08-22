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

#include "editor/scripting/function_statements.h"

#include "editor/scripting/builtin.h"
#include "editor/scripting/variable.h"

/************************************************************
          Specific function statement implementations
************************************************************/

// Return statement

constexpr uint16_t kCurrentPacketVersionFS_Return = 1;
void FS_Return::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_Return) {
			throw Widelands::UnhandledVersionError(
			   "FS_Return", packet_version, kCurrentPacketVersionFS_Return);
		}
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("FS_Return: %s", e.what());
	}
}
void FS_Return::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	return_ = loader.front() ? &l.get<Assignable>(loader.front()) : nullptr;
	loader.pop_front();
}
void FS_Return::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_Return);
	fw.unsigned_32(return_ ? return_->serial() : 0);
}
std::set<uint32_t> FS_Return::references() const {
	auto set = FunctionStatement::references();
	if (return_) {
		set.insert(return_->serial());
	}
	return set;
}
std::string FS_Return::readable() const {
	return return_ ? "return " + return_->readable() : "return";
}
void FS_Return::write_lua(int32_t i, FileWrite& fw) const {
	fw.print_f("return");
	if (return_) {
		fw.print_f(" ");
		return_->write_lua(i, fw);
	}
}

// Variable declaration and assignment

constexpr uint16_t kCurrentPacketVersionFS_LocalVarDeclOrAssign = 1;
void FS_LocalVarDeclOrAssign::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_LocalVarDeclOrAssign) {
			throw Widelands::UnhandledVersionError("FS_LocalVarDeclOrAssign", packet_version,
			                                       kCurrentPacketVersionFS_LocalVarDeclOrAssign);
		}
		declare_local_ = fr.unsigned_8();
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("FS_LocalVarDeclOrAssign: %s", e.what());
	}
}
void FS_LocalVarDeclOrAssign::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	variable_ = &l.get<Variable>(loader.front());
	loader.pop_front();
	value_ = loader.front() ? &l.get<Assignable>(loader.front()) : nullptr;
	loader.pop_front();
}
void FS_LocalVarDeclOrAssign::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_LocalVarDeclOrAssign);
	fw.unsigned_8(declare_local_ ? 1 : 0);
	fw.unsigned_32(variable_->serial());
	fw.unsigned_32(value_ ? value_->serial() : 0);
}
void FS_LocalVarDeclOrAssign::write_lua(int32_t i, FileWrite& fw) const {
	if (declare_local_) {
		fw.print_f("local ");
	}
	variable_->write_lua(i, fw);
	if (value_) {
		fw.print_f(" = ");
		value_->write_lua(i, fw);
	}
}
std::string FS_LocalVarDeclOrAssign::readable() const {
	std::string str = "";
	if (declare_local_) {
		str += "local ";
	}
	str += variable_->get_name();
	if (value_) {
		str += " = " + value_->readable();
	}
	return str;
}
std::set<uint32_t> FS_LocalVarDeclOrAssign::references() const {
	auto set = FunctionStatement::references();
	set.insert(variable_->serial());
	if (value_) {
		set.insert(value_->serial());
	}
	return set;
}
void FS_LocalVarDeclOrAssign::selftest() const {
	FunctionStatement::selftest();
	if (!variable_) {
		throw wexception("variable not set");
	}
	if (value_ && !value_->type().is_subclass(variable_->type())) {
		throw wexception("%s cannot be casted to %s", descname(value_->type()).c_str(),
		                 descname(variable_->type()).c_str());
	}
}

// Function invoking

constexpr uint16_t kCurrentPacketVersionFS_FunctionCall = 1;
void FS_FunctionCall::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_FunctionCall) {
			throw Widelands::UnhandledVersionError(
			   "FS_FunctionCall", packet_version, kCurrentPacketVersionFS_FunctionCall);
		}
		loader.push_back(fr.unsigned_32());
		if (fr.unsigned_8()) {
			function_ = builtin_f(fr.c_string()).function.get();
			assert(function_);
		} else {
			function_ = nullptr;
			loader.push_back(fr.unsigned_32());
		}
		for (uint32_t n = fr.unsigned_32(); n; --n) {
			loader.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("FS_FunctionCall: %s", e.what());
	}
}
void FS_FunctionCall::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	Assignable::load_pointers(l, loader);
	variable_ = loader.front() ? &l.get<Assignable>(loader.front()) : nullptr;
	loader.pop_front();
	if (!function_) {
		function_ = &l.get<LuaFunction>(loader.front());
		loader.pop_front();
	}
	while (!loader.empty()) {
		parameters_.push_back(&l.get<Assignable>(loader.front()));
		loader.pop_front();
	}
}
void FS_FunctionCall::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_FunctionCall);
	fw.unsigned_32(variable_ ? variable_->serial() : 0);
	const std::string f = builtin_f(*function_);
	if (!f.empty()) {
		fw.unsigned_8(1);
		fw.c_string(f.c_str());
	} else {
		fw.unsigned_8(0);
		fw.unsigned_32(dynamic_cast<const LuaFunction&>(*function_).serial());
	}
	fw.unsigned_32(parameters_.size());
	for (const Assignable* p : parameters_) {
		fw.unsigned_32(p->serial());
	}
}
void FS_FunctionCall::selftest() const {
	FunctionStatement::selftest();
	Assignable::selftest();
	if (!function_) {
		throw wexception("no function provided");
	} else if (parameters_.size() != function_->parameters().size()) {
		throw wexception("%" PRIuS " parameters provided, expected %" PRIuS, parameters_.size(),
		                 function_->parameters().size());
	}
	if (function_->get_class().id() == VariableTypeID::Nil) {
		if (variable_) {
			throw wexception("static function cannot be called on a variable");
		}
	} else {
		if (!variable_) {
			throw wexception("non-static function needs to be called on a variable");
		} else if (!function_->get_class().is_subclass(variable_->type())) {
			throw wexception("variable of type %s cannot be casted to %s",
			                 descname(variable_->type()).c_str(),
			                 descname(function_->get_class()).c_str());
		}
	}
	auto it1 = parameters_.begin();
	auto it2 = function_->parameters().begin();
	for (; it1 != parameters_.end(); ++it1, ++it2) {
		if (!(*it1)->type().is_subclass(it2->second)) {
			throw wexception("argument of type %s cannot be casted to %s",
			                 descname((*it1)->type()).c_str(), descname(it2->second).c_str());
		}
	}
}
void FS_FunctionCall::write_lua(int32_t i, FileWrite& fw) const {
	if (variable_) {
		variable_->write_lua(i, fw);
		fw.print_f(":");
	}
	fw.print_f("%s(", function_->get_name().c_str());
	for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
		if (it != parameters_.begin()) {
			fw.print_f(", ");
		}
		(*it)->write_lua(i, fw);
	}
	fw.print_f(")");
}
std::string FS_FunctionCall::readable() const {
	std::string str;
	if (variable_) {
		str += variable_->readable() + ":";
	}
	str += function_->get_name() + "(";
	for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
		if (it != parameters_.begin()) {
			str += ", ";
		}
		str += (*it)->readable();
	}
	return str + ")";
}
std::set<uint32_t> FS_FunctionCall::references() const {
	auto set = FunctionStatement::references();
	if (variable_) {
		set.insert(variable_->serial());
	}
	if (upcast(const LuaFunction, f, function_)) {
		set.insert(f->serial());
	}
	for (const Assignable* a : parameters_) {
		set.insert(a->serial());
	}
	return set;
}

// Property manipulation

constexpr uint16_t kCurrentPacketVersionFS_SetProperty = 1;
void FS_SetProperty::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_SetProperty) {
			throw Widelands::UnhandledVersionError(
			   "FS_SetProperty", packet_version, kCurrentPacketVersionFS_SetProperty);
		}
		loader.push_back(fr.unsigned_32());
		const std::string p = fr.c_string();
		property_ = p.empty() ? nullptr : builtin_p(p).property.get();
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("FS_SetProperty: %s", e.what());
	}
}
void FS_SetProperty::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	variable_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	value_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void FS_SetProperty::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_SetProperty);
	fw.unsigned_32(variable_->serial());
	fw.c_string(property_ ? builtin_p(*property_).c_str() : "");
	fw.unsigned_32(value_->serial());
}
void FS_SetProperty::set_property(Property& p) {
	property_ = &p;
	if (variable_ && !variable_->type().is_subclass(property_->get_class())) {
		variable_ = nullptr;
	}
	if (value_ && !value_->type().is_subclass(property_->get_type())) {
		value_ = nullptr;
	}
}
void FS_SetProperty::set_variable(Assignable& v) {
	variable_ = &v;
	if (property_ && !variable_->type().is_subclass(property_->get_class())) {
		property_ = nullptr;
	}
}
void FS_SetProperty::write_lua(int32_t i, FileWrite& fw) const {
	variable_->write_lua(i, fw);
	fw.print_f(".%s = ", property_->get_name().c_str());
	value_->write_lua(i, fw);
}
std::string FS_SetProperty::readable() const {
	return variable_->readable() + "." + property_->get_name() + " = " + value_->readable();
}
std::set<uint32_t> FS_SetProperty::references() const {
	auto set = FunctionStatement::references();
	set.insert(variable_->serial());
	set.insert(value_->serial());
	return set;
}
void FS_SetProperty::selftest() const {
	FunctionStatement::selftest();
	if (!variable_) {
		throw wexception("variable not set");
	}
	if (!property_) {
		throw wexception("property not set");
	}
	if (!value_) {
		throw wexception("value not set");
	}
	if (!variable_->type().is_subclass(property_->get_class())) {
		throw wexception("variable %s cannot be casted to %s", descname(variable_->type()).c_str(),
		                 descname(property_->get_class()).c_str());
	}
	if (!value_->type().is_subclass(property_->get_type())) {
		throw wexception("value %s cannot be casted to %s", descname(value_->type()).c_str(),
		                 descname(property_->get_type()).c_str());
	}
}

// Table manipulation

constexpr uint16_t kCurrentPacketVersionFS_SetTable = 1;
void FS_SetTable::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_SetTable) {
			throw Widelands::UnhandledVersionError(
			   "FS_SetTable", packet_version, kCurrentPacketVersionFS_SetTable);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("FS_SetTable: %s", e.what());
	}
}
void FS_SetTable::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	table_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	property_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	value_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void FS_SetTable::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_SetTable);
	fw.unsigned_32(table_->serial());
	fw.unsigned_32(property_->serial());
	fw.unsigned_32(value_->serial());
}
void FS_SetTable::set_property(Assignable& p) {
	property_ = &p;
	if (table_ && !property_->type().is_subclass(table_->type().key_type())) {
		table_ = nullptr;
	}
}
void FS_SetTable::set_value(Assignable& v) {
	value_ = &v;
	if (table_ && !value_->type().is_subclass(table_->type().value_type())) {
		table_ = nullptr;
	}
}
void FS_SetTable::set_table(Assignable& t) {
	table_ = &t;
	if (property_ && !property_->type().is_subclass(table_->type().key_type())) {
		property_ = nullptr;
	}
	if (value_ && !value_->type().is_subclass(table_->type().value_type())) {
		value_ = nullptr;
	}
}
void FS_SetTable::selftest() const {
	FunctionStatement::selftest();
	if (!table_) {
		throw wexception("table not set");
	}
	if (!property_) {
		throw wexception("property not set");
	}
	if (!value_) {
		throw wexception("value not set");
	}
	if (!property_->type().is_subclass(table_->type().key_type())) {
		throw wexception("%s cannot be casted to key type %s", descname(property_->type()).c_str(),
		                 descname(table_->type().key_type()).c_str());
	}
	if (!value_->type().is_subclass(table_->type().value_type())) {
		throw wexception("%s cannot be casted to value type %s", descname(value_->type()).c_str(),
		                 descname(table_->type().value_type()).c_str());
	}
}
void FS_SetTable::write_lua(int32_t i, FileWrite& fw) const {
	table_->write_lua(i, fw);
	fw.print_f("[");
	property_->write_lua(i, fw);
	fw.print_f("] = ");
	value_->write_lua(i, fw);
}
std::string FS_SetTable::readable() const {
	return table_->readable() + "[" + property_->readable() + "] = " + value_->readable();
}
std::set<uint32_t> FS_SetTable::references() const {
	auto set = FunctionStatement::references();
	set.insert(table_->serial());
	set.insert(property_->serial());
	set.insert(value_->serial());
	return set;
}

// Coroutine starting

constexpr uint16_t kCurrentPacketVersionFS_LaunchCoroutine = 1;
void FS_LaunchCoroutine::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_LaunchCoroutine) {
			throw Widelands::UnhandledVersionError(
			   "FS_LaunchCoroutine", packet_version, kCurrentPacketVersionFS_LaunchCoroutine);
		}
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("FS_LaunchCoroutine: %s", e.what());
	}
}
void FS_LaunchCoroutine::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	function_ = &l.get<FS_FunctionCall>(loader.front());
	loader.pop_front();
}
void FS_LaunchCoroutine::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_LaunchCoroutine);
	fw.unsigned_32(function_->serial());
}
void FS_LaunchCoroutine::write_lua(int32_t i, FileWrite& fw) const {
	fw.print_f("run(%s", function_->get_function()->get_name().c_str());
	for (const Assignable* p : function_->parameters()) {
		fw.print_f(", ");
		p->write_lua(i, fw);
	}
	fw.print_f(")");
}
std::string FS_LaunchCoroutine::readable() const {
	std::string str = "run(" + function_->get_function()->get_name();
	for (const Assignable* p : function_->parameters()) {
		str += ", " + p->readable();
	}
	return str + ")";
}
std::set<uint32_t> FS_LaunchCoroutine::references() const {
	auto set = FunctionStatement::references();
	set.insert(function_->serial());
	return set;
}
void FS_LaunchCoroutine::selftest() const {
	FunctionStatement::selftest();
	if (!function_) {
		throw wexception("function not set");
	}
}
