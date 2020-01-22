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

#ifndef WL_EDITOR_SCRIPTING_VARIABLE_H
#define WL_EDITOR_SCRIPTING_VARIABLE_H

#include "editor/scripting/scripting.h"

/************************************************************
                          Variable
************************************************************/

class Variable : public Assignable {
public:
	Variable(const VariableType& t, const std::string& n) : type_(t), name_(n) {
	}
	~Variable() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::Variable;
	}

	void load(FileRead&, Loader&) override;
	void save(FileWrite&) const override;

	const VariableType& type() const override {
		return type_;
	}
	const std::string& get_name() const {
		return name_;
	}
	void rename(const std::string& n) {
		name_ = n;
	}
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override {
		return name_;
	}

	void selftest() const override;

private:
	VariableType type_;
	std::string name_;

	DISALLOW_COPY_AND_ASSIGN(Variable);
};

// Get the property of a builtin (e.g. field.x)
class GetProperty : public Assignable {
public:
	GetProperty(Assignable* v, Property* p) : variable_(v), property_(p) {
	}
	~GetProperty() override {
	}

	void load(FileRead&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;
	inline ScriptingObject::ID id() const override {
		return ScriptingObject::ID::GetProperty;
	}
	const VariableType& type() const override;
	std::string readable() const override;

	const Property* get_property() const {
		return property_;
	}
	void set_property(Property&);
	const Assignable* get_variable() const {
		return variable_;
	}
	void set_variable(Assignable&);

	void load_pointers(const ScriptingLoader&, Loader&) override;
	std::set<uint32_t> references() const override;

	void selftest() const override;

private:
	Assignable* variable_;
	Property* property_;

	DISALLOW_COPY_AND_ASSIGN(GetProperty);
};

// Get a field of a table, e.g. fields[1]
class GetTable : public Assignable {
public:
	GetTable(Assignable* t, Assignable* p) : table_(t), property_(p) {
	}
	~GetTable() override {
	}

	void load(FileRead&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;
	inline ScriptingObject::ID id() const override {
		return ScriptingObject::ID::GetTable;
	}
	const VariableType& type() const override;
	std::string readable() const override;

	const Assignable* get_property() const {
		return property_;
	}
	void set_property(Assignable&);
	const Assignable* get_table() const {
		return table_;
	}
	void set_table(Assignable&);

	void load_pointers(const ScriptingLoader&, Loader&) override;
	std::set<uint32_t> references() const override;

	void selftest() const override;

private:
	Assignable* table_;
	Assignable* property_;

	DISALLOW_COPY_AND_ASSIGN(GetTable);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_VARIABLE_H
