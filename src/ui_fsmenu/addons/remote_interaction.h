/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_REMOTE_INTERACTION_H
#define WL_UI_FSMENU_ADDONS_REMOTE_INTERACTION_H

#include <memory>

#include "logic/addons.h"
#include "network/net_addons.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

namespace FsMenu {
namespace AddOnsUI {

class AddOnsCtrl;
class RemoteInteractionWindow;

class CommentRow : public UI::Box {
public:
	CommentRow(AddOnsCtrl& ctrl,
	           std::shared_ptr<AddOns::AddOnInfo> info,
	           RemoteInteractionWindow& r,
	           UI::Panel& parent,
	           const std::string& text,
	           const size_t& index);

	void update_edit_enabled();
	void layout() override;

private:
	AddOnsCtrl& ctrl_;
	std::shared_ptr<AddOns::AddOnInfo> info_;
	const size_t& index_;
	bool layouting_;
	UI::MultilineTextarea text_;
	UI::Box buttons_;
	UI::Button edit_, delete_;
};

class CommentEditor : public UI::Window {
public:
	CommentEditor(AddOnsCtrl& ctrl, std::shared_ptr<AddOns::AddOnInfo> info, const size_t* index);

	void think() override;

private:
	std::shared_ptr<AddOns::AddOnInfo> info_;
	const size_t* index_;

	UI::Box main_box_, markup_box_, buttons_box_;
	UI::MultilineTextarea preview_;
	UI::MultilineEditbox* text_;
	UI::Button ok_, reset_, cancel_;

	void apply_format(const std::string& open_tag, const std::string& close_tag);

	void reset_text();
};

class TransifexSettingsBox;

class AdminDialog : public UI::Window {
public:
	AdminDialog(AddOnsCtrl&,
	            RemoteInteractionWindow&,
	            std::shared_ptr<AddOns::AddOnInfo>,
	            AddOns::NetAddons::AdminAction);
	void think() override;

private:
	UI::Box main_box_, buttons_box_;
	UI::Button ok_, cancel_;
	UI::Listselect<std::string>* list_;
	UI::MultilineEditbox* text_;
	TransifexSettingsBox* txsettings_;
};

class RemoteInteractionWindow : public UI::Window {
public:
	RemoteInteractionWindow(AddOnsCtrl& parent, std::shared_ptr<AddOns::AddOnInfo> info);

	void on_resolution_changed_note(const GraphicResolutionChanged& note) override;
	void layout() override;
	void update_data();
	void die() override {
		UI::Window::die();
	}

private:
	static std::map<std::pair<std::string /* add-on */, std::string /* screenshot */>,
	                std::string /* image path */>
	   downloaded_screenshots_cache_;

	void next_screenshot(int8_t delta);

	void login_changed();

	AddOnsCtrl& parent_;
	std::shared_ptr<AddOns::AddOnInfo> info_;
	int32_t current_screenshot_, nr_screenshots_;
	std::vector<const Image*> screenshot_cache_;

	/** How the user voted the current add-on (1-10; 0 for not voted; -1 for unknown). */
	int current_vote_;

	void update_current_vote_on_demand();

	UI::Box main_box_;
	UI::TabPanel tabs_;
	UI::Box box_comments_, box_comment_rows_, box_screenies_, box_screenies_buttons_, box_votes_,
	   voting_stats_;
	UI::Panel box_comment_rows_placeholder_;

	UI::MultilineTextarea comments_header_;
	std::list<std::unique_ptr<CommentRow>> comment_rows_;
	UI::Icon screenshot_;

	UI::Dropdown<uint8_t> own_voting_;
	UI::ProgressBar* voting_bars_[AddOns::kMaxRating];
	UI::Textarea* voting_txt_[AddOns::kMaxRating];
	UI::Textarea screenshot_stats_, screenshot_descr_, voting_stats_summary_;
	UI::Button screenshot_next_, screenshot_prev_, write_comment_, ok_, login_button_;
	UI::Dropdown<AddOns::NetAddons::AdminAction> admin_action_;
};

}  // namespace AddOnsUI
}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_REMOTE_INTERACTION_H
