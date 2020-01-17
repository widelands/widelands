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

#ifndef WL_EDITOR_SCRIPTING_FUNCTION_STATEMENTS_H
#define WL_EDITOR_SCRIPTING_FUNCTION_STATEMENTS_H

#include "editor/scripting/function.h"

class Property;
class Variable;

/************************************************************
                  Function Statements
************************************************************/

class FS_FunctionCall : public Assignable, public FunctionStatement {
public:
	FS_FunctionCall(FunctionBase* f, Assignable* v, std::list<Assignable*> p)
	   : function_(f), variable_(v), parameters_(p) {
	}
	~FS_FunctionCall() override {
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;
	inline ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSFunctionCall;
	}
	VariableType type() const override {
		assert(function_);
		return function_->get_returns();
	}
	std::string readable() const override;

	const FunctionBase* get_function() const {
		return function_;
	}
	void set_function(FunctionBase* f) {
		function_ = f;
	}
	const Assignable* get_variable() const {
		return variable_;
	}
	void set_variable(Assignable* v);
	const std::list<Assignable*>& parameters() const {
		return parameters_;
	}
	std::list<Assignable*>& mutable_parameters() {
		return parameters_;
	}

	void check_parameters() const;

	struct Loader : public ScriptingObject::Loader {
		Loader() = default;
		~Loader() override {
		}
		uint32_t var;
		// Indices <= 0 refer to the negative index in kBuiltinFunctions
		int32_t func;
		std::list<uint32_t> params;
	};
	ScriptingObject::Loader* create_loader() const override {
		return new FS_FunctionCall::Loader();
	}
	void load_pointers(ScriptingLoader&) override;
	std::set<uint32_t> references() const override;

private:
	FunctionBase* function_;
	Assignable* variable_;
	std::list<Assignable*> parameters_;

	DISALLOW_COPY_AND_ASSIGN(FS_FunctionCall);
};

// Set the property of a builtin (e.g. field.height = 5)
class FS_SetProperty : public FunctionStatement {
public:
	FS_SetProperty(Assignable* v, Property* p, Assignable* a)
	   : variable_(v), property_(p), value_(a) {
	}
	~FS_SetProperty() override {
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;
	inline ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSSetProperty;
	}
	std::string readable() const override;

	const Property* get_property() const {
		return property_;
	}
	void set_property(Property&);
	const Assignable* get_variable() const {
		return variable_;
	}
	void set_variable(Assignable&);
	const Assignable* get_value() const {
		return value_;
	}
	void set_value(Assignable&);

	struct Loader : public ScriptingObject::Loader {
		Loader() = default;
		~Loader() override {
		}
		uint32_t var, prop, val;
	};
	ScriptingObject::Loader* create_loader() const override {
		return new FS_SetProperty::Loader();
	}
	void load_pointers(ScriptingLoader&) override;
	std::set<uint32_t> references() const override;

private:
	Assignable* variable_;
	Property* property_;
	Assignable* value_;

	DISALLOW_COPY_AND_ASSIGN(FS_SetProperty);
};

// Launch a coroutine with the given parameters
class FS_LaunchCoroutine : public FunctionStatement {
public:
	FS_LaunchCoroutine(FS_FunctionCall* f) : function_(f) {
	}
	~FS_LaunchCoroutine() override {
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;
	inline ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSLaunchCoroutine;
	}
	std::string readable() const override;

	FS_FunctionCall& get_function() const {
		return *function_;
	}
	void set_function(FS_FunctionCall& f) {
		function_ = &f;
	}

	struct Loader : public ScriptingObject::Loader {
		Loader() = default;
		~Loader() override {
		}
		uint32_t func;
	};
	ScriptingObject::Loader* create_loader() const override {
		return new FS_LaunchCoroutine::Loader();
	}
	void load_pointers(ScriptingLoader&) override;
	std::set<uint32_t> references() const override;

private:
	FS_FunctionCall* function_;

	DISALLOW_COPY_AND_ASSIGN(FS_LaunchCoroutine);
};

// Assigns a value or function result to a local or global variable.
class FS_LocalVarDeclOrAssign : public FunctionStatement {
public:
	FS_LocalVarDeclOrAssign(bool l, Variable* var, Assignable* val)
	   : variable_(var), value_(val), declare_local_(l) {
	}
	~FS_LocalVarDeclOrAssign() override {
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;
	inline ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSLocalVarDeclOrAssign;
	}
	std::string readable() const override;

	const Variable& variable() const {
		return *variable_;
	}
	Variable* get_variable() {
		return variable_;
	}
	bool get_declare_local() const {
		return declare_local_;
	}
	void set_declare_local(bool l) {
		declare_local_ = l;
	}
	const Assignable* get_value() const {
		return value_;
	}
	void set_value(Assignable* v) {
		value_ = v;
	}

	struct Loader : public ScriptingObject::Loader {
		Loader() = default;
		~Loader() override {
		}
		uint32_t var, val;
	};
	ScriptingObject::Loader* create_loader() const override {
		return new FS_LocalVarDeclOrAssign::Loader();
	}
	void load_pointers(ScriptingLoader&) override;
	std::set<uint32_t> references() const override;

private:
	Variable* variable_;
	Assignable* value_;
	bool declare_local_;

	DISALLOW_COPY_AND_ASSIGN(FS_LocalVarDeclOrAssign);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_FUNCTION_STATEMENTS_H
