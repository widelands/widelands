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

#include "editor/scripting/function.h"

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

void LuaFunction::load(FileRead& fr, Loader& loader) {
	try {
		ScriptingObject::load(fr, loader);
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
		for (size_t n = fr.unsigned_32(); n; --n) {
			loader.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor scripting lua function: %s", e.what());
	}
}

void LuaFunction::load_pointers(const ScriptingLoader& l, Loader& loader) {
	ScriptingObject::load_pointers(l, loader);
	while (!loader.empty()) {
		body_.push_back(&l.get<FunctionStatement>(loader.front()));
		loader.pop_front();
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

std::set<uint32_t> LuaFunction::references() const {
	auto set = ScriptingObject::references();
	for (const FunctionStatement* f : body_) {
		set.insert(f->serial());
	}
	return set;
}
