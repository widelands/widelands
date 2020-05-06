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

#include "ui_fsmenu/addons.h"

#include <memory>

#include "base/i18n.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"

constexpr int16_t kRowButtonSize = 32;
constexpr int16_t kRowButtonSpacing = 4;

AddOnsCtrl::AddOnsCtrl() : FullscreenMenuBase(),
		title_(this, 0, 0, get_w(), get_h() / 12, _("Add-Ons"), UI::Align::kCenter, g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
		tabs_(this, UI::TabPanelStyle::kFsMenu),
		installed_addons_box_(&tabs_, 0, 0, UI::Box::Vertical),
		browse_addons_box_(&tabs_, 0, 0, UI::Box::Vertical),
		ok_(this, "ok", 0, 0, get_w() / 2, get_h() / 12, UI::ButtonStyle::kFsMenuPrimary, _("OK")),
		refresh_(this, "refresh", 0, 0, kRowButtonSize, kRowButtonSize, UI::ButtonStyle::kFsMenuSecondary,
				_("↺"), _("Refresh the list of add-ons available from the server")) {
	installed_addons_box_.set_scrolling(true);
	browse_addons_box_.set_scrolling(true);
	tabs_.add("my", _("Installed"), &installed_addons_box_);
	tabs_.add("all", _("Browse"), &browse_addons_box_);
	ok_.sigclicked.connect([this]() {
		clicked_ok();
	});
	refresh_.sigclicked.connect([this]() {
		refresh_remotes();
	});
	refresh_remotes();
}

AddOnsCtrl::~AddOnsCtrl() {
	std::string text;
	for (const auto& pair : g_addons) {
		if (!text.empty()) {
			text += ',';
		}
		text += pair.first.internal_name + ':' + (pair.second ? "true" : "false");
	}
	set_config_string("addons", text);
	write_config();
}

void AddOnsCtrl::refresh_remotes() {
	remotes_.clear();

	// TODO(Nordfriese): Connect to the add-on server when we have one
	// and fetch a list of all available add-ons
	remotes_ = {
		AddOnInfo {
			"dummy.wad",
			_("Not yet implemented"),
			_("Sorry, the remote add-on repo is not yet implemented."),
			_("The Widelands Development Team"),
			1, &kAddOnCategories.at("maps"), {}, true
		}
	};

	rebuild();
}

void AddOnsCtrl::rebuild() {
	installed_addons_box_.free_children();
	browse_addons_box_.free_children();
	installed_addons_box_.clear();
	browse_addons_box_.clear();
	installed_.clear();
	browse_.clear();
	assert(installed_addons_box_.get_nritems() == 0);
	assert(browse_addons_box_.get_nritems() == 0);

	const size_t nr_installed = g_addons.size();
	size_t index = 0;
	for (const auto& pair : g_addons) {
		InstalledAddOnRow* i = new InstalledAddOnRow(&installed_addons_box_, this, pair.first, pair.second, index == 0, index + 1 == nr_installed);
		installed_addons_box_.add(i, UI::Box::Resizing::kFullSize);
		installed_addons_box_.add_space(kRowButtonSize);
		++index;
	}
	for (const AddOnInfo& a : remotes_) {
		uint32_t installed = kNotInstalled;
		for (const auto& pair : g_addons) {
			if (pair.first.internal_name == a.internal_name) {
				installed = pair.first.version;
				break;
			}
		}
		RemoteAddOnRow* r = new RemoteAddOnRow(&browse_addons_box_, this, a, installed);
		browse_addons_box_.add(r, UI::Box::Resizing::kFullSize);
		browse_addons_box_.add_space(kRowButtonSize);
	}

	layout();
}

void AddOnsCtrl::layout() {
	FullscreenMenuBase::layout();
	title_.set_size(get_w(), get_h() / 16);
	title_.set_pos(Vector2i(0, get_h() / 16));
	ok_.set_size(get_w() / 2, get_h() / 16);
	ok_.set_pos(Vector2i(get_w() / 4, get_h() * 14 / 16));
	tabs_.set_size(get_w() * 2 / 3, get_h() * 2 / 3);
	tabs_.set_pos(Vector2i(get_w() / 6, get_h() / 6));
	refresh_.set_pos(Vector2i(tabs_.get_x() + tabs_.get_w(), tabs_.get_y() - refresh_.get_h()));
}

static void uninstall(AddOnsCtrl* ctrl, const AddOnInfo& info) {
	if (!(SDL_GetModState() & KMOD_CTRL)) {
		UI::WLMessageBox w(ctrl, _("Uninstall"), (boost::format(_("Are you certain that you want to uninstall this add-on?\n\n"
			"%1$s\n"
			"by %2$s\n"
			"Version %3$u\n"
			"Category: %4$s\n"
			"%5$s\n"
			))
			% info.descname
			% info.author
			% info.version
			% info.category->descname()
			% info.description
			).str(), UI::WLMessageBox::MBoxType::kOkCancel);
		if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) { return; }
	}
	g_fs->fs_unlink(kAddOnDir + g_fs->file_separator() + info.internal_name);
	for (auto it = g_addons.begin(); it != g_addons.end(); ++it) {
		if (it->first.internal_name == info.internal_name) {
			g_addons.erase(it);
			return ctrl->rebuild();
		}
	}
	NEVER_HERE();
}

InstalledAddOnRow::InstalledAddOnRow(Panel* parent, AddOnsCtrl* ctrl, const AddOnInfo& info, bool enabled, bool is_first, bool is_last)
	: UI::Panel(parent, 0, 0, 3 * kRowButtonSize, 3 * kRowButtonSize),
	move_up_(this, "up", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/ui_basic/scrollbar_up.png"), _("Move up")),
	move_down_(this, "down", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/ui_basic/scrollbar_down.png"), _("Move down")),
	uninstall_(this, "uninstall", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/wui/menus/exit.png"), _("Uninstall")),
	toggle_enabled_(info.category->can_disable_addons ? new UI::Button(this, "on-off", 0, 0, 24, 24,
			UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get(
					enabled ? "images/ui_basic/checkbox_checked.png" : "images/ui_basic/checkbox_empty.png"),
					enabled ? _("Disable") : _("Enable")) : nullptr),
	category_(this, g_gr->images().get(info.category->icon)),
	version_(this, 0, 0, 0, 0, std::to_string(static_cast<int>(info.version)), UI::Align::kCenter, g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
	txt_(this, 0, 0, 24, 24, UI::PanelStyle::kFsMenu, (boost::format("<rt>%s<p>%s</p><p>%s</p></rt>")
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelHeading).as_font_tag(info.descname)
		% g_gr->styles().font_style(UI::FontStyle::kChatWhisper).as_font_tag((boost::format(_("by %s")) % info.author).str())
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(info.description)).str()) {

	uninstall_.sigclicked.connect([this, ctrl, info]() {
		uninstall(ctrl, info);
	});
	move_up_.sigclicked.connect([this, ctrl, info]() {
		auto it = g_addons.begin();
		for (; it->first.internal_name != info.internal_name; ++it);
		const bool state = it->second;
		it = g_addons.erase(it);
		--it;
		g_addons.insert(it, std::make_pair(info, state));
		ctrl->rebuild();
	});
	move_down_.sigclicked.connect([this, ctrl, info]() {
		auto it = g_addons.begin();
		for (; it->first.internal_name != info.internal_name; ++it);
		const bool state = it->second;
		it = g_addons.erase(it);
		++it;
		g_addons.insert(it, std::make_pair(info, state));
		ctrl->rebuild();
	});
	if (toggle_enabled_) {
		toggle_enabled_->sigclicked.connect([this, info]() {
			for (auto& pair : g_addons) {
				if (pair.first.internal_name == info.internal_name) {
					pair.second = !pair.second;
					toggle_enabled_->set_pic(g_gr->images().get(
							pair.second ? "images/ui_basic/checkbox_checked.png" : "images/ui_basic/checkbox_empty.png"));
					toggle_enabled_->set_tooltip(pair.second ? _("Disable") : _("Enable"));
					return;
				}
			}
			NEVER_HERE();
		});
	}
	move_up_.set_enabled(!is_first);
	move_down_.set_enabled(!is_last);
	category_.set_handle_mouse(true);
	category_.set_tooltip((boost::format(_("Category: %s")) % info.category->descname()).str());
	version_.set_handle_mouse(true);
	version_.set_tooltip(_("Version"));
	layout();
}

void InstalledAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 2 * kRowButtonSize + 2 * kRowButtonSpacing) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 3 * kRowButtonSize);
	for (UI::Panel* p : std::vector<UI::Panel*>{&move_up_, &move_down_, &uninstall_, &category_, &version_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	if (toggle_enabled_) {
		toggle_enabled_->set_size(kRowButtonSize, kRowButtonSize);
		toggle_enabled_->set_pos(Vector2i(get_w() - kRowButtonSize, kRowButtonSize));
	}
	move_up_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	move_down_.set_pos(Vector2i(get_w() - kRowButtonSize, 2 * kRowButtonSize));
	category_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 0));
	version_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, kRowButtonSize));
	uninstall_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 2 * kRowButtonSize));
	txt_.set_size(get_w() - 2 * kRowButtonSize - 2 * kRowButtonSpacing, 3 * kRowButtonSize);
	txt_.set_pos(Vector2i(0, 0));
}

RemoteAddOnRow::RemoteAddOnRow(Panel* parent, AddOnsCtrl* ctrl, const AddOnInfo& info, uint32_t installed)
	: UI::Panel(parent, 0, 0, 3 * kRowButtonSize, 3 * kRowButtonSize),
	install_(this, "install", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/ui_basic/continue.png"), _("Install")),
	upgrade_(this, "upgrade", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/ui_basic/different.png"), _("Upgrade")),
	uninstall_(this, "uninstall", 0, 0, 24, 24, UI::ButtonStyle::kFsMenuSecondary, g_gr->images().get("images/wui/menus/exit.png"), _("Uninstall")),
	category_(this, g_gr->images().get(info.category->icon)),
	verified_(this, g_gr->images().get(info.verified ? "images/ui_basic/list_selected.png" : "images/ui_basic/stop.png")),
	version_(this, 0, 0, 0, 0, std::to_string(static_cast<int>(info.version)), UI::Align::kCenter, g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
	txt_(this, 0, 0, 24, 24, UI::PanelStyle::kFsMenu, (boost::format("<rt>%s<p>%s</p><p>%s</p></rt>")
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelHeading).as_font_tag(info.descname)
		% g_gr->styles().font_style(UI::FontStyle::kChatWhisper).as_font_tag((boost::format(_("by %s")) % info.author).str())
		% g_gr->styles().font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(info.description)).str()) {

	uninstall_.sigclicked.connect([this, ctrl, info]() {
		uninstall(ctrl, info);
	});
	install_.sigclicked.connect([this, ctrl, info]() {
		// Ctrl-click skips the confirmation. Never skip for non-verified stuff though.
		if (!info.verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(ctrl, _("Install"), (boost::format(_("Are you certain that you want to install this add-on?\n\n"
				"%1$s\n"
				"by %2$s\n"
				"%3$s\n"
				"Version %4$u\n"
				"Category: %5$s\n"
				"%6$s\n"
				))
				% info.descname
				% info.author
				% (info.verified ? _("Verified") : _("NOT VERIFIED"))
				% info.version
				% info.category->descname()
				% info.description
				).str(), UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) { return; }
		}
		// TODO(Nordfriese): Download this add-on from the server and save
		// the fetched ZIP-file as ~/.widelands/addons/my_new_addon.wad
		log("NOCOM: Installing not yet implemented\n");
	});
	upgrade_.sigclicked.connect([this, ctrl, info, installed]() {
		if (!info.verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(ctrl, _("Upgrade"), (boost::format(_("Are you certain that you want to upgrade this add-on?\n\n"
				"%1$s\n"
				"by %2$s\n"
				"%3$s\n"
				"Installed version: %4$u\n"
				"Available version: %5$u\n"
				"Category: %6$s\n"
				"%7$s\n"
				))
				% info.descname
				% info.author
				% (info.verified ? _("Verified") : _("NOT VERIFIED"))
				% installed
				% info.version
				% info.category->descname()
				% info.description
				).str(), UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) { return; }
		}
		// TODO(Nordfriese): Download this add-on from the server, then replace
		// ~/.widelands/addons/my_new_addon.wad with the fetched ZIP-file
		log("NOCOM: Upgrading not yet implemented\n");
	});
	if (installed == kNotInstalled) {
		uninstall_.set_enabled(false);
		upgrade_.set_enabled(false);
	} else {
		install_.set_enabled(false);
		upgrade_.set_enabled(installed < info.version);
	}
	category_.set_handle_mouse(true);
	category_.set_tooltip((boost::format(_("Category: %s")) % info.category->descname()).str());
	version_.set_handle_mouse(true);
	version_.set_tooltip(_("Version"));
	verified_.set_handle_mouse(true);
	verified_.set_tooltip(info.verified ? _("Verified by the Widelands Development Team") :
		_("This add-on was not checked by the Widelands Development Team yet. We cannot guarantee that it does not contain harmful or offensive content."));
	layout();
}

void RemoteAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 2 * kRowButtonSize + 2 * kRowButtonSpacing) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 3 * kRowButtonSize);
	for (UI::Panel* p : std::vector<UI::Panel*>{&install_, &uninstall_, &upgrade_, &category_, &version_, &verified_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	install_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	upgrade_.set_pos(Vector2i(get_w() - kRowButtonSize, kRowButtonSize));
	uninstall_.set_pos(Vector2i(get_w() - kRowButtonSize, 2 * kRowButtonSize));
	category_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, 0));
	version_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, kRowButtonSize));
	verified_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, 2 * kRowButtonSize));
	txt_.set_size(get_w() - 2 * kRowButtonSize - 2 * kRowButtonSpacing, 3 * kRowButtonSize);
	txt_.set_pos(Vector2i(0, 0));
}
