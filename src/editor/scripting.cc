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

#include "editor/scripting.h"

#include <memory>

#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"

// Identifier helper functions
bool is_name_valid(const char* name) {
	if (name == nullptr)
		return false;
	if ((name[0] < 'a' || name[0] > 'z') && name[0] != '_')
		return false;
	for (const char* c = name; *c; ++c)
		if ((*c < 'a' || *c > 'z') && (*c < '0' || *c > '9') && *c != '_')
			return false;
	return true;
}
bool is_name_valid(const std::string& name) {
	return is_name_valid(name.c_str());
}
void check_name_valid(const std::string& name) {
	if (!is_name_valid(name)) {
		throw wexception("Invalid variable or function name: %s", name.c_str());
	}
}

/************************************************************
             ScriptingObject, Assignable, and
        FunctionStatement abstract implementations
************************************************************/

uint32_t ScriptingObject::next_serial_ = 0;

constexpr uint16_t kCurrentPacketVersionScriptingObject = 1;
constexpr uint16_t kCurrentPacketVersionAssignable = 1;
constexpr uint16_t kCurrentPacketVersionFunctionStatement = 1;

ScriptingObject::ScriptingObject(ScriptingSaver& s) : serial_(++next_serial_) {
	s.add(*this);
}
void ScriptingObject::load(FileRead& fr, ScriptingLoader&) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScriptingObject) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingObject", packet_version, kCurrentPacketVersionScriptingObject);
		}
		serial_ = fr.unsigned_32();
		next_serial_ = std::max(next_serial_, serial_);
	} catch (const WException& e) {
		throw wexception("editor abstract assignable: %s", e.what());
	}
}

void ScriptingObject::save(FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersionScriptingObject);
	fw.unsigned_16(static_cast<uint16_t>(id()));
	fw.unsigned_32(serial_);
}

// static
ScriptingObject* ScriptingObject::load(FileRead& fr) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScriptingObject) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingObject", packet_version, kCurrentPacketVersionScriptingObject);
		}
		const ID id = static_cast<ID>(fr.unsigned_16());
		switch (id) {
		case ID::ConstexprString:
			return new ConstexprString();
		case ID::ConstexprInteger:
			return new ConstexprInteger();
		case ID::ConstexprBoolean:
			return new ConstexprBoolean();
		case ID::ConstexprNil:
			return new ConstexprNil();
		case ID::StringConcat:
			return new StringConcat();
		case ID::Variable:
			return new Variable();
		case ID::FSLocalVarDeclOrAssign:
			return new FS_LocalVarDeclOrAssign();
		case ID::FSPrint:
			return new FS_Print();
		default:
			throw Widelands::GameDataError("Invalid ScriptingObject ID");
		}
	} catch (const WException& e) {
		throw wexception("editor abstract scripting object: %s", e.what());
	}
}

/************************************************************
                 Constexpr implementations
************************************************************/

constexpr uint16_t kCurrentPacketVersionConstexprString = 1;
constexpr uint16_t kCurrentPacketVersionConstexprInteger = 1;
constexpr uint16_t kCurrentPacketVersionConstexprBoolean = 1;

void ConstexprString::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionConstexprString) {
			throw Widelands::UnhandledVersionError(
			   "ConstexprString", packet_version, kCurrentPacketVersionConstexprString);
		}
		value_ = fr.c_string();
		translate_ = fr.unsigned_8();
	} catch (const WException& e) {
		throw wexception("editor constexpr string: %s", e.what());
	}
}
void ConstexprInteger::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionConstexprInteger) {
			throw Widelands::UnhandledVersionError(
			   "ConstexprInteger", packet_version, kCurrentPacketVersionConstexprInteger);
		}
		value_ = fr.signed_32();
	} catch (const WException& e) {
		throw wexception("editor constexpr integer: %s", e.what());
	}
}
void ConstexprBoolean::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionConstexprBoolean) {
			throw Widelands::UnhandledVersionError(
			   "ConstexprBoolean", packet_version, kCurrentPacketVersionConstexprBoolean);
		}
		value_ = fr.unsigned_8();
	} catch (const WException& e) {
		throw wexception("editor constexpr boolean: %s", e.what());
	}
}

void ConstexprString::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionConstexprString);
	fw.c_string(value_.c_str());
	fw.unsigned_8(translate_ ? 1 : 0);
}
void ConstexprInteger::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionConstexprInteger);
	fw.signed_32(value_);
}
void ConstexprBoolean::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionConstexprBoolean);
	fw.unsigned_8(value_ ? 1 : 0);
}

int32_t ConstexprString::write_lua(FileWrite& fw) const {
	fw.print_f(translate_ ? "_(\"%s\")" : "\"%s\"", value_.c_str());
	return 0;
}
int32_t ConstexprInteger::write_lua(FileWrite& fw) const {
	fw.print_f("%i", value_);
	return 0;
}
int32_t ConstexprBoolean::write_lua(FileWrite& fw) const {
	fw.print_f(value_ ? "true" : "false");
	return 0;
}
int32_t ConstexprNil::write_lua(FileWrite& fw) const {
	fw.print_f("nil");
	return 0;
}

/************************************************************
                StringConcat implementation
************************************************************/

constexpr uint16_t kCurrentPacketVersionStringConcat = 1;
StringConcat::StringConcat(ScriptingSaver& s, size_t argc, Assignable** argv) : Assignable(s) {
	for (size_t i = 0; i < argc; ++i) {
		append(argv[i]);
	}
}
void StringConcat::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionStringConcat) {
			throw Widelands::UnhandledVersionError(
			   "StringConcat", packet_version, kCurrentPacketVersionStringConcat);
		}
		StringConcat::Loader& loader = l.loader<StringConcat::Loader>(this);
		for (uint32_t n = fr.unsigned_32(); n; --n) {
			loader.values.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor string concatenation: %s", e.what());
	}
}
void StringConcat::load_pointers(ScriptingLoader& l) {
	Assignable::load_pointers(l);
	StringConcat::Loader& loader = l.loader<StringConcat::Loader>(this);
	for (uint32_t s : loader.values) {
		values_.push_back(&l.get<Assignable>(s));
	}
}
void StringConcat::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionStringConcat);
	fw.unsigned_32(values_.size());
	for (const Assignable* a : values_) {
		fw.unsigned_32(a->serial());
	}
}
int32_t StringConcat::write_lua(FileWrite& fw) const {
	if (values_.empty()) {
		fw.print_f("\"\"");
	} else {
		for (auto it = values_.begin(); it != values_.end(); ++it) {
			if (it != values_.begin()) {
				fw.print_f(" .. ");
			}
			(*it)->write_lua(fw);
		}
	}
	return 0;
}

std::string StringConcat::readable() const {
	if (values_.empty()) {
		return "\"\"";
	} else {
		std::string res = "";
		for (auto it = values_.begin(); it != values_.end(); ++it) {
			if (it != values_.begin()) {
				res += " .. ";  // No i18n markup for Lua syntax
			}
			res += (*it)->readable();
		}
		return res;
	}
}

/************************************************************
                 Variable implementation
************************************************************/

constexpr uint16_t kCurrentPacketVersionVariable = 1;

Variable::Variable(ScriptingSaver& s, VariableType t, const std::string& n)
   : Assignable(s), type_(t), name_(n) {
	check_name_valid(name_);
	assert(type_ != VariableType::kInvalidType);
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
		if (type_ == VariableType::kInvalidType) {
			throw Widelands::GameDataError("kInvalidType");
		}
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
                  Functions implementation
************************************************************/

constexpr uint16_t kCurrentPacketVersionFunction = 1;
Function::Function(ScriptingSaver& s, const std::string& n, bool a)
   : ScriptingObject(s), name_(n), autostart_(a) {
	check_name_valid(name_);
}

void Function::load(FileRead& fr, ScriptingLoader& l) {
	try {
		ScriptingObject::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFunction) {
			throw Widelands::UnhandledVersionError(
			   "Function", packet_version, kCurrentPacketVersionFunction);
		}
		name_ = fr.c_string();
		check_name_valid(name_);
		autostart_ = fr.unsigned_8();
		for (size_t n = fr.unsigned_32(); n; --n) {
			const std::string v(fr.c_string());
			parameters_.push_back(std::make_pair(v, static_cast<VariableType>(fr.unsigned_16())));
		}
		Function::Loader& loader = l.loader<Function::Loader>(this);
		for (size_t n = fr.unsigned_32(); n; --n) {
			loader.body.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor scripting function: %s", e.what());
	}
}

void Function::load_pointers(ScriptingLoader& l) {
	ScriptingObject::load_pointers(l);
	Function::Loader& loader = l.loader<Function::Loader>(this);
	for (uint32_t s : loader.body) {
		body_.push_back(&l.get<FunctionStatement>(s));
	}
}

void Function::save(FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersionFunction);
	fw.c_string(name_);
	fw.unsigned_8(autostart_ ? 1 : 0);

	fw.unsigned_32(parameters_.size());
	for (const auto& pair : parameters_) {
		fw.c_string(pair.first.c_str());
		fw.unsigned_16(static_cast<uint16_t>(pair.second));
	}

	fw.unsigned_32(body_.size());
	for (const auto& f : body_) {
		f->save(fw);
	}
}

std::string Function::header() const {
	std::string s = "function " + name_ + "(";
	const size_t nr_params = parameters_.size();
	if (nr_params) {
		auto it = parameters_.begin();
		s += it->first;
		for (size_t i = 1; i < nr_params; ++i) {
			++it;
			s += ", " + it->first;
		}
		assert(it == parameters_.end());
	}
	return s + ")";
}

int32_t Function::write_lua(FileWrite& fw) const {
	fw.print_f("\n%s\n", header().c_str());
	int32_t indent = 1;
	for (const auto& f : body_) {
		for (int32_t i = 0; i < indent; ++i) {
			fw.print_f("   ");
		}
		indent += f->write_lua(fw);
		fw.print_f("\n");
	}
	fw.print_f("end\n");
	return 0;
}

std::string Function::readable() const {
	return header();
}

/************************************************************
          Specific function statement implementations
************************************************************/

constexpr uint16_t kCurrentPacketVersionFS_LocalVarDeclOrAssign = 1;
FS_LocalVarDeclOrAssign::FS_LocalVarDeclOrAssign(ScriptingSaver& s,
                                                 bool l,
                                                 Variable& var,
                                                 Assignable* val)
   : FunctionStatement(s), variable_(&var), value_(val), declare_local_(l) {
}
void FS_LocalVarDeclOrAssign::load(FileRead& fr, ScriptingLoader& l) {
	try {
		FunctionStatement::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_LocalVarDeclOrAssign) {
			throw Widelands::UnhandledVersionError("FS_LocalVarDeclOrAssign", packet_version,
			                                       kCurrentPacketVersionFS_LocalVarDeclOrAssign);
		}
		declare_local_ = fr.unsigned_8();
		FS_LocalVarDeclOrAssign::Loader& loader = l.loader<FS_LocalVarDeclOrAssign::Loader>(this);
		loader.var = fr.unsigned_32();
		loader.val = fr.unsigned_32();
	} catch (const WException& e) {
		throw wexception("FS_LocalVarDeclOrAssign: %s", e.what());
	}
}
void FS_LocalVarDeclOrAssign::load_pointers(ScriptingLoader& l) {
	FunctionStatement::load_pointers(l);
	FS_LocalVarDeclOrAssign::Loader& loader = l.loader<FS_LocalVarDeclOrAssign::Loader>(this);
	variable_ = &l.get<Variable>(loader.var);
	value_ = loader.val ? &l.get<Assignable>(loader.val) : nullptr;
}
void FS_LocalVarDeclOrAssign::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_LocalVarDeclOrAssign);
	fw.unsigned_8(declare_local_ ? 1 : 0);
	fw.unsigned_32(variable_->serial());
	fw.unsigned_32(value_ ? value_->serial() : 0);
}
int32_t FS_LocalVarDeclOrAssign::write_lua(FileWrite& fw) const {
	if (declare_local_) {
		fw.print_f("local ");
	}
	assert(variable_);
	variable_->write_lua(fw);
	if (value_) {
		fw.print_f(" = ");
		value_->write_lua(fw);
	}
	return 0;
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

constexpr uint16_t kCurrentPacketVersionFS_Print = 1;
void FS_Print::load(FileRead& fr, ScriptingLoader& l) {
	try {
		FunctionStatement::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_Print) {
			throw Widelands::UnhandledVersionError(
			   "FS_Print", packet_version, kCurrentPacketVersionFS_Print);
		}
		FS_Print::Loader& loader = l.loader<FS_Print::Loader>(this);
		loader.text = fr.unsigned_32();
	} catch (const WException& e) {
		throw wexception("FS_Print: %s", e.what());
	}
}
void FS_Print::load_pointers(ScriptingLoader& l) {
	FunctionStatement::load_pointers(l);
	FS_Print::Loader& loader = l.loader<FS_Print::Loader>(this);
	text_ = loader.text ? &l.get<Assignable>(loader.text) : nullptr;
}
void FS_Print::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_Print);
	fw.unsigned_32(text_ ? text_->serial() : 0);
}
int32_t FS_Print::write_lua(FileWrite& fw) const {
	fw.print_f("print(");
	if (text_) {
		text_->write_lua(fw);
	}
	fw.print_f(")");
	return 0;
}
std::string FS_Print::readable() const {
	std::string str = "print(";
	if (text_) {
		str += text_->readable();
	}
	return str + ")";
}

/************************************************************
             VariableType static helper functions
************************************************************/

// static
std::string descname(VariableType t) {
	switch (t) {
	case VariableType::Integer:
		return _("Integer");
	case VariableType::Boolean:
		return _("Boolean");
	case VariableType::String:
		return _("String");
	case VariableType::Table:
		return _("Table");
	case VariableType::Game:
		return _("Game");
	case VariableType::Map:
		return _("Map");
	case VariableType::Field:
		return _("Field");
	case VariableType::Player:
		return _("Player");
	case VariableType::Message:
		return _("Message");
	case VariableType::Objective:
		return _("Objective");
	case VariableType::MapObject:
		return _("Map Object");
	case VariableType::BaseImmovable:
		return _("Generic Immovable");
	case VariableType::Immovable:
		return _("Map Immovable");
	case VariableType::PlayerImmovable:
		return _("Generic Player Immovable");
	case VariableType::Flag:
		return _("Flag");
	case VariableType::Building:
		return _("Generic Building");
	case VariableType::Warehouse:
		return _("Warehouse");
	case VariableType::MilitarySite:
		return _("Militarysite");
	case VariableType::ProductionSite:
		return _("Productionsite");
	case VariableType::TrainingSite:
		return _("Trainingsite");
	case VariableType::PartiallyFinishedBuilding:
		return _("Generic Partially Finished Building");
	case VariableType::ConstructionSite:
		return _("Constructionsite");
	case VariableType::DismantleSite:
		return _("Dismantlesite");
	case VariableType::Market:
		return _("Market");
	case VariableType::RoadBase:
		return _("Generic Road");
	case VariableType::Road:
		return _("Road");
	case VariableType::Waterway:
		return _("Waterway");
	case VariableType::Bob:
		return _("Bob");
	case VariableType::Worker:
		return _("Worker");
	case VariableType::Carrier:
		return _("Carrier");
	case VariableType::Ferry:
		return _("Ferry");
	case VariableType::Soldier:
		return _("Soldier");
	case VariableType::Ship:
		return _("Ship");
	case VariableType::MapObjectDescr:
		return _("Map Object Description");
	case VariableType::BaseImmovableDescr:
		return _("Generic Immovable Description");
	case VariableType::ImmovableDescr:
		return _("Map Immovable Description");
	case VariableType::PlayerImmovableDescr:
		return _("Generic Player Immovable Description");
	case VariableType::FlagDescr:
		return _("Flag Description");
	case VariableType::BuildingDescr:
		return _("Generic Building Description");
	case VariableType::WarehouseDescr:
		return _("Warehouse Description");
	case VariableType::MilitarySiteDescr:
		return _("Militarysite Description");
	case VariableType::ProductionSiteDescr:
		return _("Productionsite Description");
	case VariableType::TrainingSiteDescr:
		return _("Trainingsite Description");
	case VariableType::PartiallyFinishedBuildingDescr:
		return _("Generic Partially Finished Building Description");
	case VariableType::ConstructionSiteDescr:
		return _("Constructionsite Description");
	case VariableType::DismantleSiteDescr:
		return _("Dismantlesite Description");
	case VariableType::MarketDescr:
		return _("Market Description");
	case VariableType::RoadBaseDescr:
		return _("Generic Road Description");
	case VariableType::RoadDescr:
		return _("Road Description");
	case VariableType::WaterwayDescr:
		return _("Waterway Description");
	case VariableType::BobDescr:
		return _("Bob Description");
	case VariableType::WorkerDescr:
		return _("Worker Description");
	case VariableType::CarrierDescr:
		return _("Carrier Description");
	case VariableType::FerryDescr:
		return _("Ferry Description");
	case VariableType::SoldierDescr:
		return _("Soldier Description");
	case VariableType::ShipDescr:
		return _("Ship Description");
	case VariableType::TerrainDescr:
		return _("Terrain Description");
	case VariableType::ResourceDescr:
		return _("Resource Description");
	default:
		NEVER_HERE();
	}
}

// Checks whether a variable of type s may be assigned a value of type t.
// This is the case if t==s, or t is a subclass of s.
// We do so by recursively calling is(s, direct_superclass_of_t)
// static
bool is(VariableType t, VariableType s) {
	if (t == s) {
		return true;
	}
	switch (t) {
	case VariableType::Integer:
	case VariableType::Boolean:
	case VariableType::Table:
	case VariableType::String:
	case VariableType::Game:
	case VariableType::Map:
	case VariableType::Field:
	case VariableType::Player:
	case VariableType::Message:
	case VariableType::Objective:
	case VariableType::MapObject:
	case VariableType::MapObjectDescr:
	case VariableType::TerrainDescr:
	case VariableType::ResourceDescr:
		return false;

	case VariableType::BaseImmovable:
	case VariableType::Bob:
		return s == VariableType::MapObject;
	case VariableType::Immovable:
	case VariableType::PlayerImmovable:
		return is(s, VariableType::BaseImmovable);
	case VariableType::Flag:
	case VariableType::Building:
	case VariableType::RoadBase:
		return is(s, VariableType::PlayerImmovable);
	case VariableType::Warehouse:
	case VariableType::MilitarySite:
	case VariableType::ProductionSite:
	case VariableType::PartiallyFinishedBuilding:
	case VariableType::Market:
		return is(s, VariableType::Building);
	case VariableType::TrainingSite:
		return is(s, VariableType::ProductionSite);
	case VariableType::ConstructionSite:
	case VariableType::DismantleSite:
		return is(s, VariableType::PartiallyFinishedBuilding);
	case VariableType::Road:
	case VariableType::Waterway:
		return is(s, VariableType::RoadBase);
	case VariableType::Ship:
	case VariableType::Worker:
		return is(s, VariableType::Bob);
	case VariableType::Soldier:
	case VariableType::Carrier:
		return is(s, VariableType::Worker);
	case VariableType::Ferry:
		return is(s, VariableType::Carrier);
	case VariableType::BaseImmovableDescr:
	case VariableType::BobDescr:
		return s == VariableType::MapObjectDescr;
	case VariableType::ImmovableDescr:
	case VariableType::PlayerImmovableDescr:
		return is(s, VariableType::BaseImmovableDescr);
	case VariableType::FlagDescr:
	case VariableType::BuildingDescr:
	case VariableType::RoadBaseDescr:
		return is(s, VariableType::PlayerImmovableDescr);
	case VariableType::WarehouseDescr:
	case VariableType::MilitarySiteDescr:
	case VariableType::ProductionSiteDescr:
	case VariableType::PartiallyFinishedBuildingDescr:
	case VariableType::MarketDescr:
		return is(s, VariableType::BuildingDescr);
	case VariableType::TrainingSiteDescr:
		return is(s, VariableType::ProductionSiteDescr);
	case VariableType::ConstructionSiteDescr:
	case VariableType::DismantleSiteDescr:
		return is(s, VariableType::PartiallyFinishedBuildingDescr);
	case VariableType::RoadDescr:
	case VariableType::WaterwayDescr:
		return is(s, VariableType::RoadBaseDescr);
	case VariableType::WorkerDescr:
	case VariableType::ShipDescr:
		return is(s, VariableType::BobDescr);
	case VariableType::SoldierDescr:
	case VariableType::CarrierDescr:
		return is(s, VariableType::WorkerDescr);
	case VariableType::FerryDescr:
		return is(s, VariableType::CarrierDescr);
	default:
		NEVER_HERE();
	}
}

/************************************************************
                   Saveloading helper
************************************************************/

constexpr uint16_t kCurrentPacketVersionScripting = 1;

void ScriptingSaver::add(ScriptingObject& a) {
	for (const auto& o : list_) {
		if (o.get() == &a) {
			throw Widelands::GameDataError("ScriptingObject %u was already declared", a.serial());
		} else if (o->serial() == a.serial()) {
			throw Widelands::GameDataError(
			   "Two scripting objects have the same serial %u", a.serial());
		}
	}
	list_.push_back(std::unique_ptr<ScriptingObject>(&a));
}

void ScriptingSaver::save(FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersionScripting);
	fw.unsigned_32(list_.size());
	for (const auto& a : list_) {
		a->save(fw);
	}
}

ScriptingLoader::ScriptingLoader(FileRead& fr, ScriptingSaver& s) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScripting) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingSaver", packet_version, kCurrentPacketVersionScripting);
		}

		// First load phase: Create all saved objects.
		for (uint32_t n = fr.unsigned_32(); n; --n) {
			ScriptingObject* a = ScriptingObject::load(fr);
			list_.emplace(
			   std::make_pair(a, std::unique_ptr<ScriptingObject::Loader>(a->create_loader())));
			a->load(fr, *this);
			s.add(*a);
		}
		// Second load phase: Load pointers between objects.
		for (const auto& pair : list_) {
			pair.first->load_pointers(*this);
		}
	} catch (const WException& e) {
		throw wexception("ScriptingLoader: %s", e.what());
	}
}
