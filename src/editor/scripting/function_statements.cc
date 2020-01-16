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

#include "editor/scripting/variable.h"

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
