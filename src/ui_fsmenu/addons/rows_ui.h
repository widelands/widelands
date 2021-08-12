/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_ROWS_UI_H
#define WL_UI_FSMENU_ADDONS_ROWS_UI_H

#include <memory>

#include "logic/addons.h"
#include "ui_basic/button.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

namespace FsMenu {
namespace AddOnsUI {

class AddOnsCtrl;

class InstalledAddOnRow : public UI::Panel {
public:
	InstalledAddOnRow(Panel*, AddOnsCtrl*, std::shared_ptr<AddOns::AddOnInfo>, bool enabled);
	~InstalledAddOnRow() override {
	}
	const std::shared_ptr<AddOns::AddOnInfo> info() const {
		return info_;
	}
	void layout() override;
	void draw(RenderTarget&) override;

private:
	std::shared_ptr<AddOns::AddOnInfo> info_;
	bool enabled_;
	UI::Button uninstall_, toggle_enabled_;
	UI::Icon icon_, category_;
	UI::Textarea version_;
	UI::MultilineTextarea txt_;
};

class RemoteAddOnRow : public UI::Panel {
public:
	RemoteAddOnRow(Panel*,
	               AddOnsCtrl*,
	               const std::shared_ptr<AddOns::AddOnInfo>,
	               const AddOns::AddOnVersion& installed_version,
	               uint32_t installed_i18n_version);
	~RemoteAddOnRow() override {
	}
	void layout() override;
	void draw(RenderTarget&) override;
	const std::shared_ptr<AddOns::AddOnInfo> info() const {
		return info_;
	}
	bool upgradeable() const;
	bool full_upgrade_possible() const {
		return full_upgrade_possible_;
	}

private:
	std::shared_ptr<AddOns::AddOnInfo> info_;
	UI::Button install_, upgrade_, uninstall_, interact_;
	UI::Icon icon_, category_, verified_;
	UI::Textarea version_, bottom_row_left_, bottom_row_right_;
	UI::MultilineTextarea txt_;

	const bool full_upgrade_possible_;
};

}  // namespace AddOnsUI
}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_ROWS_UI_H
