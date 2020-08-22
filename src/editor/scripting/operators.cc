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

#include "editor/scripting/operators.h"

// Abstract Operator

constexpr uint16_t kCurrentPacketVersionOperatorBase = 1;

OperatorBase::OperatorBase(const VariableType& in,
                           const VariableType& out,
                           Assignable* a,
                           Assignable* b)
   : a_(a), b_(b), input_type_(in), output_type_(out) {
}
void OperatorBase::load(FileRead& fr, Loader& loader) {
	try {
		Assignable::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionOperatorBase) {
			throw Widelands::UnhandledVersionError(
			   "OperatorBase", packet_version, kCurrentPacketVersionOperatorBase);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("editor base operator: %s", e.what());
	}
}
void OperatorBase::load_pointers(const ScriptingLoader& l, Loader& loader) {
	Assignable::load_pointers(l, loader);
	a_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	b_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void OperatorBase::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionOperatorBase);
	fw.unsigned_32(a_->serial());
	fw.unsigned_32(b_->serial());
}
std::set<uint32_t> OperatorBase::references() const {
	auto set = Assignable::references();
	set.insert(a_->serial());
	set.insert(b_->serial());
	return set;
}
std::string OperatorBase::readable() const {
	return a_->readable() + " " + op() + " " + b_->readable();
}
void OperatorBase::write_lua(int32_t i, FileWrite& fw) const {
	fw.print_f("(");
	a_->write_lua(i, fw);
	fw.print_f(" %s ", op().c_str());
	b_->write_lua(i, fw);
	fw.print_f(")");
}
void OperatorBase::selftest() const {
	Assignable::selftest();
	if (!a_) {
		throw wexception("first parameter not set");
	}
	if (!b_) {
		throw wexception("first parameter not set");
	}
	if (input_type_.id() == VariableTypeID::Nil) {
		throw wexception("input type is nil");
	}
	if (output_type_.id() == VariableTypeID::Nil) {
		throw wexception("output type is nil");
	}
}

// The '..' operator

constexpr uint16_t kCurrentPacketVersionOperatorStringConcat = 1;

static inline bool is_string_convertible(const VariableTypeID& t) {
	return t == VariableTypeID::String || t == VariableTypeID::Integer ||
	       t == VariableTypeID::Double;
}

OperatorStringConcat::OperatorStringConcat(Assignable* a, Assignable* b) : a_(a), b_(b) {
}
void OperatorStringConcat::set_a(Assignable& a) {
	a_ = &a;
}
void OperatorStringConcat::set_b(Assignable& b) {
	b_ = &b;
}

void OperatorStringConcat::load(FileRead& fr, Loader& loader) {
	try {
		Assignable::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionOperatorStringConcat) {
			throw Widelands::UnhandledVersionError(
			   "OperatorStringConcat", packet_version, kCurrentPacketVersionOperatorStringConcat);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("editor string concat operator: %s", e.what());
	}
}
void OperatorStringConcat::load_pointers(const ScriptingLoader& l, Loader& loader) {
	Assignable::load_pointers(l, loader);
	a_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	b_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void OperatorStringConcat::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionOperatorStringConcat);
	fw.unsigned_32(a_->serial());
	fw.unsigned_32(b_->serial());
}
std::set<uint32_t> OperatorStringConcat::references() const {
	auto set = Assignable::references();
	set.insert(a_->serial());
	set.insert(b_->serial());
	return set;
}
std::string OperatorStringConcat::readable() const {
	return "(" + a_->readable() + " .. " + b_->readable() + ")";
}
void OperatorStringConcat::write_lua(int32_t i, FileWrite& fw) const {
	fw.print_f("(");
	a_->write_lua(i, fw);
	fw.print_f(" .. ");
	b_->write_lua(i, fw);
	fw.print_f(")");
}
void OperatorStringConcat::selftest() const {
	Assignable::selftest();
	if (!a_) {
		throw wexception("first parameter not set");
	}
	if (!b_) {
		throw wexception("second parameter not set");
	}
	if (!is_string_convertible(a_->type().id())) {
		throw wexception("first parameter is not a string");
	}
	if (!is_string_convertible(b_->type().id())) {
		throw wexception("second parameter is not a string");
	}
}

// The 'not' operator

constexpr uint16_t kCurrentPacketVersionOperatorNot = 1;

void OperatorNot::load(FileRead& fr, Loader& loader) {
	try {
		Assignable::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionOperatorNot) {
			throw Widelands::UnhandledVersionError(
			   "OperatorNot", packet_version, kCurrentPacketVersionOperatorNot);
		}
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("editor not operator: %s", e.what());
	}
}
void OperatorNot::load_pointers(const ScriptingLoader& l, Loader& loader) {
	Assignable::load_pointers(l, loader);
	a_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void OperatorNot::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionOperatorNot);
	fw.unsigned_32(a_->serial());
}
std::set<uint32_t> OperatorNot::references() const {
	auto set = Assignable::references();
	set.insert(a_->serial());
	return set;
}
std::string OperatorNot::readable() const {
	return "not " + a_->readable();
}
void OperatorNot::write_lua(int32_t i, FileWrite& fw) const {
	fw.print_f("(not ");
	a_->write_lua(i, fw);
	fw.print_f(")");
}
void OperatorNot::selftest() const {
	Assignable::selftest();
	if (!a_) {
		throw wexception("parameter not set");
	}
}

// The '== nil' operator

constexpr uint16_t kCurrentPacketVersionOperatorIsNil = 1;

void OperatorIsNil::load(FileRead& fr, Loader& loader) {
	try {
		Assignable::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionOperatorIsNil) {
			throw Widelands::UnhandledVersionError(
			   "OperatorIsNil", packet_version, kCurrentPacketVersionOperatorIsNil);
		}
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("editor IsNil operator: %s", e.what());
	}
}
void OperatorIsNil::load_pointers(const ScriptingLoader& l, Loader& loader) {
	Assignable::load_pointers(l, loader);
	a_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void OperatorIsNil::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionOperatorIsNil);
	fw.unsigned_32(a_->serial());
}
std::set<uint32_t> OperatorIsNil::references() const {
	auto set = Assignable::references();
	set.insert(a_->serial());
	return set;
}
std::string OperatorIsNil::readable() const {
	return a_->readable() + " == nil";
}
void OperatorIsNil::write_lua(int32_t i, FileWrite& fw) const {
	fw.print_f("(");
	a_->write_lua(i, fw);
	fw.print_f(" == nil)");
}
void OperatorIsNil::selftest() const {
	Assignable::selftest();
	if (!a_) {
		throw wexception("parameter not set");
	}
}
