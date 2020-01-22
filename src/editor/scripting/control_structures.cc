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

// While

constexpr uint16_t kCurrentPacketVersionFS_While = 1;

void FS_While::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_While) {
			throw Widelands::UnhandledVersionError(
			   "FS_While", packet_version, kCurrentPacketVersionFS_While);
		}
		is_while_ = fr.unsigned_8();
		loader.push_back(fr.unsigned_32());
		for (size_t n = fr.unsigned_32(); n; --n) {
			loader.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor while loop: %s", e.what());
	}
}
void FS_While::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	condition_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	while (!loader.empty()) {
		body_.push_back(&l.get<FunctionStatement>(loader.front()));
		loader.pop_front();
	}
}
void FS_While::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_While);
	fw.unsigned_8(is_while_ ? 1 : 0);
	fw.unsigned_32(condition_->serial());
	fw.unsigned_32(body_.size());
	for (const auto& f : body_) {
		fw.unsigned_32(f->serial());
	}
}
void FS_While::selftest() const {
	FunctionStatement::selftest();
	if (!condition_)
		throw wexception("condition not set");
	if (!is(condition_->type().id(), VariableTypeID::Boolean))
		throw wexception("condition is not a boolean expression");
	for (const FunctionStatement* a : body_)
		if (!a)
			throw wexception("nullptr body statement");
}
std::set<uint32_t> FS_While::references() const {
	auto set = FunctionStatement::references();
	set.insert(condition_->serial());
	for (const FunctionStatement* f : body_) {
		set.insert(f->serial());
	}
	return set;
}
std::string FS_While::readable() const {
	const uint32_t n = body_.size();
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
	::write_lua(indent, fw, body_);
}

// For Each

constexpr uint16_t kCurrentPacketVersionFS_ForEach = 1;

void FS_ForEach::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_ForEach) {
			throw Widelands::UnhandledVersionError(
			   "FS_ForEach", packet_version, kCurrentPacketVersionFS_ForEach);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
		for (size_t n = fr.unsigned_32(); n; --n) {
			loader.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor for-each loop: %s", e.what());
	}
}
void FS_ForEach::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	table_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	i_ = &l.get<Variable>(loader.front());
	loader.pop_front();
	j_ = &l.get<Variable>(loader.front());
	loader.pop_front();
	while (!loader.empty()) {
		body_.push_back(&l.get<FunctionStatement>(loader.front()));
		loader.pop_front();
	}
}
void FS_ForEach::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_ForEach);
	fw.unsigned_32(table_->serial());
	fw.unsigned_32(i_->serial());
	fw.unsigned_32(j_->serial());
	fw.unsigned_32(body_.size());
	for (const auto& f : body_) {
		fw.unsigned_32(f->serial());
	}
}
void FS_ForEach::selftest() const {
	FunctionStatement::selftest();
	if (!table_)
		throw wexception("table not set");
	if (!i_)
		throw wexception("first variable not set");
	if (!j_)
		throw wexception("second variable not set");
	for (const FunctionStatement* a : body_)
		if (!a)
			throw wexception("nullptr body statement");
}
std::set<uint32_t> FS_ForEach::references() const {
	auto set = FunctionStatement::references();
	set.insert(table_->serial());
	set.insert(j_->serial());
	set.insert(i_->serial());
	for (const FunctionStatement* f : body_) {
		set.insert(f->serial());
	}
	return set;
}
std::string FS_ForEach::readable() const {
	const uint32_t n = body_.size();
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
	::write_lua(indent, fw, body_);
}

// For

constexpr uint16_t kCurrentPacketVersionFS_For = 1;

void FS_For::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionFS_For) {
			throw Widelands::UnhandledVersionError(
			   "FS_For", packet_version, kCurrentPacketVersionFS_For);
		}
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
		loader.push_back(fr.unsigned_32());
		for (size_t n = fr.unsigned_32(); n; --n) {
			loader.push_back(fr.unsigned_32());
		}
	} catch (const WException& e) {
		throw wexception("editor for loop: %s", e.what());
	}
}
void FS_For::load_pointers(const ScriptingLoader& l, Loader& loader) {
	FunctionStatement::load_pointers(l, loader);
	variable_ = &l.get<Variable>(loader.front());
	loader.pop_front();
	i_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	j_ = &l.get<Assignable>(loader.front());
	loader.pop_front();
	while (!loader.empty()) {
		body_.push_back(&l.get<FunctionStatement>(loader.front()));
		loader.pop_front();
	}
}
void FS_For::save(FileWrite& fw) const {
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_For);
	fw.unsigned_32(variable_->serial());
	fw.unsigned_32(i_->serial());
	fw.unsigned_32(j_->serial());
	fw.unsigned_32(body_.size());
	for (const auto& f : body_) {
		fw.unsigned_32(f->serial());
	}
}
void FS_For::selftest() const {
	FunctionStatement::selftest();
	if (!variable_)
		throw wexception("variable not set");
	if (!i_)
		throw wexception("first border not set");
	if (!j_)
		throw wexception("second border not set");
	for (const FunctionStatement* a : body_)
		if (!a)
			throw wexception("nullptr body statement");
}
std::set<uint32_t> FS_For::references() const {
	auto set = FunctionStatement::references();
	set.insert(variable_->serial());
	set.insert(j_->serial());
	set.insert(i_->serial());
	for (const FunctionStatement* f : body_) {
		set.insert(f->serial());
	}
	return set;
}
std::string FS_For::readable() const {
	const uint32_t n = body_.size();
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
	::write_lua(indent, fw, body_);
}

// If

constexpr uint16_t kCurrentPacketVersionFS_If = 1;

void FS_If::load(FileRead& fr, Loader& loader) {
	try {
		FunctionStatement::load(fr, loader);
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
	FunctionStatement::load_pointers(l, loader);
	condition_ = &l.get<Assignable>(loader.front());
	loader.pop_front();

	size_t n = loader.front();
	loader.pop_front();
	for (; n; --n) {
		if_body_.push_back(&l.get<FunctionStatement>(loader.front()));
		loader.pop_front();
	}

	n = loader.front();
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
	FunctionStatement::save(fw);
	fw.unsigned_16(kCurrentPacketVersionFS_If);
	fw.unsigned_32(condition_->serial());

	fw.unsigned_32(if_body_.size());
	for (const auto& f : if_body_) {
		fw.unsigned_32(f->serial());
	}

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
	auto set = FunctionStatement::references();
	set.insert(condition_->serial());
	for (const FunctionStatement* f : if_body_) {
		set.insert(f->serial());
	}
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
	FunctionStatement::selftest();
	if (!condition_)
		throw wexception("condition not set");
	if (!is(condition_->type().id(), VariableTypeID::Boolean))
		throw wexception("condition is not a boolean expression");
	for (const FunctionStatement* a : if_body_) {
		if (!a) {
			throw wexception("nullptr if body statement");
		} else if (is_a(FS_Break, a)) {
			throw wexception("break statement outside for or while loop");
		}
	}
	for (const FunctionStatement* a : else_body_) {
		if (!a) {
			throw wexception("nullptr else body statement");
		} else if (is_a(FS_Break, a)) {
			throw wexception("break statement outside for or while loop");
		}
	}
	for (const auto& pair : elseif_bodies_) {
		if (!pair.first)
			throw wexception("elseif condition not set");
		if (!is(pair.first->type().id(), VariableTypeID::Boolean))
			throw wexception("elseif condition is not a boolean expression");
		for (const FunctionStatement* a : pair.second) {
			if (!a) {
				throw wexception("nullptr elseif body statement");
			} else if (is_a(FS_Break, a)) {
				throw wexception("break statement outside for or while loop");
			}
		}
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

	if (if_body_.empty()) {
		/* This would look like this:
		 *    if x then
		 *    else
		 *       do_something()
		 *    end
		 * Ugly!
		 */
		throw wexception("if statement %u: Empty if body", serial());
	}
	::write_lua(indent, fw, if_body_);

	for (const auto& pair : elseif_bodies_) {
		if (pair.second.empty()) {
			/* This would look like this:
			 *    if x then
			 *       do_something()
			 *    elseif y then
			 *    else
			 *       do_something()
			 *    end
			 * Ugly!
			 */
			throw wexception("if statement %u: Empty elseif body", serial());
		}
		fw.print_f("elseif ");
		pair.first->write_lua(indent, fw);
		fw.print_f(" then\n");
		::write_lua(indent, fw, pair.second);
	}

	if (!else_body_.empty()) {
		// Not a problem since else, unlike if and elseif, doesn't have a
		// condition, so leaving it out won't effect any downstream clauses.
		fw.print_f("else\n");
		::write_lua(indent, fw, else_body_);
	}
}
