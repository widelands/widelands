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
#include "editor/scripting/constexpr.h"
#include "editor/scripting/control_structures.h"
#include "editor/scripting/function.h"
#include "editor/scripting/function_statements.h"
#include "editor/scripting/operators.h"
#include "editor/scripting/variable.h"

// Identifier helper functions
bool is_name_valid(const char* name) {
	if (name == nullptr) {
		return false;
	}
	if ((name[0] < 'a' || name[0] > 'z') && name[0] != '_') {
		return false;
	}
	for (const char* c = name; *c; ++c) {
		if ((*c < 'a' || *c > 'z') && (*c < '0' || *c > '9') && *c != '_') {
			return false;
		}
	}
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

void ScriptingObject::selftest() const {
	if (!serial_) {
		throw wexception("not initialized");
	}
}
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
			return new Variable(VariableType(VariableTypeID::Nil), "");
		case ID::GetProperty:
			return new GetProperty(nullptr, nullptr);
		case ID::GetTable:
			return new GetTable(nullptr, nullptr);
		case ID::LuaFunction:
			return new LuaFunction("");
		case ID::FSReturn:
			return new FS_Return(nullptr);
		case ID::FSBreak:
			return new FS_Break();
		case ID::FSFunctionCall:
			return new FS_FunctionCall(nullptr, nullptr, {});
		case ID::FSLocalVarDeclOrAssign:
			return new FS_LocalVarDeclOrAssign(false, nullptr, nullptr);
		case ID::FSLaunchCoroutine:
			return new FS_LaunchCoroutine(nullptr);
		case ID::FSSetProperty:
			return new FS_SetProperty(nullptr, nullptr, nullptr);
		case ID::FSSetTable:
			return new FS_SetTable(nullptr, nullptr, nullptr);
		case ID::OperatorIsNil:
			return new OperatorIsNil(nullptr);
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
		case ID::OperatorModulo:
			return new OperatorModulo(nullptr, nullptr);
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
		case ID::FSFor:
			return new FS_For(nullptr, nullptr, nullptr);
		case ID::FSForEach:
			return new FS_ForEach(nullptr, nullptr, nullptr);
		case ID::FSIf:
			return new FS_If(nullptr);
		default:
			throw Widelands::GameDataError("Invalid ScriptingObject ID: %u", id);
		}
	} catch (const WException& e) {
		throw wexception("editor abstract scripting object: %s", e.what());
	}
}

/************************************************************
                       VariableTypes
************************************************************/

constexpr uint16_t kCurrentPacketVersionVariableType = 1;

VariableType::VariableType(FileRead& fr) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionVariableType) {
			throw Widelands::UnhandledVersionError(
			   "VariableType", packet_version, kCurrentPacketVersionVariableType);
		}
		id_ = static_cast<VariableTypeID>(fr.unsigned_16());
		if (id_ == VariableTypeID::Table) {
			key_.reset(new VariableType(fr));
			value_.reset(new VariableType(fr));
		} else {
			key_.reset(nullptr);
			value_.reset(nullptr);
		}
	} catch (const WException& e) {
		throw wexception("VariableType: %s", e.what());
	}
}
void VariableType::write(FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersionVariableType);
	fw.unsigned_16(static_cast<uint16_t>(id_));
	if (id_ == VariableTypeID::Table) {
		key_->write(fw);
		value_->write(fw);
	}
}

bool VariableType::is_subclass(const VariableType& v) const {
	if (id_ == VariableTypeID::Table) {
		return v.id_ == VariableTypeID::Table && key_->is_subclass(*v.key_) &&
		       value_->is_subclass(*v.value_);
	}
	return is(id_, v.id_);
}

// static
std::string descname(const VariableType& t) {
	switch (t.id()) {
	case VariableTypeID::Table:
		return (boost::format(_("Table<%1$s = %2$s>")) % descname(t.key_type()) %
		        descname(t.value_type()))
		   .str();
	case VariableTypeID::Any:
		return _("Any");
	case VariableTypeID::Nil:
		return _("Nil");
	case VariableTypeID::Integer:
		return _("Integer");
	case VariableTypeID::Double:
		return _("Double");
	case VariableTypeID::Boolean:
		return _("Boolean");
	case VariableTypeID::String:
		return _("String");
	case VariableTypeID::Game:
		return _("Game");
	case VariableTypeID::Map:
		return _("Map");
	case VariableTypeID::Field:
		return _("Field");
	case VariableTypeID::Player:
		return _("Player");
	case VariableTypeID::Message:
		return _("Message");
	case VariableTypeID::Objective:
		return _("Objective");
	case VariableTypeID::Economy:
		return _("Economy");
	case VariableTypeID::MapObject:
		return _("Map Object");
	case VariableTypeID::BaseImmovable:
		return _("Generic Immovable");
	case VariableTypeID::Immovable:
		return _("Map Immovable");
	case VariableTypeID::PlayerImmovable:
		return _("Generic Player Immovable");
	case VariableTypeID::Flag:
		return _("Flag");
	case VariableTypeID::PortDock:
		return _("Port Dock");
	case VariableTypeID::Building:
		return _("Generic Building");
	case VariableTypeID::Warehouse:
		return _("Warehouse");
	case VariableTypeID::MilitarySite:
		return _("Militarysite");
	case VariableTypeID::ProductionSite:
		return _("Productionsite");
	case VariableTypeID::TrainingSite:
		return _("Trainingsite");
	case VariableTypeID::ConstructionSite:
		return _("Constructionsite");
	case VariableTypeID::DismantleSite:
		return _("Dismantlesite");
	case VariableTypeID::Market:
		return _("Market");
	case VariableTypeID::Road:
		return _("Road");
	case VariableTypeID::Bob:
		return _("Bob");
	case VariableTypeID::Worker:
		return _("Worker");
	case VariableTypeID::Carrier:
		return _("Carrier");
	case VariableTypeID::Ferry:
		return _("Ferry");
	case VariableTypeID::Soldier:
		return _("Soldier");
	case VariableTypeID::Ship:
		return _("Ship");
	case VariableTypeID::MapObjectDescr:
		return _("Map Object Description");
	case VariableTypeID::BaseImmovableDescr:
		return _("Generic Immovable Description");
	case VariableTypeID::ImmovableDescr:
		return _("Map Immovable Description");
	case VariableTypeID::PlayerImmovableDescr:
		return _("Generic Player Immovable Description");
	case VariableTypeID::FlagDescr:
		return _("Flag Description");
	case VariableTypeID::BuildingDescr:
		return _("Generic Building Description");
	case VariableTypeID::WarehouseDescr:
		return _("Warehouse Description");
	case VariableTypeID::MilitarySiteDescr:
		return _("Militarysite Description");
	case VariableTypeID::ProductionSiteDescr:
		return _("Productionsite Description");
	case VariableTypeID::TrainingSiteDescr:
		return _("Trainingsite Description");
	case VariableTypeID::ConstructionSiteDescr:
		return _("Constructionsite Description");
	case VariableTypeID::DismantleSiteDescr:
		return _("Dismantlesite Description");
	case VariableTypeID::MarketDescr:
		return _("Market Description");
	case VariableTypeID::RoadDescr:
		return _("Road Description");
	case VariableTypeID::BobDescr:
		return _("Bob Description");
	case VariableTypeID::WorkerDescr:
		return _("Worker Description");
	case VariableTypeID::CarrierDescr:
		return _("Carrier Description");
	case VariableTypeID::FerryDescr:
		return _("Ferry Description");
	case VariableTypeID::SoldierDescr:
		return _("Soldier Description");
	case VariableTypeID::ShipDescr:
		return _("Ship Description");
	case VariableTypeID::TribeDescr:
		return _("Tribe Description");
	case VariableTypeID::WareDescr:
		return _("Ware Description");
	case VariableTypeID::TerrainDescr:
		return _("Terrain Description");
	case VariableTypeID::ResourceDescr:
		return _("Resource Description");
	default:
		NEVER_HERE();
	}
}

// Checks whether a variable of type s may be assigned a value of type t.
// This is the case if t==s, or t is a subclass of s.
// We do so by recursively calling is(s, direct_superclass_of_t).
// Implicit conversion to boolean is not permitted, use OperatorIsNil and OperatorNot(OperatorIsNil)
// instead.

// static
bool is(VariableTypeID t, VariableTypeID s) {
	if (t == s || t == VariableTypeID::Nil || s == VariableTypeID::Any) {
		return true;
	}
	switch (t) {
	case VariableTypeID::Boolean:
	case VariableTypeID::String:
	case VariableTypeID::Integer:
	case VariableTypeID::Double:
	case VariableTypeID::Table:
	case VariableTypeID::Game:
	case VariableTypeID::Map:
	case VariableTypeID::Field:
	case VariableTypeID::Player:
	case VariableTypeID::Message:
	case VariableTypeID::Objective:
	case VariableTypeID::Economy:
	case VariableTypeID::MapObject:
	case VariableTypeID::MapObjectDescr:
	case VariableTypeID::TribeDescr:
	case VariableTypeID::WareDescr:
	case VariableTypeID::TerrainDescr:
	case VariableTypeID::ResourceDescr:
		return false;

	case VariableTypeID::BaseImmovable:
	case VariableTypeID::Bob:
		return s == VariableTypeID::MapObject;
	case VariableTypeID::Immovable:
	case VariableTypeID::PlayerImmovable:
		return is(s, VariableTypeID::BaseImmovable);
	case VariableTypeID::Flag:
	case VariableTypeID::PortDock:
	case VariableTypeID::Building:
	case VariableTypeID::Road:
		return is(s, VariableTypeID::PlayerImmovable);
	case VariableTypeID::Warehouse:
	case VariableTypeID::MilitarySite:
	case VariableTypeID::ProductionSite:
	case VariableTypeID::ConstructionSite:
	case VariableTypeID::DismantleSite:
	case VariableTypeID::Market:
		return is(s, VariableTypeID::Building);
	case VariableTypeID::TrainingSite:
		return is(s, VariableTypeID::ProductionSite);
	case VariableTypeID::Ship:
	case VariableTypeID::Worker:
		return is(s, VariableTypeID::Bob);
	case VariableTypeID::Soldier:
	case VariableTypeID::Carrier:
		return is(s, VariableTypeID::Worker);
	case VariableTypeID::Ferry:
		return is(s, VariableTypeID::Carrier);
	case VariableTypeID::BaseImmovableDescr:
	case VariableTypeID::BobDescr:
		return s == VariableTypeID::MapObjectDescr;
	case VariableTypeID::ImmovableDescr:
	case VariableTypeID::PlayerImmovableDescr:
		return is(s, VariableTypeID::BaseImmovableDescr);
	case VariableTypeID::FlagDescr:
	case VariableTypeID::BuildingDescr:
	case VariableTypeID::RoadDescr:
		return is(s, VariableTypeID::PlayerImmovableDescr);
	case VariableTypeID::WarehouseDescr:
	case VariableTypeID::MilitarySiteDescr:
	case VariableTypeID::ProductionSiteDescr:
	case VariableTypeID::ConstructionSiteDescr:
	case VariableTypeID::DismantleSiteDescr:
	case VariableTypeID::MarketDescr:
		return is(s, VariableTypeID::BuildingDescr);
	case VariableTypeID::TrainingSiteDescr:
		return is(s, VariableTypeID::ProductionSiteDescr);
	case VariableTypeID::WorkerDescr:
	case VariableTypeID::ShipDescr:
		return is(s, VariableTypeID::BobDescr);
	case VariableTypeID::SoldierDescr:
	case VariableTypeID::CarrierDescr:
		return is(s, VariableTypeID::WorkerDescr);
	case VariableTypeID::FerryDescr:
		return is(s, VariableTypeID::CarrierDescr);
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

void ScriptingSaver::selftest() const {
	for (const auto& a : list_) {
		if (!a) {
			throw wexception("ScriptingSaver: A list entry is nullptr");
		}
		try {
			a->selftest();
		} catch (const WException& w) {
			throw wexception("selftest for ScriptingObject %s %u failed: %s", typeid(*a).name(),
			                 a->serial(), w.what());
		}
	}
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
	if (remove.empty()) {
		return;
	}
	log("ScriptingSaver::delete_unused(): Deleting %" PRIuS " item(s):\n", remove.size());
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
		if (!found) {
			throw wexception(
			   "ScriptingSaver::delete_unused(): Attempt to delete nonexistent object %u",
			   remove.back());
		}
	}
}

ScriptingLoader::ScriptingLoader(FileRead& fr, ScriptingSaver& s) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version != kCurrentPacketVersionScripting) {
			throw Widelands::UnhandledVersionError(
			   "ScriptingLoader", packet_version, kCurrentPacketVersionScripting);
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
