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

#include "editor/scripting/constexpr.h"

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
