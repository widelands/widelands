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

Variable::Variable(VariableType t, const std::string& n, bool spellcheck) : type_(t), name_(n) {
	if (spellcheck)
		check_name_valid(name_);
}

void Variable::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionVariable) {
			throw Widelands::UnhandledVersionError(
			   "Variable", packet_version, kCurrentPacketVersionVariable);
		}
		type_ = static_cast<VariableType>(fr.unsigned_16());
		name_ = fr.c_string();
		check_name_valid(name_);
	} catch (const WException& e) {
		throw wexception("editor scripting variable: %s", e.what());
	}
}

void Variable::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionVariable);
	fw.unsigned_16(static_cast<uint16_t>(type_));
	fw.c_string(name_.c_str());
}

int32_t Variable::write_lua(FileWrite& fw) const {
	fw.print_f("%s", name_.c_str());
	return 0;
}

/************************************************************
                     Property access
************************************************************/

constexpr uint16_t kCurrentPacketVersionGetProperty = 1;
void GetProperty::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionGetProperty) {
			throw Widelands::UnhandledVersionError(
			   "GetProperty", packet_version, kCurrentPacketVersionGetProperty);
		}
		GetProperty::Loader& loader = l.loader<GetProperty::Loader>(this);
		loader.var = fr.unsigned_32();
		loader.prop = fr.unsigned_32();
	} catch (const WException& e) {
		throw wexception("GetProperty: %s", e.what());
	}
}
void GetProperty::load_pointers(ScriptingLoader& l) {
	Assignable::load_pointers(l);
	GetProperty::Loader& loader = l.loader<GetProperty::Loader>(this);
	variable_ = &l.get<Assignable>(loader.var);
	property_ = kBuiltinProperties[loader.prop]->property.get();
}
void GetProperty::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionGetProperty);
	assert(variable_);
	assert(property_);
	fw.unsigned_32(variable_->serial());
	fw.unsigned_32(property_to_serial(*property_));
}
VariableType GetProperty::type() const {
	assert(property_);
	return property_->get_type();
}
void GetProperty::set_property(Property& p) {
	property_ = &p;
	if (variable_ && !is(variable_->type(), property_->get_class())) {
		variable_ = nullptr;
	}
}
void GetProperty::set_variable(Assignable& v) {
	variable_ = &v;
	if (property_ && !is(variable_->type(), property_->get_class())) {
		property_ = nullptr;
	}
}
int32_t GetProperty::write_lua(FileWrite& fw) const {
	assert(variable_);
	assert(property_);
	assert(is(variable_->type(), property_->get_class()));
	variable_->write_lua(fw);
	fw.print_f(".%s", property_->get_name().c_str());
	return 0;
}
std::string GetProperty::readable() const {
	assert(variable_);
	assert(property_);
	return variable_->readable() + "." + property_->get_name();
}
std::set<uint32_t> GetProperty::references() const {
	auto set = Assignable::references();
	assert(variable_);
	set.insert(variable_->serial());
	return set;
}
