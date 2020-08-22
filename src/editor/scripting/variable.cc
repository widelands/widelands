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

#include "editor/scripting/variable.h"

#include "editor/scripting/builtin.h"

/************************************************************
                 Variable implementation
************************************************************/

constexpr uint16_t kCurrentPacketVersionVariable = 1;

void Variable::selftest() const {
	Assignable::selftest();
	check_name_valid(name_);
	if (type_.id() == VariableTypeID::Nil) {
		throw wexception("type is nil");
	} else if (type_.id() == VariableTypeID::Any) {
		throw wexception("type is any");
	}
}

void Variable::load(FileRead& fr, Loader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionVariable) {
			throw Widelands::UnhandledVersionError(
			   "Variable", packet_version, kCurrentPacketVersionVariable);
		}
		type_ = VariableType(fr);
		name_ = fr.c_string();
	} catch (const WException& e) {
		throw wexception("editor scripting variable: %s", e.what());
	}
}

void Variable::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionVariable);
	type_.write(fw);
	fw.c_string(name_.c_str());
}

void Variable::write_lua(int32_t, FileWrite& fw) const {
	fw.print_f("%s", name_.c_str());
}

/************************************************************
                     Property access
************************************************************/

constexpr uint16_t kCurrentPacketVersionGetProperty = 1;
void GetProperty::load(FileRead& fr, Loader& loader) {
	try {
		Assignable::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionGetProperty) {
			throw Widelands::UnhandledVersionError(
			   "GetProperty", packet_version, kCurrentPacketVersionGetProperty);
		}
		loader.push_back(fr.unsigned_32());
		const std::string p = fr.c_string();
		property_ = p.empty() ? nullptr : builtin_p(p).property.get();
	} catch (const WException& e) {
		throw wexception("GetProperty: %s", e.what());
	}
}
void GetProperty::load_pointers(const ScriptingLoader& l, Loader& loader) {
	Assignable::load_pointers(l, loader);
	variable_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void GetProperty::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionGetProperty);
	fw.unsigned_32(variable_->serial());
	fw.c_string(property_ ? builtin_p(*property_).c_str() : "");
}
const VariableType& GetProperty::type() const {
	return property_->get_type();
}
void GetProperty::set_property(Property& p) {
	property_ = &p;
	if (variable_ && !variable_->type().is_subclass(property_->get_class())) {
		variable_ = nullptr;
	}
}
void GetProperty::set_variable(Assignable& v) {
	variable_ = &v;
	if (property_ && !variable_->type().is_subclass(property_->get_class())) {
		property_ = nullptr;
	}
}
void GetProperty::write_lua(int32_t i, FileWrite& fw) const {
	variable_->write_lua(i, fw);
	fw.print_f(".%s", property_->get_name().c_str());
}
void GetProperty::selftest() const {
	Assignable::selftest();
	if (!variable_) {
		throw wexception("variable not set");
	}
	if (!property_) {
		throw wexception("property not set");
	}
	if (!variable_->type().is_subclass(property_->get_class())) {
		throw wexception("%s cannot be casted to %s", descname(variable_->type()).c_str(),
		                 descname(property_->get_class()).c_str());
	}
}
std::string GetProperty::readable() const {
	return variable_->readable() + "." + property_->get_name();
}
std::set<uint32_t> GetProperty::references() const {
	auto set = Assignable::references();
	set.insert(variable_->serial());
	return set;
}

/************************************************************
                         Table access
************************************************************/

constexpr uint16_t kCurrentPacketVersionGetTable = 1;
void GetTable::load(FileRead& fr, Loader& loader) {
	try {
		Assignable::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionGetTable) {
			throw Widelands::UnhandledVersionError(
			   "GetTable", packet_version, kCurrentPacketVersionGetTable);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("GetTable: %s", e.what());
	}
}
void GetTable::load_pointers(const ScriptingLoader& l, Loader& loader) {
	Assignable::load_pointers(l, loader);
	table_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	property_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void GetTable::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionGetTable);
	fw.unsigned_32(table_->serial());
	fw.unsigned_32(property_->serial());
}
const VariableType& GetTable::type() const {
	return table_->type().value_type();
}
void GetTable::set_table(Assignable& t) {
	table_ = &t;
	if (property_ && !property_->type().is_subclass(table_->type().key_type())) {
		property_ = nullptr;
	}
}
void GetTable::set_property(Assignable& p) {
	property_ = &p;
	if (table_ && !property_->type().is_subclass(table_->type().key_type())) {
		table_ = nullptr;
	}
}
void GetTable::write_lua(int32_t i, FileWrite& fw) const {
	table_->write_lua(i, fw);
	// We do not use 'x.y' syntax even if we have a string as key type, because figuring
	// out whether we have a string literal here and telling it not to use quotation marks
	// would be more trouble than it's worth. So we just write 'x["y"]' instead.
	fw.print_f("[");
	property_->write_lua(i, fw);
	fw.print_f("]");
}
void GetTable::selftest() const {
	Assignable::selftest();
	if (!table_) {
		throw wexception("table not set");
	}
	if (!property_) {
		throw wexception("property not set");
	}
	if (!property_->type().is_subclass(table_->type().key_type())) {
		throw wexception("%s cannot be casted to %s", descname(property_->type()).c_str(),
		                 descname(table_->type().key_type()).c_str());
	}
}
std::string GetTable::readable() const {
	return table_->readable() + "[" + property_->readable() + "]";
}
std::set<uint32_t> GetTable::references() const {
	auto set = Assignable::references();
	set.insert(table_->serial());
	set.insert(property_->serial());
	return set;
}
