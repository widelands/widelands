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

#ifndef WL_EDITOR_SCRIPTING_CONSTEXPR_H
#define WL_EDITOR_SCRIPTING_CONSTEXPR_H

#include "editor/scripting/scripting.h"

/************************************************************
                        Constexprs
************************************************************/

// A string literal, e.g. "xyz". You do NOT need to surround the value with escaped quotes –
// they will be added automatically when writing the lua file.
class ConstexprString : public Assignable {
public:
	ConstexprString(const std::string& v, bool t = false) : value_(v), translate_(t) {
	}
	~ConstexprString() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprString;
	}
	VariableType type() const override {
		return VariableType::String;
	}

	const std::string& get_value() const {
		return value_;
	}
	void set_value(const std::string& v) {
		value_ = v;
	}
	bool get_translate() const {
		return translate_;
	}
	void set_translate(bool t) {
		translate_ = t;
	}

	void load(FileRead&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override {
		return "\"" + value_ + "\"";
	}

private:
	std::string value_;
	bool translate_;

	DISALLOW_COPY_AND_ASSIGN(ConstexprString);
};

// An integer constant, e.g. 123.
class ConstexprInteger : public Assignable {
public:
	ConstexprInteger(int32_t i) : value_(i) {
	}
	~ConstexprInteger() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprInteger;
	}
	VariableType type() const override {
		return VariableType::Integer;
	}

	int32_t get_value() const {
		return value_;
	}
	void set_value(int32_t v) {
		value_ = v;
	}

	void load(FileRead&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override {
		return std::to_string(value_);
	}

private:
	int32_t value_;

	DISALLOW_COPY_AND_ASSIGN(ConstexprInteger);
};

// A boolean constant: true or false.
class ConstexprBoolean : public Assignable {
public:
	ConstexprBoolean(bool b) : value_(b) {
	}
	~ConstexprBoolean() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprBoolean;
	}
	VariableType type() const override {
		return VariableType::Boolean;
	}

	bool get_value() const {
		return value_;
	}
	void set_value(bool v) {
		value_ = v;
	}

	void load(FileRead&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override {
		return value_ ? _("true") : _("false");
	}

private:
	bool value_;

	DISALLOW_COPY_AND_ASSIGN(ConstexprBoolean);
};

// The nil constant.
class ConstexprNil : public Assignable {
public:
	// As many constructors and attributes as there are flavours of nil…
	ConstexprNil() = default;
	~ConstexprNil() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprNil;
	}
	VariableType type() const override {
		return VariableType::Nil;
	}
	void write_lua(int32_t, FileWrite&) const override;
	std::string readable() const override {
		return _("nil");
	}

	DISALLOW_COPY_AND_ASSIGN(ConstexprNil);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_CONSTEXPR_H
