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

#ifndef WL_EDITOR_SCRIPTING_SCRIPTING_H
#define WL_EDITOR_SCRIPTING_SCRIPTING_H

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"

class EditorInteractive;
class FunctionBase;
class Property;
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
// Do not change the order!
enum class VariableTypeID : uint16_t {
	Nil = 0,
	// The Any type is BAD STYLE!!! DO NOT USE!!!
	// Required for certain builtin functions like array_combine() and save_campaign_data()
	Any,
	// One type for both tables and arrays: Arrays are Tables with Integer as key type
	Table,

	Integer,
	Double,
	Boolean,
	String,

	Game,
	Map,
	Field,
	Player,
	Message,
	Objective,
	Economy,

	MapObject,

	BaseImmovable,
	Immovable,
	PlayerImmovable,
	Flag,
	PortDock,
	Building,
	Warehouse,
	MilitarySite,
	ProductionSite,
	TrainingSite,
	ConstructionSite,
	DismantleSite,
	Market,
	Road,

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
	ConstructionSiteDescr,
	DismantleSiteDescr,
	MarketDescr,
	RoadDescr,

	BobDescr,
	WorkerDescr,
	CarrierDescr,
	FerryDescr,
	SoldierDescr,
	ShipDescr,

	WareDescr,
	TribeDescr,

	TerrainDescr,
	ResourceDescr,
};

struct VariableType {
public:
	VariableType(VariableTypeID i) : id_(i), key_(nullptr), value_(nullptr) {
	}
	VariableType(const VariableType& k, const VariableType& v)
	   : id_(VariableTypeID::Table), key_(new VariableType(k)), value_(new VariableType(v)) {
	}
	VariableType(FileRead&);

	VariableType(const VariableType& vt)
	   : id_(vt.id_),
	     key_(vt.key_ ? new VariableType(*vt.key_) : nullptr),
	     value_(vt.value_ ? new VariableType(*vt.value_) : nullptr) {
	}
	VariableType& operator=(const VariableType& vt) {
		id_ = vt.id_;
		key_.reset(vt.key_ ? new VariableType(*vt.key_) : nullptr);
		value_.reset(vt.value_ ? new VariableType(*vt.value_) : nullptr);
		return *this;
	}

	~VariableType() {
	}

	const VariableTypeID& id() const {
		return id_;
	}
	const VariableType& key_type() const {
		assert(key_);
		return *key_;
	}
	const VariableType& value_type() const {
		assert(value_);
		return *value_;
	}

	bool is_subclass(const VariableType&) const;

	void write(FileWrite&) const;

	bool operator==(const VariableType& v) const {
		return id_ == v.id_ &&
		       (id_ != VariableTypeID::Table || (*key_ == *v.key_ && *value_ == *v.value_));
	}

private:
	VariableTypeID id_;

	// Valid only for tables
	std::unique_ptr<VariableType> key_;
	std::unique_ptr<VariableType> value_;
};

std::string descname(const VariableType&);
bool is(VariableTypeID check, VariableTypeID supposed_superclass);

/************************************************************
             Abstract ScriptingObject and Assignable
************************************************************/

using Loader = std::list<int32_t>;

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

	// Do not change the order!
	enum class ID : uint16_t {
		ConstexprNil = 0,
		ConstexprString,
		ConstexprInteger,
		ConstexprBoolean,
		Variable,
		GetProperty,
		GetTable,
		LuaFunction,
		FSReturn,
		FSBreak,
		FSLaunchCoroutine,
		FSFunctionCall,
		FSSetProperty,
		FSSetTable,
		FSLocalVarDeclOrAssign,
		OperatorIsNil,
		OperatorNot,
		OperatorAnd,
		OperatorOr,
		OperatorLogicalEquals,
		OperatorLogicalUnequal,
		OperatorMathematicalEquals,
		OperatorMathematicalUnequal,
		OperatorAdd,
		OperatorSubtract,
		OperatorMultiply,
		OperatorDivide,
		OperatorModulo,
		OperatorLess,
		OperatorLessEq,
		OperatorGreater,
		OperatorGreaterEq,
		OperatorStringConcat,
		FSIf,
		FSFor,
		FSForEach,
		FSWhile,
	};
	virtual ScriptingObject::ID id() const = 0;

	uint32_t serial() const {
		if (!serial_)
			throw wexception("ScriptingObject of type %s was not initialized", typeid(*this).name());
		return serial_;
	}

	virtual void save(FileWrite&) const;

	virtual void write_lua(int32_t, FileWrite&) const = 0;

	// ± localized human-readable description of this object
	virtual std::string readable() const = 0;

	// Throws wexception if some argument is invalid
	virtual void selftest() const;

	// Returns the serials of all ScriptingObjects this ScriptingObject references in some way (e.g.
	// assignments reference a variable and a value)
	virtual std::set<uint32_t> references() const {
		return {};
	}

	virtual void load(FileRead&, Loader&);
	virtual void load_pointers(const ScriptingLoader&, Loader&) {
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

// Helper class: Everything that can stand right of "=" or left of '.' and ':' or be used as a
// function parameter.
class Assignable : virtual public ScriptingObject {
public:
	~Assignable() override {
	}

	virtual const VariableType& type() const = 0;

protected:
	Assignable() = default;

	DISALLOW_COPY_AND_ASSIGN(Assignable);
};

/************************************************************
                       Abstract FS
************************************************************/

// Abstract superclass
class FunctionStatement : virtual public ScriptingObject {
protected:
	FunctionStatement() = default;

public:
	~FunctionStatement() override {
	}

	DISALLOW_COPY_AND_ASSIGN(FunctionStatement);
};

/************************************************************
                   Saveloading support
************************************************************/

/* All scripting objects are registered with the EIA's ScriptingSaver on construction.
 * It takes care of deleting them all when the editor is closed or a new map is loaded.
 * Objects loaded from file do not register themselves; that is taken care of by the
 * ScriptingLoader that calls ScriptingObject::load(). Objects not loaded from file are
 * registered with the ScriptingSaver by ScriptingObject::init().
 *
 * The ScriptingLoader class is instantiated in MapScenarioEditorPacket when loading
 * and is needed only until all ScriptingObjects have been loaded. Loading is performed
 * in two stages: First, all ScriptingObjects are loaded in arbitary order. (More precisely,
 * they are loaded in the order in which they were saved, which is the order of their creation.)
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
	void delete_unused(const EditorInteractive&);
	void selftest() const;

	template <typename T> std::list<T*> all() const {
		std::list<T*> result;
		for (auto& so : list_) {
			if (upcast(T, t, so.get())) {
				result.push_back(t);
			}
		}
		return result;
	}
	template <typename T> T& get(uint32_t serial) const {
		for (const auto& so : list_) {
			if (so->serial() == serial) {
				if (upcast(T, t, so.get())) {
					return *t;
				}
				throw Widelands::GameDataError(
				   "ScriptingSaver::get: ScriptingObject with serial %u is a %s, expected %s", serial,
				   typeid(*so).name(), typeid(T).name());
			}
		}
		throw Widelands::GameDataError(
		   "ScriptingLoader::get: ScriptingObject with serial %u not found", serial);
	}
	template <typename T> T* get(const std::string& name) const {
		for (const auto& so : all<T>()) {
			if (so->get_name() == name) {
				return so;
			}
		}
		return nullptr;
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
				throw Widelands::GameDataError(
				   "ScriptingLoader::get: ScriptingObject with serial %u is a %s, expected %s", serial,
				   typeid(pair.first).name(), typeid(T).name());
			}
		}
		throw Widelands::GameDataError(
		   "ScriptingLoader::get: ScriptingObject with serial %u not found", serial);
	}

private:
	std::map<ScriptingObject*, std::unique_ptr<Loader>> list_;
	DISALLOW_COPY_AND_ASSIGN(ScriptingLoader);
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_SCRIPTING_H
