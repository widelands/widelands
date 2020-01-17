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

#ifndef WL_EDITOR_SCRIPTING_FUNCTION_H
#define WL_EDITOR_SCRIPTING_FUNCTION_H

#include "editor/scripting/scripting.h"

class FunctionStatement;

/************************************************************
                      Abstract Function
************************************************************/

/* Not a child of ScriptingObject. LuaFunctions (defined by the user) inherit from this
 * class as well as from ScriptingObject. This class is here to provide all the builtin function
 * headers. Since this class is not derived from Assignable, its instances need to be embedded
 * in a FunctionCall object to be used for anything.
 */
class FunctionBase {
public:
	// Full constructor, for builtin functions only
	FunctionBase(const std::string&,
	             VariableType c,
	             VariableType r,
	             std::list<std::pair<std::string, VariableType>>,
	             bool = true);

	virtual ~FunctionBase() {
	}

	const std::string& get_name() const {
		return name_;
	}
	void rename(const std::string& n) {
		check_name_valid(n);
		name_ = n;
	}
	const std::list<std::pair<std::string, VariableType>>& parameters() const {
		return parameters_;
	}
	std::list<std::pair<std::string, VariableType>>& mutable_parameters() {
		return parameters_;
	}

	std::string header(bool lua_format) const;

	VariableType get_class() const {
		return class_;
	}
	VariableType get_returns() const {
		return returns_;
	}

protected:
	// for LuaFunction
	FunctionBase(const std::string&, bool spellcheck = true);

	void set_returns(VariableType r) {
		returns_ = r;
	}

	std::list<std::pair<std::string, VariableType>> parameters_;

private:
	std::string name_;
	VariableType class_;
	VariableType returns_;

	DISALLOW_COPY_AND_ASSIGN(FunctionBase);
};

/************************************************************
                   User-defined functions
************************************************************/

class LuaFunction : public ScriptingObject, public FunctionBase {
public:
	LuaFunction(const std::string& n, bool spellcheck = true) : FunctionBase(n, spellcheck) {
	}
	~LuaFunction() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::LuaFunction;
	}

	void load(FileRead&, Loader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

	std::string readable() const override {
		return header(true);
	}

	std::set<uint32_t> references() const override;

	const std::list<FunctionStatement*>& body() const {
		return body_;
	}
	std::list<FunctionStatement*>& mutable_body() {
		return body_;
	}

	void load_pointers(const ScriptingLoader&, Loader&) override;

private:
	std::list<FunctionStatement*> body_;

	DISALLOW_COPY_AND_ASSIGN(LuaFunction);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_FUNCTION_H
