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

OperatorBase::OperatorBase(VariableType in, VariableType out, Assignable* a, Assignable* b)
   : a_(a), b_(b), input_type_(in), output_type_(out) {
	if (a_)
		assert(is(a_->type(), input_type_));
	if (b_)
		assert(is(b_->type(), input_type_));
}
void OperatorBase::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionOperatorBase) {
			throw Widelands::UnhandledVersionError(
			   "OperatorBase", packet_version, kCurrentPacketVersionOperatorBase);
		}
		OperatorBase::Loader& loader = l.loader<OperatorBase::Loader>(this);
		loader.a = fr.unsigned_32();
		loader.b = fr.unsigned_32();
	} catch (const WException& e) {
		throw wexception("editor base operator: %s", e.what());
	}
}
void OperatorBase::load_pointers(ScriptingLoader& l) {
	Assignable::load_pointers(l);
	OperatorBase::Loader& loader = l.loader<OperatorBase::Loader>(this);
	a_ = &l.get<Assignable>(loader.a);
	b_ = &l.get<Assignable>(loader.b);
}
void OperatorBase::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionOperatorBase);
	assert(a_);
	assert(b_);
	fw.unsigned_32(a_->serial());
	fw.unsigned_32(b_->serial());
}
std::set<uint32_t> OperatorBase::references() const {
	auto set = Assignable::references();
	assert(a_);
	assert(b_);
	set.insert(a_->serial());
	set.insert(b_->serial());
	return set;
}
std::string OperatorBase::readable() const {
	assert(a_);
	assert(b_);
	return a_->readable() + " " + op() + " " + b_->readable();
}
int32_t OperatorBase::write_lua(FileWrite& fw) const {
	assert(a_);
	assert(b_);
	fw.print_f("(");
	a_->write_lua(fw);
	fw.print_f(" %s ", op().c_str());
	b_->write_lua(fw);
	fw.print_f(")");
	return 0;
}

// The 'not' operator

constexpr uint16_t kCurrentPacketVersionOperatorNot = 1;

void OperatorNot::load(FileRead& fr, ScriptingLoader& l) {
	try {
		Assignable::load(fr, l);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionOperatorNot) {
			throw Widelands::UnhandledVersionError(
			   "OperatorNot", packet_version, kCurrentPacketVersionOperatorNot);
		}
		OperatorNot::Loader& loader = l.loader<OperatorNot::Loader>(this);
		loader.a = fr.unsigned_32();
	} catch (const WException& e) {
		throw wexception("editor not operator: %s", e.what());
	}
}
void OperatorNot::load_pointers(ScriptingLoader& l) {
	Assignable::load_pointers(l);
	OperatorNot::Loader& loader = l.loader<OperatorNot::Loader>(this);
	a_ = &l.get<Assignable>(loader.a);
}
void OperatorNot::save(FileWrite& fw) const {
	Assignable::save(fw);
	fw.unsigned_16(kCurrentPacketVersionOperatorNot);
	assert(a_);
	fw.unsigned_32(a_->serial());
}
std::set<uint32_t> OperatorNot::references() const {
	auto set = Assignable::references();
	assert(a_);
	set.insert(a_->serial());
	return set;
}
std::string OperatorNot::readable() const {
	assert(a_);
	return "not " + a_->readable();
}
int32_t OperatorNot::write_lua(FileWrite& fw) const {
	assert(a_);
	fw.print_f("(not ");
	a_->write_lua(fw);
	fw.print_f(")");
	return 0;
}
