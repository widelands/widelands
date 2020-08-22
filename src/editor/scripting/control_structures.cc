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

#include "editor/scripting/control_structures.h"

#include <boost/format.hpp>

#include "editor/scripting/function.h"
#include "editor/scripting/function_statements.h"

// Abstract ControlStructure

constexpr uint16_t kCurrentPacketVersionControlStructure = 1;

void ControlStructure::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionControlStructure) {
			throw Widelands::UnhandledVersionError(
			   "ControlStructure", packet_version, kCurrentPacketVersionControlStructure);
		}
		size_t n = fr.unsigned_32();
		loader.push_back(n);
		for (; n; --n) {
			loader.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor while loop: %s", e.what());
	}
}
void ControlStructure::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	size_t n = loader.front();
	loader.pop_front();
	for (; n; --n) {
		body_.push_back(&l.get<FunctionStatement>(loader.front()));
		loader.pop_front();
	}
}
void ControlStructure::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionControlStructure);
	fw.unsigned_32(body_.size());
	for (const auto& f : body_) {
		fw.unsigned_32(f->serial());
	}
}
std::set<uint32_t> ControlStructure::references() const {
	auto set = FunctionStatement::references();
	for (const FunctionStatement* f : body_) {
		set.insert(f->serial());
	}
	return set;
}

void ControlStructure::selftest() const {
	FunctionStatement::selftest();
	if (body_.empty()) {
		throw wexception("empty body");
	}
}

// static
void ControlStructure::selftest_body(const LuaFunction& f,
                                     const std::list<FunctionStatement*>& body,
                                     bool allow_break,
                                     bool enforce_return) {
	bool last_statement_is_return = false;
	bool last_statement_is_break = false;
	for (const FunctionStatement* a : body) {
		assert(!last_statement_is_return || !last_statement_is_break);
		if (!a) {
			throw wexception("nullptr body statement");
		} else if (is_a(FS_Break, a)) {
			last_statement_is_break = true;
			last_statement_is_return = false;
			if (!allow_break) {
				throw wexception("break statement outside for or while loop");
			}
		} else if (upcast(const FS_Return, r, a)) {
			last_statement_is_return = true;
			last_statement_is_break = false;
			if (f.get_returns().id() == VariableTypeID::Nil) {
				if (r->get_return()) {
					throw wexception("non-empty return statement in function returning nil");
				}
			} else {
				if (!r->get_return()) {
					throw wexception("empty return statement in function returning non-nil");
				}
				if (!r->get_return()->type().is_subclass(f.get_returns())) {
					throw wexception("in return statement: %s cannot be casted to %s",
					                 descname(r->get_return()->type()).c_str(),
					                 descname(f.get_returns()).c_str());
				}
			}
		} else {
			if (last_statement_is_break) {
				throw wexception("unreachable statement after break statement");
			}
			if (last_statement_is_return) {
				throw wexception("unreachable statement after return statement");
			}
			last_statement_is_break = false;
			last_statement_is_return = false;
		}
	}
	if (last_statement_is_break) {
		throw wexception("unnecessary break statement at end of loop");
	}
	if (enforce_return && !last_statement_is_return && f.get_returns().id() != VariableTypeID::Nil) {
		throw wexception("missing return statement at end of function");
	}
	if (enforce_return && last_statement_is_return && f.get_returns().id() == VariableTypeID::Nil) {
		throw wexception("unnecessary return statement at end of function returning nil");
	}
}

// While

constexpr uint16_t kCurrentPacketVersionFS_While = 1;

void FS_While::load(FileRead& fr, Loader& loader) {
	try {
		ControlStructure::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_While) {
			throw Widelands::UnhandledVersionError(
			   "FS_While", packet_version, kCurrentPacketVersionFS_While);
		}
		is_while_ = fr.unsigned_8();
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("editor while loop: %s", e.what());
	}
}
void FS_While::load_pointers(const ScriptingLoader& l, Loader& loader) {
	ControlStructure::load_pointers(l, loader);
	condition_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void FS_While::save(FileWrite& fw) const {
	ControlStructure::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_While);
	fw.unsigned_8(is_while_ ? 1 : 0);
	fw.unsigned_32(condition_->serial());
}
void FS_While::selftest() const {
	ControlStructure::selftest();
	if (!condition_) {
		throw wexception("condition not set");
	}
	if (!is(condition_->type().id(), VariableTypeID::Boolean)) {
		throw wexception("condition is not a boolean expression");
	}
}
std::set<uint32_t> FS_While::references() const {
	auto set = ControlStructure::references();
	set.insert(condition_->serial());
	return set;
}
std::string FS_While::readable() const {
	const uint32_t n = body().size();
	const std::string str = (boost::format(ngettext("%u statement", "%u statements", n)) % n).str();
	return is_while_ ? ("while " + condition_->readable() + " do [" + str + "]") :
	                   ("repeat [" + str + "] until " + condition_->readable());
}
void FS_While::write_lua(int32_t indent, FileWrite& fw) const {
	if (is_while_) {
		fw.print_f("while ");
		condition_->write_lua(indent, fw);
		fw.print_f(" do");
	} else {
		fw.print_f("repeat");
	}
	fw.print_f("\n");
	::write_lua(indent, fw, body(), true);
}

// For Each

constexpr uint16_t kCurrentPacketVersionFS_ForEach = 1;

void FS_ForEach::load(FileRead& fr, Loader& loader) {
	try {
		ControlStructure::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_ForEach) {
			throw Widelands::UnhandledVersionError(
			   "FS_ForEach", packet_version, kCurrentPacketVersionFS_ForEach);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("editor for-each loop: %s", e.what());
	}
}
void FS_ForEach::load_pointers(const ScriptingLoader& l, Loader& loader) {
	ControlStructure::load_pointers(l, loader);
	table_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	i_ = &l.get<Variable>(loader.front());
	loader.pop_front();
	j_ = &l.get<Variable>(loader.front());
	loader.pop_front();
}
void FS_ForEach::save(FileWrite& fw) const {
	ControlStructure::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_ForEach);
	fw.unsigned_32(table_->serial());
	fw.unsigned_32(i_->serial());
	fw.unsigned_32(j_->serial());
}
void FS_ForEach::selftest() const {
	ControlStructure::selftest();
	if (!table_) {
		throw wexception("table not set");
	}
	if (!i_) {
		throw wexception("first variable not set");
	}
	if (!j_) {
		throw wexception("second variable not set");
	}
}
std::set<uint32_t> FS_ForEach::references() const {
	auto set = ControlStructure::references();
	set.insert(table_->serial());
	set.insert(j_->serial());
	set.insert(i_->serial());
	return set;
}
std::string FS_ForEach::readable() const {
	const uint32_t n = body().size();
	const std::string str = (boost::format(ngettext("%u statement", "%u statements", n)) % n).str();
	return "for " + i_->get_name() + "," + j_->get_name() + " in " +
	       (table_->type().key_type().is_subclass(VariableType(VariableTypeID::Integer)) ? "ipairs" :
	                                                                                       "pairs") +
	       "(" + table_->readable() + ") do [" + str + "]";
}
void FS_ForEach::write_lua(int32_t indent, FileWrite& fw) const {
	fw.print_f("for ");
	i_->write_lua(indent, fw);
	fw.print_f(",");
	j_->write_lua(indent, fw);
	fw.print_f(
	   " in %s(", table_->type().key_type().is_subclass(VariableType(VariableTypeID::Integer)) ?
	                 "ipairs" :
	                 "pairs");
	table_->write_lua(indent, fw);
	fw.print_f(") do\n");
	::write_lua(indent, fw, body(), true);
}

// For

constexpr uint16_t kCurrentPacketVersionFS_For = 1;

void FS_For::load(FileRead& fr, Loader& loader) {
	try {
		ControlStructure::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_For) {
			throw Widelands::UnhandledVersionError(
			   "FS_For", packet_version, kCurrentPacketVersionFS_For);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
	} catch (const WException& e) {
		throw wexception("editor for loop: %s", e.what());
	}
}
void FS_For::load_pointers(const ScriptingLoader& l, Loader& loader) {
	ControlStructure::load_pointers(l, loader);
	variable_ = &l.get<Variable>(loader.front());
	loader.pop_front();
	i_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	j_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
}
void FS_For::save(FileWrite& fw) const {
	ControlStructure::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_For);
	fw.unsigned_32(variable_->serial());
	fw.unsigned_32(i_->serial());
	fw.unsigned_32(j_->serial());
}
void FS_For::selftest() const {
	ControlStructure::selftest();
	if (!variable_) {
		throw wexception("variable not set");
	}
	if (!i_) {
		throw wexception("first border not set");
	}
	if (!j_) {
		throw wexception("second border not set");
	}
}
std::set<uint32_t> FS_For::references() const {
	auto set = ControlStructure::references();
	set.insert(variable_->serial());
	set.insert(j_->serial());
	set.insert(i_->serial());
	return set;
}
std::string FS_For::readable() const {
	const uint32_t n = body().size();
	const std::string str = (boost::format(ngettext("%u statement", "%u statements", n)) % n).str();
	return "for " + variable_->get_name() + " = " + i_->readable() + "," + j_->readable() + " do [" +
	       str + "]";
}
void FS_For::write_lua(int32_t indent, FileWrite& fw) const {
	fw.print_f("for ");
	variable_->write_lua(indent, fw);
	fw.print_f(" = ");
	i_->write_lua(indent, fw);
	fw.print_f(",");
	j_->write_lua(indent, fw);
	fw.print_f(" do\n");
	::write_lua(indent, fw, body(), true);
}

// If

constexpr uint16_t kCurrentPacketVersionFS_If = 1;

void FS_If::load(FileRead& fr, Loader& loader) {
	try {
		ControlStructure::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_If) {
			throw Widelands::UnhandledVersionError(
			   "FS_If", packet_version, kCurrentPacketVersionFS_If);
		}
		loader.push_back(fr.unsigned_32());

		size_t n = fr.unsigned_32();
		loader.push_back(n);
		for (; n; --n) {
			loader.push_back(fr.unsigned_32());
		}

		n = fr.unsigned_32();
		loader.push_back(n);
		for (; n; --n) {
			loader.push_back(fr.unsigned_32());
			size_t nn = fr.unsigned_32();
			loader.push_back(nn);
			for (; nn; --nn) {
				loader.push_back(fr.unsigned_32());
			}
		}
	} catch (const WException& e) {
		throw wexception("editor if clause: %s", e.what());
	}
}
void FS_If::load_pointers(const ScriptingLoader& l, Loader& loader) {
	ControlStructure::load_pointers(l, loader);
	condition_ = &l.get<Assignable>(loader.front());
	loader.pop_front();

	size_t n = loader.front();
	loader.pop_front();
	for (; n; --n) {
		else_body_.push_back(&l.get<FunctionStatement>(loader.front()));
		loader.pop_front();
	}

	n = loader.front();
	loader.pop_front();
	for (; n; --n) {
		Assignable* a = &l.get<Assignable>(loader.front());
		loader.pop_front();
		size_t nn = loader.front();
		loader.pop_front();
		std::list<FunctionStatement*> list;
		for (; nn; --nn) {
			list.push_back(&l.get<FunctionStatement>(loader.front()));
			loader.pop_front();
		}
		elseif_bodies_.push_back(std::make_pair(a, list));
	}
}
void FS_If::save(FileWrite& fw) const {
	ControlStructure::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_If);
	fw.unsigned_32(condition_->serial());

	fw.unsigned_32(else_body_.size());
	for (const auto& f : else_body_) {
		fw.unsigned_32(f->serial());
	}

	fw.unsigned_32(elseif_bodies_.size());
	for (const auto& pair : elseif_bodies_) {
		fw.unsigned_32(pair.first->serial());
		fw.unsigned_32(pair.second.size());
		for (const auto& f : pair.second) {
			fw.unsigned_32(f->serial());
		}
	}
}
std::set<uint32_t> FS_If::references() const {
	auto set = ControlStructure::references();
	set.insert(condition_->serial());
	for (const FunctionStatement* f : else_body_) {
		set.insert(f->serial());
	}
	for (const auto& pair : elseif_bodies_) {
		set.insert(pair.first->serial());
		for (const FunctionStatement* f : pair.second) {
			set.insert(f->serial());
		}
	}
	return set;
}
void FS_If::selftest() const {
	ControlStructure::selftest();
	if (!condition_) {
		throw wexception("condition not set");
	}
	if (!is(condition_->type().id(), VariableTypeID::Boolean)) {
		throw wexception("condition is not a boolean expression");
	}
	for (const auto& pair : elseif_bodies_) {
		if (!pair.first) {
			throw wexception("elseif condition not set");
		}
		if (!is(pair.first->type().id(), VariableTypeID::Boolean)) {
			throw wexception("elseif condition is not a boolean expression");
		}
		if (pair.second.empty()) {
			throw wexception("empty elseif body");
		}
	}
}
void FS_If::selftest_body(const LuaFunction& f) const {
	ControlStructure::selftest_body(f);
	ControlStructure::selftest_body(f, else_body_, false, false);
	for (const auto& pair : elseif_bodies_) {
		ControlStructure::selftest_body(f, pair.second, false, false);
	}
}
std::string FS_If::readable() const {
	std::string str = "if " + condition_->readable() + " then …";
	for (const auto& pair : elseif_bodies_) {
		str += " elseif " + pair.first->readable() + " then …";
	}
	if (!else_body_.empty()) {
		str += " else …";
	}
	return str + " end";
}
void FS_If::write_lua(int32_t indent, FileWrite& fw) const {
	fw.print_f("if ");
	condition_->write_lua(indent, fw);
	fw.print_f(" then\n");
	::write_lua(indent, fw, body(), false);

	for (const auto& pair : elseif_bodies_) {
		fw.print_f("elseif ");
		pair.first->write_lua(indent, fw);
		fw.print_f(" then\n");
		::write_lua(indent, fw, pair.second, false);
	}

	if (!else_body_.empty()) {
		fw.print_f("else\n");
		::write_lua(indent, fw, else_body_, false);
	}

	fw.print_f("end");
}
