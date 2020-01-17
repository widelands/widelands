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

#ifndef WL_EDITOR_SCRIPTING_OPERATORS_H
#define WL_EDITOR_SCRIPTING_OPERATORS_H

#include "editor/scripting/scripting.h"

/************************************************************
              Mathematical operators: + - * /
************************************************************/

// Abstract base class for all operators with two arguments
class OperatorBase : public Assignable {
public:
	~OperatorBase() override {
	}
	VariableType type() const override {
		return output_type_;
	}

	Assignable* get_a() const {
		return a_;
	}
	void set_a(Assignable& a) {
		assert(is(a.type(), input_type_));
		a_ = &a;
	}
	Assignable* get_b() const {
		return b_;
	}
	void set_b(Assignable& b) {
		assert(is(b.type(), input_type_));
		b_ = &b;
	}

	std::set<uint32_t> references() const override;

	void load(FileRead&, Loader&) override;
	void load_pointers(const ScriptingLoader&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override;

	virtual std::string op() const = 0;

protected:
	OperatorBase(VariableType in, VariableType out, Assignable*, Assignable*);

private:
	Assignable* a_;
	Assignable* b_;
	VariableType input_type_;
	VariableType output_type_;

	DISALLOW_COPY_AND_ASSIGN(OperatorBase);
};

#define OPERATOR_FACTORY(name, i, o, sign)                                                         \
	class Operator##name : public OperatorBase {                                                    \
	public:                                                                                         \
		Operator##name(Assignable* a, Assignable* b)                                                 \
		   : OperatorBase(VariableType::i, VariableType::o, a, b) {                                  \
		}                                                                                            \
		ScriptingObject::ID id() const override {                                                    \
			return ScriptingObject::ID::Operator##name;                                               \
		}                                                                                            \
		inline std::string op() const override {                                                     \
			return sign;                                                                              \
		}                                                                                            \
		DISALLOW_COPY_AND_ASSIGN(Operator##name);                                                    \
	};

OPERATOR_FACTORY(Add, Integer, Integer, "+")
OPERATOR_FACTORY(Subtract, Integer, Integer, "-")
OPERATOR_FACTORY(Multiply, Integer, Integer, "*")
OPERATOR_FACTORY(Divide, Integer, Integer, "/")

OPERATOR_FACTORY(Less, Integer, Boolean, "<")
OPERATOR_FACTORY(LessEq, Integer, Boolean, "<=")
OPERATOR_FACTORY(Greater, Integer, Boolean, ">")
OPERATOR_FACTORY(GreaterEq, Integer, Boolean, ">=")

OPERATOR_FACTORY(MathematicalEquals, Integer, Boolean, "==")
OPERATOR_FACTORY(MathematicalUnequal, Integer, Boolean, "~=")

OPERATOR_FACTORY(LogicalEquals, Boolean, Boolean, "==")
OPERATOR_FACTORY(LogicalUnequal, Boolean, Boolean, "~=")

OPERATOR_FACTORY(And, Boolean, Boolean, "and")
OPERATOR_FACTORY(Or, Boolean, Boolean, "or")

OPERATOR_FACTORY(StringConcat, String, String, "..")

#undef OPERATOR_FACTORY

// Logical not
class OperatorNot : public Assignable {
public:
	OperatorNot(Assignable* a) : a_(a) {
		if (a_)
			assert(is(a_->type(), VariableType::Boolean));
	}
	~OperatorNot() override {
	}
	VariableType type() const override {
		return VariableType::Boolean;
	}

	Assignable* get_a() const {
		return a_;
	}
	void set_a(Assignable& a) {
		assert(is(a.type(), VariableType::Boolean));
		a_ = &a;
	}

	std::set<uint32_t> references() const override;

	void load(FileRead&, Loader&) override;
	void load_pointers(const ScriptingLoader&, Loader&) override;
	void save(FileWrite&) const override;
	void write_lua(int32_t, FileWrite&) const override;

	std::string readable() const override;
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::OperatorNot;
	}

private:
	Assignable* a_;

	DISALLOW_COPY_AND_ASSIGN(OperatorNot);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_OPERATORS_H
