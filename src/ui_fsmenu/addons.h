/*
 * Copyright (C) 2020-2020 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_H
#define WL_UI_FSMENU_ADDONS_H

#include <memory>
#include <set>
#include <vector>

#include "logic/addons.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/base.h"

class AddOnsCtrl;

struct InstalledAddOnRow : public UI::Panel {
	InstalledAddOnRow(Panel*, AddOnsCtrl*, const AddOnInfo&, bool enabled, bool is_first, bool is_last);
	~InstalledAddOnRow() override {
	}
	void layout() override;
private:
	UI::Button move_up_;
	UI::Button move_down_;
	UI::Button uninstall_;
	std::unique_ptr<UI::Button> toggle_enabled_;
	UI::Icon category_;
	UI::Textarea version_;
	UI::MultilineTextarea txt_;
};
struct RemoteAddOnRow : public UI::Panel {
	RemoteAddOnRow(Panel*, AddOnsCtrl*, const AddOnInfo&, uint32_t installed_version);
	~RemoteAddOnRow() override {
	}
	void layout() override;
private:
	UI::Button install_;
	UI::Button upgrade_;
	UI::Button uninstall_;
	UI::Icon category_;
	UI::Icon verified_;
	UI::Textarea version_;
	UI::MultilineTextarea txt_;
};

class AddOnsCtrl : public FullscreenMenuBase {
public:
	AddOnsCtrl();
	~AddOnsCtrl() override;

	void rebuild();

protected:
	void layout() override;

private:
	UI::Textarea title_;
	UI::TabPanel tabs_;
	UI::Box installed_addons_wrapper_, browse_addons_wrapper_, installed_addons_box_, browse_addons_box_;
	std::vector<InstalledAddOnRow*> installed_;
	std::vector<RemoteAddOnRow*> browse_;
	UI::Button ok_, refresh_;

	std::vector<AddOnInfo> remotes_;
	void refresh_remotes();

};

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_H
