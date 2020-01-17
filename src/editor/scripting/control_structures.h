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

#include "editor/scripting/scripting.h"

// This class can serve as a 'while' or a 'repeat … until' loop, depending on is_while_
class FS_While : public FunctionStatement {
public:
	FS_While(bool w, Assignable* c) : is_while_(w), condition_(c) {
		if (condition_)
			assert(is(condition_->type(), VariableType::Boolean));
	}
	~FS_While() override {
	}

	Assignable* get_condition() const {
		return condition_;
	}
	void set_condition(Assignable& c) {
		assert(is(c.type(), VariableType::Boolean));
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

// An if clause, optionally with an else clause and any number of elseif blocks
class FS_If : public FunctionStatement {
public:
	FS_If(Assignable* c) : condition_(c) {
		if (condition_)
			assert(is(condition_->type(), VariableType::Boolean));
	}
	~FS_If() override {
	}

	Assignable* get_condition() const {
		return condition_;
	}
	void set_condition(Assignable& c) {
		assert(is(c.type(), VariableType::Boolean));
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
