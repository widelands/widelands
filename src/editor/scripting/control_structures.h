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

#ifndef WL_EDITOR_SCRIPTING_CONTROL_STRUCTURES_H
#define WL_EDITOR_SCRIPTING_CONTROL_STRUCTURES_H

#include "editor/scripting/variable.h"

// This class can serve as a 'while' or a 'repeat … until' loop, depending on is_while_
class FS_While : public FunctionStatement {
public:
	FS_While(bool w, Assignable* c) : is_while_(w), condition_(c) {
		if (condition_)
			assert(is(condition_->type().id(), VariableTypeID::Boolean));
	}
	~FS_While() override {
	}

	Assignable* get_condition() const {
		return condition_;
	}
	void set_condition(Assignable& c) {
		assert(is(c.type().id(), VariableTypeID::Boolean));
		condition_ = &c;
	}
	const std::list<FunctionStatement*>& body() const {
		return body_;
	}
	std::list<FunctionStatement*>& mutable_body() {
		return body_;
	}
	bool is_while() const {
		return is_while_;
	}
	void set_is_while(bool w) {
		is_while_ = w;
	}

	std::set<uint32_t> references() const override;

	void load(FileRead&, Loader&) override;
	void load_pointers(const ScriptingLoader&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override;
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSWhile;
	}

private:
	bool is_while_;
	Assignable* condition_;
	std::list<FunctionStatement*> body_;

	DISALLOW_COPY_AND_ASSIGN(FS_While);
};

// 'for i,x in [i]pairs(table) do'
// Note that the two iterators are implemented as variables
class FS_ForEach : public FunctionStatement {
public:
	FS_ForEach(Variable* i, Variable* j, Assignable* t) : table_(t), i_(i), j_(j) {
		if (table_)
			assert(is(table_->type().id(), VariableTypeID::Table));
	}
	~FS_ForEach() override {
	}

	Variable* get_i() const {
		return i_;
	}
	Variable* get_j() const {
		return j_;
	}
	Assignable* get_table() const {
		return table_;
	}
	void set_i(Variable& v) {
		assert(table_->type().key_type().is_subclass(v.type()));
		i_ = &v;
	}
	void set_j(Variable& v) {
		assert(table_->type().value_type().is_subclass(v.type()));
		j_ = &v;
	}
	void set_table(Assignable& t) {
		assert(is(t.type().id(), VariableTypeID::Table));
		table_ = &t;
	}
	const std::list<FunctionStatement*>& body() const {
		return body_;
	}
	std::list<FunctionStatement*>& mutable_body() {
		return body_;
	}

	std::set<uint32_t> references() const override;

	void load(FileRead&, Loader&) override;
	void load_pointers(const ScriptingLoader&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override;
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSForEach;
	}

private:
	Assignable* table_;
	Variable* i_;
	Variable* j_;
	std::list<FunctionStatement*> body_;

	DISALLOW_COPY_AND_ASSIGN(FS_ForEach);
};

// An if clause, optionally with an else clause and any number of elseif blocks
class FS_If : public FunctionStatement {
public:
	FS_If(Assignable* c) : condition_(c) {
		if (condition_)
			assert(is(condition_->type().id(), VariableTypeID::Boolean));
	}
	~FS_If() override {
	}

	Assignable* get_condition() const {
		return condition_;
	}
	void set_condition(Assignable& c) {
		assert(is(c.type().id(), VariableTypeID::Boolean));
		condition_ = &c;
	}
	const std::list<FunctionStatement*>& if_body() const {
		return if_body_;
	}
	std::list<FunctionStatement*>& mutable_if_body() {
		return if_body_;
	}
	const std::list<FunctionStatement*>& else_body() const {
		return else_body_;
	}
	std::list<FunctionStatement*>& mutable_else_body() {
		return else_body_;
	}
	const std::list<std::pair<Assignable*, std::list<FunctionStatement*>>>& elseif_bodies() const {
		return elseif_bodies_;
	}
	std::list<std::pair<Assignable*, std::list<FunctionStatement*>>>& mutable_elseif_bodies() {
		return elseif_bodies_;
	}

	std::set<uint32_t> references() const override;

	void load(FileRead&, Loader&) override;
	void load_pointers(const ScriptingLoader&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override;
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSIf;
	}

private:
	Assignable* condition_;
	std::list<FunctionStatement*> if_body_;
	std::list<std::pair<Assignable*, std::list<FunctionStatement*>>> elseif_bodies_;
	std::list<FunctionStatement*> else_body_;

	DISALLOW_COPY_AND_ASSIGN(FS_If);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_CONTROL_STRUCTURES_H
