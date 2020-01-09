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

#include "base/log.h"
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

constexpr uint16_t kCurrentPacketVersionScriptingObjectHeader = 1;
constexpr uint16_t kCurrentPacketVersionScriptingObject = 1;

void ScriptingObject::init(ScriptingSaver& s, bool init_serial) {
	if (init_serial) {
		if (serial_) {
			throw wexception("ScriptingObject::init: %u already initialized", serial_);
		}
		serial_ = (++next_serial_);
	} else {
		if (!serial_) {
			throw wexception("ScriptingObject::init: not initialized yet");
		}
	}
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
	fw.unsigned_16(kCurrentPacketVersionScriptingObjectHeader);
	fw.unsigned_16(static_cast<uint16_t>(id()));
	fw.unsigned_16(kCurrentPacketVersionScriptingObject);
	fw.unsigned_32(serial_);
}

// static
ScriptingObject* ScriptingObject::load(FileRead& fr) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScriptingObjectHeader) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingObject Header", packet_version, kCurrentPacketVersionScriptingObjectHeader);
		}
		const uint16_t id = fr.unsigned_16();
		switch (static_cast<ID>(id)) {
		case ID::ConstexprString:
			return new ConstexprString("");
		case ID::ConstexprInteger:
			return new ConstexprInteger(0);
		case ID::ConstexprBoolean:
			return new ConstexprBoolean(false);
		case ID::ConstexprNil:
			return new ConstexprNil();
		case ID::StringConcat:
			return new StringConcat({});
		case ID::Variable:
			return new Variable(VariableType::Nil, "", false);
		case ID::LuaFunction:
			return new LuaFunction("", false);
		case ID::FSFunctionCall:
			return new FS_FunctionCall(nullptr, nullptr, {});
		case ID::FSLocalVarDeclOrAssign:
			return new FS_LocalVarDeclOrAssign(false, nullptr, nullptr);
		case ID::FSLaunchCoroutine:
			return new FS_LaunchCoroutine(nullptr);
		default:
			throw Widelands::GameDataError("Invalid ScriptingObject ID: %u", id);
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
                  Functions implementation
************************************************************/

FunctionBase::FunctionBase(const std::string& n,
                           VariableType c,
                           VariableType r,
                           std::list<std::pair<std::string, VariableType>> p,
                           bool spellcheck)
   : parameters_(p), name_(n), class_(c), returns_(r) {
	if (spellcheck)
		check_name_valid(name_);
}
FunctionBase::FunctionBase(const std::string& n, bool spellcheck)
   : name_(n), class_(VariableType::Nil), returns_(VariableType::Nil) {
	if (spellcheck)
		check_name_valid(name_);
}

std::string FunctionBase::header(bool lua) const {
	std::string s = lua ? "function" : descname(returns_);
	s += " " + name_ + "(";
	for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
		if (it != parameters_.begin()) {
			s += ", ";
		}
		if (!lua) {
			s += descname(it->second) + " ";
		}
		s += it->first;
	}
	return s + ")";
}

constexpr uint16_t kCurrentPacketVersionFunction = 1;

void LuaFunction::load(FileRead& fr, ScriptingLoader& l) {
	try {
		ScriptingObject::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFunction) {
			throw Widelands::UnhandledVersionError(
			   "LuaFunction", packet_version, kCurrentPacketVersionFunction);
		}
		rename(fr.c_string());
		set_returns(static_cast<VariableType>(fr.unsigned_16()));
		for (size_t n = fr.unsigned_32(); n; --n) {
			const std::string v(fr.c_string());
			parameters_.push_back(std::make_pair(v, static_cast<VariableType>(fr.unsigned_16())));
		}
		LuaFunction::Loader& loader = l.loader<LuaFunction::Loader>(this);
		for (size_t n = fr.unsigned_32(); n; --n) {
			loader.body.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor scripting lua function: %s", e.what());
	}
}

void LuaFunction::load_pointers(ScriptingLoader& l) {
	ScriptingObject::load_pointers(l);
	LuaFunction::Loader& loader = l.loader<LuaFunction::Loader>(this);
	for (uint32_t s : loader.body) {
		body_.push_back(&l.get<FunctionStatement>(s));
	}
}

void LuaFunction::save(FileWrite& fw) const {
	ScriptingObject::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFunction);
	fw.c_string(get_name().c_str());
	fw.unsigned_16(static_cast<uint16_t>(get_returns()));

	fw.unsigned_32(parameters_.size());
	for (const auto& pair : parameters_) {
		fw.c_string(pair.first.c_str());
		fw.unsigned_16(static_cast<uint16_t>(pair.second));
	}

	fw.unsigned_32(body_.size());
	for (const auto& f : body_) {
		fw.unsigned_32(f->serial());
	}
}

int32_t LuaFunction::write_lua(FileWrite& fw) const {
	fw.print_f("\n%s\n", header(true).c_str());
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

// static
int32_t function_to_serial(FunctionBase& f) {
	if (upcast(ScriptingObject, so, &f)) {
		return so->serial();
	}
	for (int32_t i = 0;; ++i) {
		if (kBuiltinFunctions[i]->function.get() == &f) {
			return -i;
		}
	}
	NEVER_HERE();
}
// static
inline FunctionBase& serial_to_function(ScriptingLoader& l, int32_t s) {
	return s > 0 ? l.get<LuaFunction>(s) : *kBuiltinFunctions[-s]->function;
}

/************************************************************
          Specific function statement implementations
************************************************************/

// Variable declaration and assignment

constexpr uint16_t kCurrentPacketVersionFS_LocalVarDeclOrAssign = 1;
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

// Function invoking

constexpr uint16_t kCurrentPacketVersionFS_FunctionCall = 1;
void FS_FunctionCall::load(FileRead& fr, ScriptingLoader& l) {
	try {
		FunctionStatement::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_FunctionCall) {
			throw Widelands::UnhandledVersionError(
			   "FS_FunctionCall", packet_version, kCurrentPacketVersionFS_FunctionCall);
		}
		FS_FunctionCall::Loader& loader = l.loader<FS_FunctionCall::Loader>(this);
		loader.var = fr.unsigned_32();
		loader.func = fr.signed_32();
		for (uint32_t n = fr.unsigned_32(); n; --n) {
			loader.params.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("FS_FunctionCall: %s", e.what());
	}
}
void FS_FunctionCall::load_pointers(ScriptingLoader& l) {
	FunctionStatement::load_pointers(l);
	Assignable::load_pointers(l);
	FS_FunctionCall::Loader& loader = l.loader<FS_FunctionCall::Loader>(this);
	variable_ = loader.var ? &l.get<Variable>(loader.var) : nullptr;
	function_ = &serial_to_function(l, loader.func);
	for (uint32_t s : loader.params) {
		parameters_.push_back(&l.get<Assignable>(s));
	}
}
void FS_FunctionCall::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_FunctionCall);
	fw.unsigned_32(variable_ ? variable_->serial() : 0);
	assert(function_);
	fw.signed_32(function_to_serial(*function_));
	fw.unsigned_32(parameters_.size());
	for (const Assignable* p : parameters_) {
		fw.unsigned_32(p->serial());
	}
}
void FS_FunctionCall::check_parameters() const {
	if (!function_) {
		throw wexception("FS_FunctionCall: No function provided");
	} else if (parameters_.size() != function_->parameters().size()) {
		throw wexception("FS_FunctionCall %s: %" PRIuS " parameters provided, expected %" PRIuS,
		                 function_->get_name().c_str(), parameters_.size(),
		                 function_->parameters().size());
	}
	if (function_->get_class() == VariableType::Nil) {
		if (variable_) {
			throw wexception("FS_FunctionCall %s: static function cannot be called on a variable",
			                 function_->get_name().c_str());
		}
	} else {
		if (!variable_) {
			throw wexception(
			   "FS_FunctionCall %s: non-static function needs to be called on a variable",
			   function_->get_name().c_str());
		} else if (!is(function_->get_class(), variable_->type())) {
			throw wexception("FS_FunctionCall %s: variable of type %s cannot be casted to %s",
			                 function_->get_name().c_str(), typeid(variable_->type()).name(),
			                 typeid(function_->get_class()).name());
		}
	}
	auto it1 = parameters_.begin();
	auto it2 = function_->parameters().begin();
	for (; it1 != parameters_.end(); ++it1, ++it2) {
		if (!is((*it1)->type(), it2->second)) {
			throw wexception("FS_FunctionCall %s: argument of type %s cannot be casted to %s",
			                 function_->get_name().c_str(), typeid((*it1)->type()).name(),
			                 typeid(it2->second).name());
		}
	}
}
int32_t FS_FunctionCall::write_lua(FileWrite& fw) const {
	check_parameters();
	if (variable_) {
		fw.print_f("%s:", variable_->get_name().c_str());
	}
	fw.print_f("%s(", function_->get_name().c_str());
	for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
		if (it != parameters_.begin()) {
			fw.print_f(", ");
		}
		(*it)->write_lua(fw);
	}
	fw.print_f(")");
	return 0;
}
std::string FS_FunctionCall::readable() const {
	std::string str;
	if (variable_) {
		str += variable_->get_name() + ":";
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

// Coroutine starting

constexpr uint16_t kCurrentPacketVersionFS_LaunchCoroutine = 1;
void FS_LaunchCoroutine::load(FileRead& fr, ScriptingLoader& l) {
	try {
		FunctionStatement::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_LaunchCoroutine) {
			throw Widelands::UnhandledVersionError(
			   "FS_LaunchCoroutine", packet_version, kCurrentPacketVersionFS_LaunchCoroutine);
		}
		FS_LaunchCoroutine::Loader& loader = l.loader<FS_LaunchCoroutine::Loader>(this);
		loader.func = fr.unsigned_32();
	} catch (const WException& e) {
		throw wexception("FS_LaunchCoroutine: %s", e.what());
	}
}
void FS_LaunchCoroutine::load_pointers(ScriptingLoader& l) {
	FunctionStatement::load_pointers(l);
	FS_LaunchCoroutine::Loader& loader = l.loader<FS_LaunchCoroutine::Loader>(this);
	function_ = &l.get<FS_FunctionCall>(loader.func);
}
void FS_LaunchCoroutine::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_LaunchCoroutine);
	fw.unsigned_32(function_->serial());
}
int32_t FS_LaunchCoroutine::write_lua(FileWrite& fw) const {
	function_->check_parameters();
	fw.print_f("run(%s", function_->get_function()->get_name().c_str());
	for (const Assignable* p : function_->parameters()) {
		fw.print_f(", ");
		p->write_lua(fw);
	}
	fw.print_f(")");
	return 0;
}
std::string FS_LaunchCoroutine::readable() const {
	std::string str = "run(" + function_->get_function()->get_name();
	for (const Assignable* p : function_->parameters()) {
		str += ", " + p->readable();
	}
	return str + ")";
}

/************************************************************
             VariableType static helper functions
************************************************************/

// static
std::string descname(VariableType t) {
	switch (t) {
	case VariableType::Nil:
		return _("Nil");
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
	if (t == s || t == VariableType::Nil) {
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
                      Builtin functions
************************************************************/

const BuiltinFunctionInfo& builtin(const std::string& name) {
	for (size_t i = 0; kBuiltinFunctions[i]; ++i) {
		if (kBuiltinFunctions[i]->unique_name == name) {
			return *kBuiltinFunctions[i];
		}
	}
	throw wexception("Unknown builtin function %s", name.c_str());
}

// Do not change the order! Indices are stored in map files!
// The _() command is not contained here – access it instead via `ConstexprString`'s `translatable`
// attribute.
const BuiltinFunctionInfo* kBuiltinFunctions[] = {
   new BuiltinFunctionInfo(
      "print",
      []() { return _("Prints debug information to the stdandard output."); },
      new FunctionBase("print",
                       VariableType::Nil,
                       VariableType::Nil,
                       {std::make_pair("text", VariableType::String)})),
   /** TRANSLATORS: max is the name of a function parameter */
   new BuiltinFunctionInfo(
      "random_1",
      []() { return _("Returns a random value between 1 and max."); },
      new FunctionBase("math.random",
                       VariableType::Nil,
                       VariableType::Integer,
                       {std::make_pair("max", VariableType::Integer)},
                       false)),
   /** TRANSLATORS: min and max are names of function parameters */
   new BuiltinFunctionInfo(
      "random_2",
      []() { return _("Returns a random value between min and max."); },
      new FunctionBase("math.random",
                       VariableType::Nil,
                       VariableType::Integer,
                       {std::make_pair("min", VariableType::Integer),
                        std::make_pair("max", VariableType::Integer)},
                       false)),
   // NOCOM Add all builtins of all the classes in src/scripting/ here…
   nullptr};

/************************************************************
                   Saveloading helper
************************************************************/

constexpr uint16_t kCurrentPacketVersionScripting = 1;

void ScriptingSaver::add(ScriptingObject& a) {
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
			log("NOCOM ScriptingLoader no.%u ", n);
			ScriptingObject* a = ScriptingObject::load(fr);
			log("of type %s\n", typeid(*a).name());
			list_.emplace(
			   std::make_pair(a, std::unique_ptr<ScriptingObject::Loader>(a->create_loader())));
			a->load(fr, *this);
			a->init(s, false);
		}

		// Second load phase: Load pointers between objects.
		for (const auto& pair : list_) {
			pair.first->load_pointers(*this);
		}
	} catch (const WException& e) {
		throw wexception("ScriptingLoader: %s", e.what());
	}
}
