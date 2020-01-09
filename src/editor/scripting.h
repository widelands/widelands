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

#include <boost/function.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "logic/game_data_error.h"

class FileRead;
class FileWrite;
class ScriptingLoader;
class ScriptingSaver;

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
	Nil = 0,

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

/* WARNING:
 * If you instantiate any subclass of ScriptingObject, you HAVE TO
 * call init() on it immediately afterwards!!
 */

class ScriptingObject {
	friend class ScriptingLoader;

public:
	virtual ~ScriptingObject() {
	}

	static ScriptingObject* load(FileRead&);

	void init(ScriptingSaver& s) {
		init(s, true);
	}

	enum class ID : uint16_t {
		ConstexprString,
		ConstexprInteger,
		ConstexprBoolean,
		ConstexprNil,
		StringConcat,
		Variable,
		LuaFunction,
		FSLaunchCoroutine,
		FSFunctionCall,
		FSLocalVarDeclOrAssign,
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
	ScriptingObject() : serial_(0) {
	}

private:
	uint32_t serial_;
	static uint32_t next_serial_;

	void init(ScriptingSaver&, bool);

	DISALLOW_COPY_AND_ASSIGN(ScriptingObject);
};

// Helper struct: Everything that can stand right of " = " or be used as a function parameter.
class Assignable : virtual public ScriptingObject {
public:
	~Assignable() override {
	}

	virtual VariableType type() const = 0;

protected:
	Assignable() = default;

	DISALLOW_COPY_AND_ASSIGN(Assignable);
};

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

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

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

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

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

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

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
	// More constructors and as many attributes as there are flavours of nil…
	ConstexprNil() = default;
	~ConstexprNil() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::ConstexprNil;
	}
	VariableType type() const override {
		return VariableType::Nil;
	}
	int32_t write_lua(FileWrite&) const override;
	std::string readable() const override {
		return _("nil");
	}

	DISALLOW_COPY_AND_ASSIGN(ConstexprNil);
};

/************************************************************
                   String concatenations
************************************************************/

// A concatenation of any number of Assignables with '..'.
class StringConcat : public Assignable {
public:
	StringConcat(std::list<Assignable*> v) : values_(v) {
	}
	~StringConcat() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::StringConcat;
	}
	VariableType type() const override {
		return VariableType::String;
	}

	const std::list<Assignable*>& values() const {
		return values_;
	}
	std::list<Assignable*>& mutable_values() {
		return values_;
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

	DISALLOW_COPY_AND_ASSIGN(StringConcat);
};

/************************************************************
                          Variable
************************************************************/

class Variable : public Assignable {
public:
	Variable(VariableType, const std::string&, bool spellcheck = true);
	~Variable() override {
	}
	ScriptingObject::ID id() const override {
		return ScriptingObject::ID::Variable;
	}

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;

	VariableType type() const override {
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
class FunctionStatement : virtual public ScriptingObject {
protected:
	FunctionStatement() = default;

public:
	~FunctionStatement() override {
	}

	DISALLOW_COPY_AND_ASSIGN(FunctionStatement);
};

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

int32_t function_to_serial(FunctionBase&);
FunctionBase& serial_to_function(ScriptingLoader&, int32_t);

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

	void load(FileRead&, ScriptingLoader&) override;
	void save(FileWrite&) const override;
	int32_t write_lua(FileWrite&) const override;

	std::string readable() const override {
		return header(true);
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
		return new LuaFunction::Loader();
	}
	void load_pointers(ScriptingLoader&) override;

private:
	std::list<FunctionStatement*> body_;

	DISALLOW_COPY_AND_ASSIGN(LuaFunction);
};

/************************************************************
                  Function Statements
************************************************************/

class FS_FunctionCall : public Assignable, public FunctionStatement {
public:
	FS_FunctionCall(FunctionBase* f, Variable* v, std::list<Assignable*> p)
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
	const Variable* get_variable() const {
		return variable_;
	}
	void set_variable(Variable* v);
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

private:
	FunctionBase* function_;
	Variable* variable_;
	std::list<Assignable*> parameters_;

	DISALLOW_COPY_AND_ASSIGN(FS_FunctionCall);
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

private:
	Variable* variable_;
	Assignable* value_;
	bool declare_local_;

	DISALLOW_COPY_AND_ASSIGN(FS_LocalVarDeclOrAssign);
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

	template <typename T> std::list<T*> all() const {
		std::list<T*> result;
		for (auto& so : list_) {
			if (upcast(T, t, so.get())) {
				result.push_back(t);
			}
		}
		return result;
	}

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

/************************************************************
                      Builtin functions
************************************************************/

// Wrapper for a (static) FunctionBase object, for use in kBuiltinFunctions
struct BuiltinFunctionInfo {
	BuiltinFunctionInfo(std::string u, boost::function<std::string()> d, FunctionBase* f)
	   : function(f), unique_name(u), description(d) {
	}
	~BuiltinFunctionInfo() {
	}

	const std::unique_ptr<FunctionBase> function;
	// internal name, unique among all kBuiltinFunctions entries
	const std::string unique_name;
	// Implemented as a function to make it translatable
	const boost::function<std::string()> description;

	DISALLOW_COPY_AND_ASSIGN(BuiltinFunctionInfo);
};

// All supported builtin functions.
const extern BuiltinFunctionInfo* kBuiltinFunctions[];
// Quick access to a builtin by its unique name
const BuiltinFunctionInfo& builtin(const std::string&);

#endif  // end of include guard: WL_EDITOR_SCRIPTING_H
