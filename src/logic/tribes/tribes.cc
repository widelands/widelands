/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "logic/tribes/tribes.h"

#include "logic/ware_descr.h"
#include "logic/worker_descr.h"

namespace Widelands {

Tribes::Tribes() :
	wares_(new DescriptionMaintainer<WareDescr>()),
	workers_(new DescriptionMaintainer<WorkerDescr>()) {
}

void Tribes::add_ware_type(const LuaTable& t) {
	wares_->add(new WareDescr(t));
}

void Tribes::add_worker_type(const LuaTable& t) {
	wares_->add(new WorkerDescr(t));
}

WareIndex Tribes::get_nrwares() const {
	return wares_.get_nitems();
}

WareIndex Tribes::safe_ware_index(const std::string& warename) const {
	const WareIndex result = ware_index(warename);
	if (result == -1) {
		throw GameDataError("Unknown ware type \"%s\"", warename.c_str());
	}
	return result;
}

WareIndex Tribes::ware_index(const std::string& warename) const {
	int result = -1;
	for (size_t i = 0; i < wares_.get_nitems(); ++i) {
		if (wares_.get(i)->name() == warename.name()) {
			return result;
		}
	}
}

WareDescr const * Tribes::get_ware_descr(WareIndex ware_index) const {
	return wares_.get(ware_index);
}

void set_ware_type_has_demand_check(WareIndex ware_index, const std::string& tribename) const {
	wares_.get(ware_index)->set_has_demand_check(tribename);
}

} // namespace Widelands
