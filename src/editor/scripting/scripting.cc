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

#include "editor/scripting/scripting.h"

#include <memory>

#include "editor/editorinteractive.h"
#include "editor/scripting/builtin.h"
#include "editor/scripting/constexpr.h"
#include "editor/scripting/control_structures.h"
#include "editor/scripting/function.h"
#include "editor/scripting/function_statements.h"
#include "editor/scripting/operators.h"
#include "editor/scripting/variable.h"

// Identifier helper functions
bool is_name_valid(const char* name) {
	if (name == nullptr)
		return false;
	if ((name[0] < 'a' || name[0] > 'z') && name[0] != '_')
		return false;
	for (const char* c = name; *c; ++c)
		if ((*c < 'a' || *c > 'z') && (*c < '0' || *c > '9') && *c != '_')
			return false;
	return true;
}
bool is_name_valid(const std::string& name) {
	return is_name_valid(name.c_str());
}
void check_name_valid(const std::string& name) {
	if (!is_name_valid(name)) {
		throw wexception("Invalid variable or function name: %s", name.c_str());
	}
}

/************************************************************
             ScriptingObject, Assignable, and
        FunctionStatement abstract implementations
************************************************************/

uint32_t ScriptingObject::next_serial_ = 0;

constexpr uint16_t kCurrentPacketVersionScriptingObjectHeader = 1;
constexpr uint16_t kCurrentPacketVersionScriptingObject = 1;

void ScriptingObject::init(ScriptingSaver& s, bool init_serial) {
	if (init_serial) {
		if (serial_) {
			throw wexception("ScriptingObject::init: %u already initialized", serial_);
		}
		serial_ = (++next_serial_);
	} else {
		if (!serial_) {
			throw wexception("ScriptingObject::init: not initialized yet");
		}
	}
	s.add(*this);
}
void ScriptingObject::load(FileRead& fr, Loader&) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScriptingObject) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingObject", packet_version, kCurrentPacketVersionScriptingObject);
		}
		serial_ = fr.unsigned_32();
		next_serial_ = std::max(next_serial_, serial_);
	} catch (const WException& e) {
		throw wexception("editor abstract scripting object: %s", e.what());
	}
}

void ScriptingObject::save(FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersionScriptingObjectHeader);
	fw.unsigned_16(static_cast<uint16_t>(id()));
	fw.unsigned_16(kCurrentPacketVersionScriptingObject);
	fw.unsigned_32(serial_);
}

// static
ScriptingObject* ScriptingObject::load(FileRead& fr) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScriptingObjectHeader) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingObject Header", packet_version, kCurrentPacketVersionScriptingObjectHeader);
		}
		const uint16_t id = fr.unsigned_16();
		switch (static_cast<ID>(id)) {
		case ID::ConstexprString:
			return new ConstexprString("");
		case ID::ConstexprInteger:
			return new ConstexprInteger(0);
		case ID::ConstexprBoolean:
			return new ConstexprBoolean(false);
		case ID::ConstexprNil:
			return new ConstexprNil();
		case ID::Variable:
			return new Variable(VariableType::Nil, "", false);
		case ID::GetProperty:
			return new GetProperty(nullptr, nullptr);
		case ID::LuaFunction:
			return new LuaFunction("", false);
		case ID::FSFunctionCall:
			return new FS_FunctionCall(nullptr, nullptr, {});
		case ID::FSLocalVarDeclOrAssign:
			return new FS_LocalVarDeclOrAssign(false, nullptr, nullptr);
		case ID::FSLaunchCoroutine:
			return new FS_LaunchCoroutine(nullptr);
		case ID::FSSetProperty:
			return new FS_SetProperty(nullptr, nullptr, nullptr);
		case ID::OperatorNot:
			return new OperatorNot(nullptr);
		case ID::OperatorAnd:
			return new OperatorAnd(nullptr, nullptr);
		case ID::OperatorOr:
			return new OperatorOr(nullptr, nullptr);
		case ID::OperatorLogicalEquals:
			return new OperatorLogicalEquals(nullptr, nullptr);
		case ID::OperatorLogicalUnequal:
			return new OperatorLogicalUnequal(nullptr, nullptr);
		case ID::OperatorMathematicalEquals:
			return new OperatorMathematicalEquals(nullptr, nullptr);
		case ID::OperatorMathematicalUnequal:
			return new OperatorMathematicalUnequal(nullptr, nullptr);
		case ID::OperatorAdd:
			return new OperatorAdd(nullptr, nullptr);
		case ID::OperatorSubtract:
			return new OperatorSubtract(nullptr, nullptr);
		case ID::OperatorMultiply:
			return new OperatorMultiply(nullptr, nullptr);
		case ID::OperatorDivide:
			return new OperatorDivide(nullptr, nullptr);
		case ID::OperatorGreater:
			return new OperatorGreater(nullptr, nullptr);
		case ID::OperatorGreaterEq:
			return new OperatorGreaterEq(nullptr, nullptr);
		case ID::OperatorLess:
			return new OperatorLess(nullptr, nullptr);
		case ID::OperatorLessEq:
			return new OperatorLessEq(nullptr, nullptr);
		case ID::OperatorStringConcat:
			return new OperatorStringConcat(nullptr, nullptr);
		case ID::FSWhile:
			return new FS_While(false, nullptr);
		case ID::FSIf:
			return new FS_If(nullptr);
		default:
			throw Widelands::GameDataError("Invalid ScriptingObject ID: %u", id);
		}
	} catch (const WException& e) {
		throw wexception("editor abstract scripting object: %s", e.what());
	}
}

// static
int32_t function_to_serial(FunctionBase& f) {
	if (upcast(ScriptingObject, so, &f)) {
		return so->serial();
	}
	for (int32_t i = 0; kBuiltinFunctions[i]; ++i) {
		if (kBuiltinFunctions[i]->function.get() == &f) {
			return -i;
		}
	}
	NEVER_HERE();
}
// static
FunctionBase& serial_to_function(const ScriptingLoader& l, int32_t s) {
	return s > 0 ? l.get<LuaFunction>(s) : *kBuiltinFunctions[-s]->function;
}
// static
uint32_t property_to_serial(Property& p) {
	for (uint32_t i = 0; kBuiltinProperties[i]; ++i) {
		if (kBuiltinProperties[i]->property.get() == &p) {
			return i;
		}
	}
	NEVER_HERE();
}

/************************************************************
             VariableType static helper functions
************************************************************/

// static
std::string descname(VariableType t) {
	switch (t) {
	case VariableType::Nil:
		return _("Nil");
	case VariableType::Integer:
		return _("Integer");
	case VariableType::Double:
		return _("Double");
	case VariableType::Boolean:
		return _("Boolean");
	case VariableType::String:
		return _("String");
	case VariableType::Table:
		return _("Table");
	case VariableType::Game:
		return _("Game");
	case VariableType::Map:
		return _("Map");
	case VariableType::Field:
		return _("Field");
	case VariableType::Player:
		return _("Player");
	case VariableType::Message:
		return _("Message");
	case VariableType::Objective:
		return _("Objective");
	case VariableType::Economy:
		return _("Economy");
	case VariableType::MapObject:
		return _("Map Object");
	case VariableType::BaseImmovable:
		return _("Generic Immovable");
	case VariableType::Immovable:
		return _("Map Immovable");
	case VariableType::PlayerImmovable:
		return _("Generic Player Immovable");
	case VariableType::Flag:
		return _("Flag");
	case VariableType::PortDock:
		return _("Port Dock");
	case VariableType::Building:
		return _("Generic Building");
	case VariableType::Warehouse:
		return _("Warehouse");
	case VariableType::MilitarySite:
		return _("Militarysite");
	case VariableType::ProductionSite:
		return _("Productionsite");
	case VariableType::TrainingSite:
		return _("Trainingsite");
	case VariableType::PartiallyFinishedBuilding:
		return _("Generic Partially Finished Building");
	case VariableType::ConstructionSite:
		return _("Constructionsite");
	case VariableType::DismantleSite:
		return _("Dismantlesite");
	case VariableType::Market:
		return _("Market");
	case VariableType::RoadBase:
		return _("Generic Road");
	case VariableType::Road:
		return _("Road");
	case VariableType::Waterway:
		return _("Waterway");
	case VariableType::Bob:
		return _("Bob");
	case VariableType::Worker:
		return _("Worker");
	case VariableType::Carrier:
		return _("Carrier");
	case VariableType::Ferry:
		return _("Ferry");
	case VariableType::Soldier:
		return _("Soldier");
	case VariableType::Ship:
		return _("Ship");
	case VariableType::MapObjectDescr:
		return _("Map Object Description");
	case VariableType::BaseImmovableDescr:
		return _("Generic Immovable Description");
	case VariableType::ImmovableDescr:
		return _("Map Immovable Description");
	case VariableType::PlayerImmovableDescr:
		return _("Generic Player Immovable Description");
	case VariableType::FlagDescr:
		return _("Flag Description");
	case VariableType::BuildingDescr:
		return _("Generic Building Description");
	case VariableType::WarehouseDescr:
		return _("Warehouse Description");
	case VariableType::MilitarySiteDescr:
		return _("Militarysite Description");
	case VariableType::ProductionSiteDescr:
		return _("Productionsite Description");
	case VariableType::TrainingSiteDescr:
		return _("Trainingsite Description");
	case VariableType::PartiallyFinishedBuildingDescr:
		return _("Generic Partially Finished Building Description");
	case VariableType::ConstructionSiteDescr:
		return _("Constructionsite Description");
	case VariableType::DismantleSiteDescr:
		return _("Dismantlesite Description");
	case VariableType::MarketDescr:
		return _("Market Description");
	case VariableType::RoadBaseDescr:
		return _("Generic Road Description");
	case VariableType::RoadDescr:
		return _("Road Description");
	case VariableType::WaterwayDescr:
		return _("Waterway Description");
	case VariableType::BobDescr:
		return _("Bob Description");
	case VariableType::WorkerDescr:
		return _("Worker Description");
	case VariableType::CarrierDescr:
		return _("Carrier Description");
	case VariableType::FerryDescr:
		return _("Ferry Description");
	case VariableType::SoldierDescr:
		return _("Soldier Description");
	case VariableType::ShipDescr:
		return _("Ship Description");
	case VariableType::TribeDescr:
		return _("Tribe Description");
	case VariableType::WareDescr:
		return _("Ware Description");
	case VariableType::TerrainDescr:
		return _("Terrain Description");
	case VariableType::ResourceDescr:
		return _("Resource Description");
	default:
		NEVER_HERE();
	}
}

// Checks whether a variable of type s may be assigned a value of type t.
// This is the case if t==s, or t is a subclass of s.
// We do so by recursively calling is(s, direct_superclass_of_t)

// static
bool is(VariableType t, VariableType s) {
	if (t == s || t == VariableType::Nil || s == VariableType::Boolean) {
		return true;
	}
	switch (t) {
	case VariableType::String:
	case VariableType::Integer:
	case VariableType::Double:
		// Implicit conversion to String is allowed for numbers so they can be concatenated with '..'
		return is(s, VariableType::String);

	case VariableType::Boolean:
	case VariableType::Table:
	case VariableType::Game:
	case VariableType::Map:
	case VariableType::Field:
	case VariableType::Player:
	case VariableType::Message:
	case VariableType::Objective:
	case VariableType::Economy:
	case VariableType::MapObject:
	case VariableType::MapObjectDescr:
	case VariableType::TribeDescr:
	case VariableType::WareDescr:
	case VariableType::TerrainDescr:
	case VariableType::ResourceDescr:
		return false;

	case VariableType::BaseImmovable:
	case VariableType::Bob:
		return s == VariableType::MapObject;
	case VariableType::Immovable:
	case VariableType::PlayerImmovable:
		return is(s, VariableType::BaseImmovable);
	case VariableType::Flag:
	case VariableType::PortDock:
	case VariableType::Building:
	case VariableType::RoadBase:
		return is(s, VariableType::PlayerImmovable);
	case VariableType::Warehouse:
	case VariableType::MilitarySite:
	case VariableType::ProductionSite:
	case VariableType::PartiallyFinishedBuilding:
	case VariableType::Market:
		return is(s, VariableType::Building);
	case VariableType::TrainingSite:
		return is(s, VariableType::ProductionSite);
	case VariableType::ConstructionSite:
	case VariableType::DismantleSite:
		return is(s, VariableType::PartiallyFinishedBuilding);
	case VariableType::Road:
	case VariableType::Waterway:
		return is(s, VariableType::RoadBase);
	case VariableType::Ship:
	case VariableType::Worker:
		return is(s, VariableType::Bob);
	case VariableType::Soldier:
	case VariableType::Carrier:
		return is(s, VariableType::Worker);
	case VariableType::Ferry:
		return is(s, VariableType::Carrier);
	case VariableType::BaseImmovableDescr:
	case VariableType::BobDescr:
		return s == VariableType::MapObjectDescr;
	case VariableType::ImmovableDescr:
	case VariableType::PlayerImmovableDescr:
		return is(s, VariableType::BaseImmovableDescr);
	case VariableType::FlagDescr:
	case VariableType::BuildingDescr:
	case VariableType::RoadBaseDescr:
		return is(s, VariableType::PlayerImmovableDescr);
	case VariableType::WarehouseDescr:
	case VariableType::MilitarySiteDescr:
	case VariableType::ProductionSiteDescr:
	case VariableType::PartiallyFinishedBuildingDescr:
	case VariableType::MarketDescr:
		return is(s, VariableType::BuildingDescr);
	case VariableType::TrainingSiteDescr:
		return is(s, VariableType::ProductionSiteDescr);
	case VariableType::ConstructionSiteDescr:
	case VariableType::DismantleSiteDescr:
		return is(s, VariableType::PartiallyFinishedBuildingDescr);
	case VariableType::RoadDescr:
	case VariableType::WaterwayDescr:
		return is(s, VariableType::RoadBaseDescr);
	case VariableType::WorkerDescr:
	case VariableType::ShipDescr:
		return is(s, VariableType::BobDescr);
	case VariableType::SoldierDescr:
	case VariableType::CarrierDescr:
		return is(s, VariableType::WorkerDescr);
	case VariableType::FerryDescr:
		return is(s, VariableType::CarrierDescr);
	default:
		NEVER_HERE();
	}
}

/************************************************************
                       Saveloading
************************************************************/

constexpr uint16_t kCurrentPacketVersionScripting = 1;

void ScriptingSaver::add(ScriptingObject& a) {
	list_.push_back(std::unique_ptr<ScriptingObject>(&a));
}

void ScriptingSaver::save(FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersionScripting);
	fw.unsigned_32(list_.size());
	for (const auto& a : list_) {
		a->save(fw);
	}
}

void ScriptingSaver::delete_unused(const EditorInteractive& eia) {
	std::list<const ScriptingObject*> cores;
	std::set<uint32_t> reached;
	for (const auto* v : eia.variables()) {
		cores.push_back(v);
		reached.insert(v->serial());
	}
	for (const auto* f : eia.functions()) {
		cores.push_back(f);
		reached.insert(f->serial());
	}
	while (!cores.empty()) {
		const ScriptingObject& so = *cores.back();
		cores.pop_back();
		for (uint32_t serial : so.references()) {
			if (!reached.count(serial)) {
				reached.insert(serial);
				cores.push_back(&get<ScriptingObject>(serial));
			}
		}
	}
	std::list<uint32_t> remove;
	for (auto& s : list_) {
		if (!reached.count(s->serial())) {
			remove.push_back(s->serial());
		}
	}
	if (remove.empty())
		return;
	log("ScriptingSaver::cleanup(): Deleting %" PRIuS " item(s):\n", remove.size());
	while (!remove.empty()) {
		bool found = false;
		for (auto it = list_.begin(); it != list_.end(); ++it) {
			if ((*it)->serial() == remove.back()) {
				found = true;
				log(" · %u %s\n", (*it)->serial(), typeid(**it).name());
				remove.pop_back();
				list_.erase(it);
				break;
			}
		}
		if (!found)
			throw wexception(
			   "ScriptingSaver::cleanup(): Attempt to delete nonexistent object %u", remove.back());
	}
}

ScriptingLoader::ScriptingLoader(FileRead& fr, ScriptingSaver& s) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScripting) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingSaver", packet_version, kCurrentPacketVersionScripting);
		}

		// First load phase: Create all saved objects.
		for (uint32_t n = fr.unsigned_32(); n; --n) {
			ScriptingObject* a = ScriptingObject::load(fr);
			Loader* loader = new Loader();
			a->load(fr, *loader);
			a->init(s, false);
			list_.emplace(std::make_pair(a, std::unique_ptr<Loader>(loader)));
		}

		// Second load phase: Load pointers between objects.
		for (const auto& pair : list_) {
			pair.first->load_pointers(*this, *pair.second);
			if (!pair.second->empty()) {
				throw Widelands::GameDataError(
				   "load_pointers: %" PRIuS " items left in loader for %s %u", pair.second->size(),
				   typeid(*pair.first).name(), pair.first->serial());
			}
		}
	} catch (const WException& e) {
		throw wexception("ScriptingLoader: %s", e.what());
	}
}
