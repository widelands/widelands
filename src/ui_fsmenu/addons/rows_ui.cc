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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_fsmenu/addons/rows_ui.h"

#include <memory>

#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/addons/manager.h"
#include "ui_fsmenu/addons/remote_interaction.h"

namespace FsMenu {
namespace AddOnsUI {

namespace {

inline std::string safe_richtext_message(std::string body) {
	newlines_to_richtext(body);
	return as_richtext_paragraph(body, UI::FontStyle::kFsMenuLabel, UI::Align::kCenter);
}

void uninstall(AddOnsCtrl* ctrl, std::shared_ptr<AddOns::AddOnInfo> info, const bool local) {
	if (!(SDL_GetModState() & KMOD_CTRL)) {
		UI::WLMessageBox w(
		   &ctrl->get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Uninstall"),
		   safe_richtext_message(
		      format(local ? _("Are you certain that you want to uninstall this add-on?\n\n"
		                       "%1$s\n"
		                       "by %2$s\n"
		                       "Version %3$s\n"
		                       "Category: %4$s\n"
		                       "%5$s\n\n"
		                       "Note that this add-on can not be downloaded again from the server.") :
                           _("Are you certain that you want to uninstall this add-on?\n\n"
		                       "%1$s\n"
		                       "by %2$s\n"
		                       "Version %3$s\n"
		                       "Category: %4$s\n"
		                       "%5$s"),
		             info->descname(), info->author(), AddOns::version_to_string(info->version),
		             AddOns::kAddOnCategories.at(info->category).descname(), info->description())),
		   UI::WLMessageBox::MBoxType::kOkCancel);
		if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
	}

	// Delete the add-on…
	g_fs->fs_unlink(kAddOnDir + FileSystem::file_separator() + info->internal_name);

	// …and its translations
	for (const std::string& locale : g_fs->list_directory(kAddOnLocaleDir)) {
		g_fs->fs_unlink(locale + FileSystem::file_separator() + "LC_MESSAGES" +
		                FileSystem::file_separator() + info->internal_name + ".mo");
	}

	for (auto it = AddOns::g_addons.begin(); it != AddOns::g_addons.end(); ++it) {
		if (it->first->internal_name == info->internal_name) {
			AddOns::g_addons.erase(it);
			if (info->category == AddOns::AddOnCategory::kTheme &&
			    template_dir() == AddOns::theme_addon_template_dir(info->internal_name)) {
				AddOns::update_ui_theme(AddOns::UpdateThemeAction::kAutodetect);
				ctrl->get_topmost_forefather().template_directory_changed();
			}
			return ctrl->rebuild(true);
		}
	}
	NEVER_HERE();
}

std::string required_wl_version_and_sync_safety_string(std::shared_ptr<AddOns::AddOnInfo> info) {
	std::string result;
	if (!info->sync_safe) {
		result += "<br>";
		result += g_style_manager->font_style(UI::FontStyle::kWarning)
		             .as_font_tag(
		                _("This add-on is known to cause desyncs in multiplayer games and replays."));
	}
	if (!info->min_wl_version.empty() || !info->max_wl_version.empty()) {
		result += "<br>";
		std::string str;
		if (info->max_wl_version.empty()) {
			str += format(_("Requires a Widelands version of at least %s."), info->min_wl_version);
		} else if (info->min_wl_version.empty()) {
			str += format(_("Requires a Widelands version of at most %s."), info->max_wl_version);
		} else {
			str += format(_("Requires a Widelands version of at least %1$s and at most %2$s."),
			              info->min_wl_version, info->max_wl_version);
		}
		result += g_style_manager
		             ->font_style(info->matches_widelands_version() ? UI::FontStyle::kItalic :
                                                                    UI::FontStyle::kWarning)
		             .as_font_tag(str);
	}
	return result;
}

}  // namespace

InstalledAddOnRow::InstalledAddOnRow(Panel* parent,
                                     AddOnsCtrl* ctrl,
                                     std::shared_ptr<AddOns::AddOnInfo> info,
                                     bool enabled)
   : UI::Panel(parent,
               UI::PanelStyle::kFsMenu,
               0,
               0,
               3 * kRowButtonSize,
               2 * kRowButtonSize + 3 * kRowButtonSpacing),
     info_(info),
     enabled_(enabled),
     uninstall_(this,
                "uninstall",
                0,
                0,
                24,
                24,
                UI::ButtonStyle::kFsMenuSecondary,
                g_image_cache->get("images/wui/menus/exit.png"),
                _("Uninstall")),
     toggle_enabled_(this,
                     "on-off",
                     0,
                     0,
                     24,
                     24,
                     UI::ButtonStyle::kFsMenuSecondary,
                     g_image_cache->get(enabled ? "images/ui_basic/checkbox_checked.png" :
                                                  "images/ui_basic/checkbox_empty.png"),
                     enabled ? _("Disable") : _("Enable"),
                     UI::Button::VisualState::kFlat),
     icon_(this, UI::PanelStyle::kFsMenu, info_->icon),
     category_(this,
               UI::PanelStyle::kFsMenu,
               g_image_cache->get(AddOns::kAddOnCategories.at(info->category).icon)),
     version_(this,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsMenuInfoPanelHeading,
              0,
              0,
              0,
              0,
              /** TRANSLATORS: (MajorVersion)+(MinorVersion) */
              format(_("%1$s+%2$u"), AddOns::version_to_string(info->version), info->i18n_version),
              UI::Align::kCenter),
     txt_(
        this,
        0,
        0,
        24,
        24,
        UI::PanelStyle::kFsMenu,
        format(
           "<rt><p>%s</p><p>%s%s</p><p>%s</p></rt>",
           format(
              /** TRANSLATORS: Add-On localized name as header (Add-On internal name in italics) */
              _("%1$s %2$s"),
              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
                 .as_font_tag(info->descname()),
              g_style_manager->font_style(UI::FontStyle::kItalic)
                 .as_font_tag(format(_("(%s)"), info->internal_name))),
           g_style_manager->font_style(UI::FontStyle::kItalic)
              .as_font_tag(format(_("by %s"), info->author())),
           required_wl_version_and_sync_safety_string(info),
           g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
              .as_font_tag(info->description()))) {

	uninstall_.sigclicked.connect(
	   [ctrl, this]() { uninstall(ctrl, info_, !ctrl->is_remote(info_->internal_name)); });
	toggle_enabled_.sigclicked.connect([this, ctrl, info]() {
		enabled_ = !enabled_;
		for (auto& pair : AddOns::g_addons) {
			if (pair.first->internal_name == info->internal_name) {
				pair.second = !pair.second;
				toggle_enabled_.set_pic(g_image_cache->get(pair.second ?
                                                          "images/ui_basic/checkbox_checked.png" :
                                                          "images/ui_basic/checkbox_empty.png"));
				toggle_enabled_.set_tooltip(pair.second ? _("Disable") : _("Enable"));
				if (pair.first->category == AddOns::AddOnCategory::kTheme) {
					AddOns::update_ui_theme(pair.second ? AddOns::UpdateThemeAction::kEnableArgument :
                                                     AddOns::UpdateThemeAction::kAutodetect,
					                        pair.first->internal_name);
					get_topmost_forefather().template_directory_changed();
				}
				return ctrl->rebuild(true);
			}
		}
		NEVER_HERE();
	});
	category_.set_handle_mouse(true);
	category_.set_tooltip(
	   format(_("Category: %s"), AddOns::kAddOnCategories.at(info->category).descname()));
	version_.set_handle_mouse(true);
	version_.set_tooltip(format(
	   /** TRANSLATORS: (MajorVersion)+(MinorVersion) */
	   _("Version: %1$s+%2$u"), AddOns::version_to_string(info->version), info->i18n_version));
	set_can_focus(true);
	layout();
}

void InstalledAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 3 * kRowButtonSize) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 2 * kRowButtonSize + 3 * kRowButtonSpacing);

	const int icon_size = 2 * kRowButtonSize + kRowButtonSpacing;
	icon_.set_size(icon_size, icon_size);
	icon_.set_pos(Vector2i(0, kRowButtonSpacing));
	uninstall_.set_size(kRowButtonSize, kRowButtonSize);
	category_.set_size(kRowButtonSize, kRowButtonSize);
	version_.set_size(3 * kRowButtonSize + 2 * kRowButtonSpacing, kRowButtonSize);
	toggle_enabled_.set_size(kRowButtonSize, kRowButtonSize);
	toggle_enabled_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 0));
	category_.set_pos(Vector2i(get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing, 0));
	uninstall_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	version_.set_pos(Vector2i(get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing,
	                          kRowButtonSize + 3 * kRowButtonSpacing));
	txt_.set_size(get_w() - 3 * (kRowButtonSize + kRowButtonSpacing) - icon_size,
	              2 * kRowButtonSize + 3 * kRowButtonSpacing);
	txt_.set_pos(Vector2i(icon_size, 0));
}

void InstalledAddOnRow::draw(RenderTarget& r) {
	UI::Panel::draw(r);
	r.brighten_rect(Recti(0, 0, get_w(), get_h()), has_focus() ? enabled_ ? -40 : -30 :
	                                               enabled_    ? -20 :
                                                                0);
}

void RemoteAddOnRow::draw(RenderTarget& r) {
	UI::Panel::draw(r);
	r.brighten_rect(Recti(0, 0, get_w(), get_h()), -20);
}

RemoteAddOnRow::RemoteAddOnRow(Panel* parent,
                               AddOnsCtrl* ctrl,
                               std::shared_ptr<AddOns::AddOnInfo> info,
                               const AddOns::AddOnVersion& installed_version,
                               uint32_t installed_i18n_version)
   : UI::Panel(parent, UI::PanelStyle::kFsMenu, 0, 0, 3 * kRowButtonSize, 4 * kRowButtonSize),
     info_(info),
     install_(this,
              "install",
              0,
              0,
              24,
              24,
              UI::ButtonStyle::kFsMenuSecondary,
              g_image_cache->get("images/ui_basic/continue.png"),
              _("Install")),
     upgrade_(this,
              "upgrade",
              0,
              0,
              24,
              24,
              UI::ButtonStyle::kFsMenuSecondary,
              g_image_cache->get("images/wui/buildings/menu_up_train.png"),
              _("Upgrade")),
     uninstall_(this,
                "uninstall",
                0,
                0,
                24,
                24,
                UI::ButtonStyle::kFsMenuSecondary,
                g_image_cache->get("images/wui/menus/exit.png"),
                _("Uninstall")),
     interact_(this,
               "interact",
               0,
               0,
               24,
               24,
               UI::ButtonStyle::kFsMenuSecondary,
               "…",
               _("Comments and Votes")),
     icon_(this, UI::PanelStyle::kFsMenu, info_->icon),
     category_(this,
               UI::PanelStyle::kFsMenu,
               g_image_cache->get(AddOns::kAddOnCategories.at(info->category).icon)),
     verified_(this,
               UI::PanelStyle::kFsMenu,
               g_image_cache->get(info->verified ? "images/ui_basic/list_selected.png" :
                                                   "images/ui_basic/stop.png")),
     quality_(this, UI::PanelStyle::kFsMenu, AddOnQuality::kQualities.at(info->quality)().icon),
     version_(this,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsMenuInfoPanelHeading,
              0,
              0,
              0,
              0,
              /** TRANSLATORS: (MajorVersion)+(MinorVersion) */
              format(_("%1$s+%2$u"), AddOns::version_to_string(info->version), info->i18n_version),
              UI::Align::kCenter),
     bottom_row_left_(this,
                      UI::PanelStyle::kFsMenu,
                      UI::FontStyle::kFsTooltip,
                      0,
                      0,
                      0,
                      0,
                      time_string(info->upload_timestamp),
                      UI::Align::kLeft),
     bottom_row_right_(
        this,
        UI::PanelStyle::kFsMenu,
        UI::FontStyle::kFsTooltip,
        0,
        0,
        0,
        0,
        info->internal_name.empty() ?
           "" :
           format(
              /** TRANSLATORS: Filesize · Download count · Average rating · Number of comments ·
                 Number of screenshots */
              _("%1$s   ⬇ %2$u   ★ %3$s   “” %4$u   ▣ %5$u"),
              filesize_string(info->total_file_size),
              info->download_count,
              (info->number_of_votes() ? format_l("%.2f", info->average_rating()) : "–"),
              info->user_comments.size(),
              info->screenshots.size()),
        UI::Align::kRight),
     txt_(
        this,
        0,
        0,
        24,
        24,
        UI::PanelStyle::kFsMenu,
        format(
           "<rt><p>%s</p><p>%s%s</p><p>%s</p></rt>",
           format(
              /** TRANSLATORS: Add-On localized name as header (Add-On internal name in italics) */
              _("%1$s %2$s"),
              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
                 .as_font_tag(info->descname()),
              g_style_manager->font_style(UI::FontStyle::kItalic)
                 .as_font_tag(format(_("(%s)"), info->internal_name))),
           g_style_manager->font_style(UI::FontStyle::kItalic)
              .as_font_tag(
                 info->author() == info->upload_username ?
                    format(_("by %s"), info->author()) :
                    format(_("by %1$s (uploaded by %2$s)"), info->author(), info->upload_username)),
           required_wl_version_and_sync_safety_string(info),
           g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
              .as_font_tag(info->description()))),
     full_upgrade_possible_(AddOns::is_newer_version(installed_version, info->version)) {

	interact_.sigclicked.connect([ctrl, info]() {
		RemoteInteractionWindow m(*ctrl, info);
		m.run<UI::Panel::Returncodes>();
	});
	uninstall_.sigclicked.connect([ctrl, this]() { uninstall(ctrl, info_, false); });
	install_.sigclicked.connect([ctrl, this]() {
		// Ctrl-click skips the confirmation. Never skip for non-verified stuff though.
		if (!info_->verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(
			   &ctrl->get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Install"),
			   safe_richtext_message(format(
			      _("Are you certain that you want to install this add-on?\n\n"
			        "%1$s\n"
			        "by %2$s\n"
			        "%3$s\n"
			        "Version %4$s\n"
			        "Category: %5$s\n"
			        "%6$s\n"),
			      info_->descname(), info_->author(),
			      (info_->verified ? _("Verified") : _("NOT VERIFIED")),
			      AddOns::version_to_string(info_->version),
			      AddOns::kAddOnCategories.at(info_->category).descname(), info_->description())),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		ctrl->install_or_upgrade(info_, false);
		ctrl->rebuild(true);
	});
	upgrade_.sigclicked.connect([this, ctrl, info, installed_version]() {
		if (!info->verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(
			   &ctrl->get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Upgrade"),
			   safe_richtext_message(
			      format(_("Are you certain that you want to upgrade this add-on?\n\n"
			               "%1$s\n"
			               "by %2$s\n"
			               "%3$s\n"
			               "Installed version: %4$s\n"
			               "Available version: %5$s\n"
			               "Category: %6$s\n"
			               "%7$s"),
			             info->descname(), info->author(),
			             (info->verified ? _("Verified") : _("NOT VERIFIED")),
			             AddOns::version_to_string(installed_version),
			             AddOns::version_to_string(info->version),
			             AddOns::kAddOnCategories.at(info->category).descname(), info->description())),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		ctrl->install_or_upgrade(info, !full_upgrade_possible_);
		ctrl->rebuild(true);
	});
	if (info->internal_name.empty()) {
		install_.set_enabled(false);
		upgrade_.set_enabled(false);
		uninstall_.set_enabled(false);
		interact_.set_enabled(false);
	} else if (installed_version.empty()) {
		uninstall_.set_enabled(false);
		upgrade_.set_enabled(false);
	} else {
		install_.set_enabled(false);
		upgrade_.set_enabled(full_upgrade_possible_ || installed_i18n_version < info->i18n_version);
	}

	for (UI::Panel* p :
	     std::vector<UI::Panel*>{&category_, &version_, &verified_, &quality_, &bottom_row_right_}) {
		p->set_handle_mouse(true);
	}
	category_.set_tooltip(
	   format(_("Category: %s"), AddOns::kAddOnCategories.at(info->category).descname()));
	version_.set_tooltip(format(
	   /** TRANSLATORS: (MajorVersion)+(MinorVersion) */
	   _("Version: %1$s+%2$u"), AddOns::version_to_string(info->version), info->i18n_version));
	verified_.set_tooltip(
	   info->internal_name.empty() ?
         _("Error") :
	   info->verified ?
         _("Verified by the Widelands Development Team") :
         _("This add-on was not checked by the Widelands Development Team yet. We cannot guarantee "
	        "that it does not contain harmful or offensive content."));
	quality_.set_tooltip(info->internal_name.empty() ?
                           _("Error") :
                           AddOnQuality::kQualities.at(info->quality)().description);
	bottom_row_right_.set_tooltip(
	   info->internal_name.empty() ?
         "" :
         format(
	         "%s<br>%s<br>%s<br>%s<br>%s",
	         format(ngettext("Total size: %u byte", "Total size: %u bytes", info->total_file_size),
	                info->total_file_size),
	         format(
	            ngettext("%u download", "%u downloads", info->download_count), info->download_count),
	         (info->number_of_votes() ?
                format_l(ngettext("Average rating: %1$.3f (%2$u vote)",
	                               "Average rating: %1$.3f (%2$u votes)", info->number_of_votes()),
	                      info->average_rating(), info->number_of_votes()) :
                _("No votes yet")),
	         format(ngettext("%u comment", "%u comments", info->user_comments.size()),
	                info->user_comments.size()),
	         format(ngettext("%u screenshot", "%u screenshots", info->screenshots.size()),
	                info->screenshots.size())));

	layout();
}

void RemoteAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 3 * kRowButtonSize) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 4 * kRowButtonSize);
	for (UI::Panel* p : std::vector<UI::Panel*>{
	        &install_, &uninstall_, &upgrade_, &category_, &version_, &verified_, &quality_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	const int icon_size = 2 * kRowButtonSize;
	icon_.set_size(icon_size, icon_size);
	icon_.set_pos(Vector2i(0, 0));
	version_.set_size(
	   3 * kRowButtonSize + 2 * kRowButtonSpacing, kRowButtonSize - kRowButtonSpacing);
	version_.set_pos(Vector2i(
	   get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing, kRowButtonSize + kRowButtonSpacing));
	uninstall_.set_pos(Vector2i(get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing, 0));
	upgrade_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 0));
	install_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	interact_.set_size(2 * kRowButtonSize + kRowButtonSpacing, kRowButtonSize);
	interact_.set_pos(
	   Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 2 * kRowButtonSize));
	category_.set_pos(
	   Vector2i(get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing, 2 * kRowButtonSize));
	verified_.set_pos(Vector2i(0, 2 * kRowButtonSize));
	quality_.set_pos(Vector2i(kRowButtonSize, 2 * kRowButtonSize));
	txt_.set_size(
	   get_w() - icon_size - 3 * (kRowButtonSize + kRowButtonSpacing), 3 * kRowButtonSize);
	txt_.set_pos(Vector2i(icon_size, 0));
	bottom_row_left_.set_size(
	   get_w() / 2 - kRowButtonSpacing, kRowButtonSize - 2 * kRowButtonSpacing);
	bottom_row_right_.set_size(get_w() / 2 - kRowButtonSpacing, bottom_row_left_.get_h());
	bottom_row_left_.set_pos(
	   Vector2i(kRowButtonSpacing, 4 * kRowButtonSize - bottom_row_left_.get_h()));
	bottom_row_right_.set_pos(Vector2i(bottom_row_left_.get_x() + bottom_row_left_.get_w(),
	                                   4 * kRowButtonSize - bottom_row_right_.get_h()));
}

bool RemoteAddOnRow::upgradeable() const {
	return upgrade_.enabled();
}

}  // namespace AddOnsUI
}  // namespace FsMenu
