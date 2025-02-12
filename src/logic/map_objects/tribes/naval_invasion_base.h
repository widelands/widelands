/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_NAVAL_INVASION_BASE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_NAVAL_INVASION_BASE_H

#include "logic/map_objects/tribes/soldier.h"

namespace Widelands {

class NavalInvasionBaseDescr : public BobDescr {
public:
	NavalInvasionBaseDescr(char const* const init_name, char const* const init_descname)
	   : BobDescr(init_name,
	              init_descname,
	              MapObjectType::NAVAL_INVASION_BASE,
	              MapObjectDescr::OwnerType::kTribe) {
	}
	~NavalInvasionBaseDescr() override = default;
	[[nodiscard]] Bob& create_object() const override;

private:
	DISALLOW_COPY_AND_ASSIGN(NavalInvasionBaseDescr);
};

class NavalInvasionBase : public Bob {
public:
	NavalInvasionBase();
	static NavalInvasionBase* create(EditorGameBase& egbase, Soldier& soldier, const Coords& pos);

	const NavalInvasionBaseDescr& descr() const;
	void init_auto_task(Game& game) override;
	void cleanup(EditorGameBase&) override;
	void log_general_info(const EditorGameBase&) const override;

	void add_soldier(EditorGameBase& egbase, Soldier* soldier);

	[[nodiscard]] const std::set<OPtr<Soldier>>& get_soldiers() const {
		return soldiers_;
	}

	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	static Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);

private:
	std::set<OPtr<Soldier>> soldiers_;

	void check_unconquer();

protected:
	struct Loader : Bob::Loader {
		Loader() = default;

		void load(FileRead& fr);
		void load_pointers() override;

	private:
		std::set<Serial> soldiers_;
	};
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_NAVAL_INVASION_BASE_H
