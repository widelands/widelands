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

#ifndef WL_EDITOR_SCRIPTING_H
#define WL_EDITOR_SCRIPTING_H

#include <list>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>

#include "base/i18n.h"
#include "base/macros.h"
#include "logic/game_data_error.h"

class FileRead;
class FileWrite;
class ScriptingLoader;
class ScriptingSaver;

const std::string kMainFunction = "mission_thread";

// Check whether a certain name may be used as a Lua identifier for variables or functions.
// A valid name may contain only small letters, digits, and underscores;
// it may not be empty; and the first char may not be a digit.
bool is_name_valid(const char*);
bool is_name_valid(const std::string&);
// Throws an exception if the name is not a valid identifier
void check_name_valid(const std::string&);

// Lua is not typesafe, but we are. A "variable" is declared as a specific type
// and it may be assigned only values of this type and its subtypes, and nil.
// We support just about every Lua type and class the scenario scripter will need –
// classes they will likely not need are not supported. (No editor-Lua functions,
// generic classes like EGBase where only one subclass will ever be accessed, etc.)
enum class VariableType : uint16_t {
	kInvalidType = 0,  // NOCOM do we need this?

	Integer,
	Boolean,
	String,
	Table,

	Game,
	Map,
	Field,
	Player,
	Message,
	Objective,

	MapObject,

	BaseImmovable,
	Immovable,
	PlayerImmovable,
	Flag,
	Building,
	Warehouse,
	MilitarySite,
	ProductionSite,
	TrainingSite,
	PartiallyFinishedBuilding,
	ConstructionSite,
	DismantleSite,
	Market,
	RoadBase,
	Road,
	Waterway,

	Bob,
	Worker,
	Carrier,
	Ferry,
	Soldier,
	Ship,

	MapObjectDescr,

	BaseImmovableDescr,
	ImmovableDescr,
	PlayerImmovableDescr,
	FlagDescr,
	BuildingDescr,
	WarehouseDescr,
	MilitarySiteDescr,
	ProductionSiteDescr,
	TrainingSiteDescr,
	PartiallyFinishedBuildingDescr,
	ConstructionSiteDescr,
	DismantleSiteDescr,
	MarketDescr,
	RoadBaseDescr,
	RoadDescr,
	WaterwayDescr,

	BobDescr,
	WorkerDescr,
	CarrierDescr,
	FerryDescr,
	SoldierDescr,
	ShipDescr,

	TerrainDescr,
	ResourceDescr,
};

std::string descname(VariableType);
bool is(VariableType check, VariableType supposed_superclass);

/************************************************************
             Abstract ScriptingObject and Assignable
************************************************************/

class ScriptingObject {
public:
	virtual ~ScriptingObject() {
	}

	enum class ID : uint16_t {
		ConstexprString,
		ConstexprInteger,
		ConstexprBoolean,
		ConstexprNil,
		StringConcat,
		Variable,
		Function,
		FunctionCall,  // NOCOM not yet implemented
		FSLocalVarDeclOrAssign,
		FSPrint,
	};
	virtual ScriptingObject::ID id() const = 0;

	uint32_t serial() const {
		return serial_;
	}

	virtual void save(FileWrite&) const;

	// Returns the amount by which to change the indentation: `if`/`for`/`while` statements etc
	// should increase the indent level; `end` should decrease it.
	virtual int32_t write_lua(FileWrite&) const = 0;

	// Localized human-readable description of this object
	virtual std::string readable() const = 0;

	static ScriptingObject* load(FileRead&);

	struct Loader {
		Loader() {
		}
		virtual ~Loader() {
		}
	};
	virtual ScriptingObject::Loader* create_loader() const {
		return new ScriptingObject::Loader();
	}
	virtual void load(FileRead&, ScriptingLoader&);
	virtual void load_pointers(ScriptingLoader&) {
	}

protected:
	ScriptingObject(ScriptingSaver&);
	ScriptingObject() {
	}  // for saveloading only
private:
	uint32_t serial_;
	static uint32_t next_serial_;
	DISALLOW_COPY_AND_ASSIGN(ScriptingObject);
};

// Helper struct: Everything that can stand right of " = " or be used as a function parameter.
class Assignable : public ScriptingObject {
protected:
	Assignable(ScriptingSaver& s) : ScriptingObject(s) {
	}
	// for saveloading only
	Assignable() : ScriptingObject() {
	}

public:
	~Assignable() override {
	}
};

/************************************************************
                        Constexprs
************************************************************/

// A string literal, e.g. "xyz". You do NOT need to surround the value with escaped quotes –
// they will be added automatically when writing the lua file.
class ConstexprString : public Assignable {
public:
	ConstexprString(ScriptingSaver& s, const std::string& v, bool t = false)
	   : Assignable(s), value_(v), translate_(t) {
	}
	ConstexprString() : Assignable() {
	}  // for saveloading only
	~ConstexprString() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprString;
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

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

	std::string readable() const override {
		return "\"" + value_ + "\"";
	}

private:
	std::string value_;
	bool translate_;
};

// An integer constant, e.g. 123.
class ConstexprInteger : public Assignable {
public:
	ConstexprInteger(ScriptingSaver& s, int32_t i) : Assignable(s), value_(i) {
	}
	ConstexprInteger() : Assignable() {
	}  // for saveloading only
	~ConstexprInteger() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprInteger;
	}

	int32_t get_value() const {
		return value_;
	}
	void set_value(int32_t v) {
		value_ = v;
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

	std::string readable() const override {
		return std::to_string(value_);
	}

private:
	int32_t value_;
};

// A boolean constant: true or false.
class ConstexprBoolean : public Assignable {
public:
	ConstexprBoolean(ScriptingSaver& s, bool b) : Assignable(s), value_(b) {
	}
	ConstexprBoolean() : Assignable() {
	}  // for saveloading only
	~ConstexprBoolean() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprBoolean;
	}

	bool get_value() const {
		return value_;
	}
	void set_value(bool v) {
		value_ = v;
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

	std::string readable() const override {
		return value_ ? _("true") : _("false");
	}

private:
	bool value_;
};

// The nil constant.
class ConstexprNil : public Assignable {
public:
	// More constructors and as many attributes as there are flavours of nil…
	ConstexprNil(ScriptingSaver& s) : Assignable(s) {
	}
	// for saveloading only
	ConstexprNil() : Assignable() {
	}
	~ConstexprNil() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprNil;
	}
	int32_t write_lua(FileWrite&) const override;
	std::string readable() const override {
		return _("nil");
	}
};

/************************************************************
                   String concatenations
************************************************************/

// A concatenation of any number of Assignables with '..'.
class StringConcat : public Assignable {
public:
	StringConcat(ScriptingSaver&, size_t argc = 0, Assignable** argv = nullptr);
	StringConcat() : Assignable() {
	}  // for saveloading only
	~StringConcat() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::StringConcat;
	}

	const std::list<Assignable*>& values() const {
		return values_;
	}
	std::list<Assignable*>& mutable_values() {
		return values_;
	}
	void append(Assignable* a) {
		values_.push_back(a);
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

	std::string readable() const override;

	struct Loader : public ScriptingObject::Loader {
		Loader() = default;
		~Loader() override {
		}
		std::list<uint32_t> values;
	};
	ScriptingObject::Loader* create_loader() const override {
		return new StringConcat::Loader();
	}
	void load_pointers(ScriptingLoader&) override;

private:
	std::list<Assignable*> values_;
};

/************************************************************
                          Variable
************************************************************/

class Variable : public Assignable {
public:
	Variable(ScriptingSaver&, VariableType, const std::string&);
	Variable() : Assignable() {
	}  // for saveloading only
	~Variable() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::Variable;
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;

	VariableType type() const {
		return type_;
	}
	const std::string& get_name() const {
		return name_;
	}
	void rename(const std::string& n) {
		check_name_valid(n);
		name_ = n;
	}
	int32_t write_lua(FileWrite&) const override;

	std::string readable() const override {
		return name_;
	}

private:
	VariableType type_;
	std::string name_;

	DISALLOW_COPY_AND_ASSIGN(Variable);
};

/************************************************************
                       Abstract FS
************************************************************/

// Abstract superclass. Subclasses below.
class FunctionStatement : public ScriptingObject {
protected:
	FunctionStatement(ScriptingSaver& s) : ScriptingObject(s) {
	}
	// for saveloading only
	FunctionStatement() : ScriptingObject() {
	}

public:
	~FunctionStatement() override {
	}

	DISALLOW_COPY_AND_ASSIGN(FunctionStatement);
};

/************************************************************
                         Function
************************************************************/

class Function : public ScriptingObject {
public:
	Function(ScriptingSaver&, const std::string&, bool = false);
	Function() : ScriptingObject() {
	}  // for saveloading only
	~Function() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::Function;
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;
	std::string readable() const override;

	const std::string& get_name() const {
		return name_;
	}
	void rename(const std::string& n) {
		check_name_valid(n);
		name_ = n;
	}
	bool get_autostart() const {
		return autostart_;
	}
	void set_autostart(bool a) {
		autostart_ = a;
	}
	const std::list<std::pair<std::string, VariableType>>& parameters() const {
		return parameters_;
	}
	std::list<std::pair<std::string, VariableType>>& mutable_parameters() {
		return parameters_;
	}
	const std::list<FunctionStatement*>& body() const {
		return body_;
	}
	std::list<FunctionStatement*>& mutable_body() {
		return body_;
	}

	struct Loader : public ScriptingObject::Loader {
		Loader() = default;
		~Loader() override {
		}
		std::list<uint32_t> body;
	};
	ScriptingObject::Loader* create_loader() const override {
		return new Function::Loader();
	}
	void load_pointers(ScriptingLoader&) override;

private:
	std::string name_;
	bool autostart_;
	std::list<std::pair<std::string, VariableType>> parameters_;
	std::list<FunctionStatement*> body_;

	std::string header() const;

	DISALLOW_COPY_AND_ASSIGN(Function);
};

/************************************************************
                  Function Statements
************************************************************/

// Assigns a value or function result to a local or global variable.
class FS_LocalVarDeclOrAssign : public FunctionStatement {
public:
	FS_LocalVarDeclOrAssign(ScriptingSaver&, bool, Variable&, Assignable* = nullptr);
	FS_LocalVarDeclOrAssign() : FunctionStatement() {
	}  // for saveloading only
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

private:
	Variable* variable_;
	Assignable* value_;
	bool declare_local_;

	DISALLOW_COPY_AND_ASSIGN(FS_LocalVarDeclOrAssign);
};

// Lua's builtin print() function.
/* NOCOM: We do NOT want a specialised class for every single builtin function!
 * Instead we will have some `class BuiltinFunctionHeader` (with type-safe parameter lists
 * and lots of static instances to represent all supported functions) of which print() will
 * be just one specialisation of many. The builtins can then be called using the not yet
 * implemented `FunctionCall` (to be derived from both `Assignable` and `FunctionStatement`).
 * Implementing `FunctionCall` and `BuiltinFunctionHeader` is my next urgent task.
 */
class FS_Print : public FunctionStatement {
public:
	FS_Print(ScriptingSaver& s, Assignable* t) : FunctionStatement(s), text_(t) {
	}
	FS_Print() : FunctionStatement() {
	}  // for saveloading only
	~FS_Print() override {
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;
	inline ScriptingObject::ID id() const override {
		return ScriptingObject::ID::FSPrint;
	}
	std::string readable() const override;

	const Assignable* get_text() const {
		return text_;
	}
	void set_text(Assignable* t) {
		text_ = t;
	}

	struct Loader : public ScriptingObject::Loader {
		Loader() = default;
		~Loader() override {
		}
		uint32_t text;
	};
	ScriptingObject::Loader* create_loader() const override {
		return new FS_Print::Loader();
	}
	void load_pointers(ScriptingLoader&) override;

private:
	Assignable* text_;

	DISALLOW_COPY_AND_ASSIGN(FS_Print);
};

/************************************************************
                   Saveloading support
************************************************************/

/* All scripting objects are registered with the EIA's ScriptingSaver on construction.
 * It takes care of deleting them all when the editor is closed or a new map is loaded.
 * Objects loaded from file do not register themselves; that is taken care of by the
 * ScriptingLoader that calls ScriptingObject::load(). Objects not loaded from file are
 * registered with the ScriptingSaver by the ScriptingObject constructor.
 *
 * The ScriptingLoader class is instantiated in MapScenarioEditorPacket when loading
 * and is needed only until all ScriptingObjects have been loaded. Loading is performed
 * in two stages: First, all ScriptingObjects are loaded in arbitary order. More precisely:
 * They are loaded in the order in which they were saved, which is the order of their creation.
 * Since ScriptingObjects may have pointers to other ScriptingObjects which may not be loaded
 * yet, pointers are not initialized yet. Instead, the serials of the referenced objects are
 * stored in the object's Loader. When all objects were loaded, the second loading phase is
 * started by calling load_pointers(). Now the pointers are initialized to point to the
 * objects whose serials are stored in the Loader.
 */

class ScriptingSaver {
public:
	ScriptingSaver() {
	}
	~ScriptingSaver() {
	}
	void add(ScriptingObject&);
	void save(FileWrite&) const;

private:
	std::list<std::unique_ptr<ScriptingObject>> list_;
	DISALLOW_COPY_AND_ASSIGN(ScriptingSaver);
};

class ScriptingLoader {
public:
	ScriptingLoader(FileRead&, ScriptingSaver&);
	~ScriptingLoader() {
	}
	template <typename T> T& get(uint32_t serial) const {
		for (const auto& pair : list_) {
			if (pair.first->serial() == serial) {
				if (upcast(T, t, pair.first)) {
					return *t;
				}
				throw Widelands::GameDataError("ScriptingObject with serial %u is a %s, expected %s",
				                               serial, typeid(pair.first).name(), typeid(T).name());
			}
		}
		throw Widelands::GameDataError("get: ScriptingObject with serial %u not found", serial);
	}
	template <typename T> T& loader(ScriptingObject* s) const {
		for (const auto& pair : list_) {
			if (pair.first == s) {
				if (upcast(T, t, pair.second.get())) {
					return *t;
				}
				throw Widelands::GameDataError(
				   "ScriptingObject of type %s with serial %u has a loader of type %s, expected %s",
				   typeid(s).name(), s ? s->serial() : 0, typeid(*pair.second).name(),
				   typeid(T).name());
			}
		}
		throw Widelands::GameDataError(
		   "loader: ScriptingObject with serial %u not found", s ? s->serial() : 0);
	}

private:
	std::map<ScriptingObject*, std::unique_ptr<ScriptingObject::Loader>> list_;
	DISALLOW_COPY_AND_ASSIGN(ScriptingLoader);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_H
