/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#include <cstdlib>
#include <iomanip>
#include <memory>

#include <SDL.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/warning.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "network/crypto.h"
#include "scripting/lua_table.h"
#include "ui_basic/color_chooser.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_fsmenu/addons_packager.h"
#include "wlapplication.h"
#include "wlapplication_options.h"

namespace FsMenu {

constexpr int16_t kRowButtonSize = 32;
constexpr int16_t kRowButtonSpacing = 4;

constexpr const char* const kDocumentationURL = "https://www.widelands.org/documentation/add-ons/";
constexpr const char* const kForumURL = "https://www.widelands.org/forum/17/";

// UI::Box by defaults limits its size to the window resolution. We use scrollbars,
// so we can and need to allow somewhat larger dimensions.
constexpr int32_t kHugeSize = std::numeric_limits<int32_t>::max() / 2;

static std::string underline_tag(const std::string& text) {
	std::string str = "<font underline=true>";
	str += text;
	str += "</font>";
	return str;
}
static std::string time_string(const std::time_t& time) {
	std::ostringstream oss("");
	try {
		oss.imbue(std::locale(i18n::get_locale()));
	} catch (...) {
		// silently ignore
	}
	oss << std::put_time(std::localtime(&time), "%c");
	return oss.str();
}
static std::string filesize_string(const uint32_t bytes) {
	if (bytes > 1000000000) {
		return (boost::format(_("%.2f GB")) % (bytes / 1000000000.f)).str();
	} else if (bytes > 1000000) {
		return (boost::format(_("%.2f MB")) % (bytes / 1000000.f)).str();
	} else if (bytes > 1000) {
		return (boost::format(_("%.2f kB")) % (bytes / 1000.f)).str();
	} else {
		return (boost::format(_("%u bytes")) % bytes).str();
	}
}

struct OperationCancelledByUserException : std::exception {};

class AddOnsLoginBox : public UI::Window {
public:
	explicit AddOnsLoginBox(AddOnsCtrl& ctrl)
	   : UI::Window(&ctrl.get_topmost_forefather(),
	                UI::WindowStyle::kFsMenu,
	                "login",
	                0,
	                0,
	                100,
	                100,
	                _("Login")),
	     password_sha1_(get_config_string("password_sha1", "")),
	     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     hbox_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     left_box_(&hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     right_box_(&hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     buttons_box_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     username_(&right_box_, 0, 0, 400, UI::PanelStyle::kFsMenu),
	     password_(&right_box_, 0, 0, 400, UI::PanelStyle::kFsMenu),
	     ok_(&buttons_box_,
	         "ok",
	         0,
	         0,
	         kRowButtonSize,
	         kRowButtonSize,
	         UI::ButtonStyle::kFsMenuPrimary,
	         _("OK")),
	     cancel_(&buttons_box_,
	             "cancel",
	             0,
	             0,
	             kRowButtonSize,
	             kRowButtonSize,
	             UI::ButtonStyle::kFsMenuSecondary,
	             _("Cancel")),
	     reset_(&buttons_box_,
	            "reset",
	            0,
	            0,
	            kRowButtonSize,
	            kRowButtonSize,
	            UI::ButtonStyle::kFsMenuSecondary,
	            _("Reset")) {
		UI::MultilineTextarea* m = new UI::MultilineTextarea(
		   &box_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu, "", UI::Align::kLeft,
		   UI::MultilineTextarea::ScrollMode::kNoScrolling);
		m->set_style(UI::FontStyle::kFsMenuInfoPanelParagraph);
		m->set_text(
		   (boost::format(_(
		       "In order to use a registered account, you need an account on the Widelands website. "
		       "Please log in at %s and set an online gaming password on your profile page.")) %
		    "\n\nhttps://widelands.org/accounts/register/\n\n")
		      .str());

		left_box_.add_inf_space();
		left_box_.add(new UI::Textarea(&left_box_, UI::PanelStyle::kFsMenu,
		                               UI::FontStyle::kFsMenuInfoPanelHeading, _("Username:"),
		                               UI::Align::kRight),
		              UI::Box::Resizing::kFullSize);
		left_box_.add_inf_space();
		left_box_.add(new UI::Textarea(&left_box_, UI::PanelStyle::kFsMenu,
		                               UI::FontStyle::kFsMenuInfoPanelHeading, _("Password:"),
		                               UI::Align::kRight),
		              UI::Box::Resizing::kFullSize);
		left_box_.add_inf_space();

		right_box_.add(&username_, UI::Box::Resizing::kExpandBoth);
		right_box_.add_space(kRowButtonSpacing);
		right_box_.add(&password_, UI::Box::Resizing::kExpandBoth);

		hbox_.add(&left_box_, UI::Box::Resizing::kFullSize);
		hbox_.add_space(kRowButtonSpacing);
		hbox_.add(&right_box_, UI::Box::Resizing::kExpandBoth);

		buttons_box_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
		buttons_box_.add_space(kRowButtonSpacing);
		buttons_box_.add(&reset_, UI::Box::Resizing::kExpandBoth);
		buttons_box_.add_space(kRowButtonSpacing);
		buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);

		box_.add(&hbox_, UI::Box::Resizing::kFullSize);
		box_.add_space(kRowButtonSpacing);
		box_.add(m, UI::Box::Resizing::kFullSize);
		box_.add_space(kRowButtonSpacing);
		box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

		ok_.sigclicked.connect([this]() { ok(); });
		username_.ok.connect([this]() { ok(); });
		password_.ok.connect([this]() { ok(); });
		cancel_.sigclicked.connect([this]() { die(); });
		username_.cancel.connect([this]() { die(); });
		password_.cancel.connect([this]() { die(); });
		reset_.sigclicked.connect([this]() {
			password_.set_can_focus(false);
			reset();
			password_.set_can_focus(true);
		});
		password_.set_password(true);

		reset();
		set_center_panel(&box_);
		center_to_parent();

		initialization_complete();
	}

	const std::string& get_username() const {
		return username_.text();
	}
	std::string get_password() const {
		const std::string& p = password_.text();
		return (p.empty() || p == password_sha1_) ? p : crypto::sha1(p);
	}

	bool handle_key(bool down, SDL_Keysym code) override {
		if (down) {
			switch (code.sym) {
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				ok();
				return true;
			case SDLK_ESCAPE:
				die();
				return true;
			default:
				break;
			}
		}
		return UI::Window::handle_key(down, code);
	}

	void think() override {
		UI::Window::think();
		ok_.set_enabled(!username_.text().empty() && !password_.text().empty());
		if (!password_sha1_.empty() && password_.has_focus() && password_.text() == password_sha1_) {
			password_.set_text("");
		}
	}

private:
	const std::string password_sha1_;
	UI::Box box_, hbox_, left_box_, right_box_, buttons_box_;
	UI::EditBox username_, password_;
	UI::Button ok_, cancel_, reset_;

	void ok() {
		if (!username_.text().empty() && !password_.text().empty()) {
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
		}
	}

	void reset() {
		if (get_config_bool("registered", false)) {
			username_.set_text(get_config_string("nickname", ""));
			password_.set_text(password_sha1_);
		} else {
			username_.set_text("");
			password_.set_text("");
		}
	}
};

class ScreenshotUploadWindow : public UI::Window {
public:
	explicit ScreenshotUploadWindow(AddOnsCtrl& ctrl,
	                                std::shared_ptr<AddOns::AddOnInfo> info,
	                                std::shared_ptr<AddOns::AddOnInfo> remote)
	   : UI::Window(&ctrl.get_topmost_forefather(),
	                UI::WindowStyle::kFsMenu,
	                "upload_screenshot",
	                0,
	                0,
	                100,
	                100,
	                (boost::format(_("Upload Screenshot for ‘%s’")) % info->internal_name).str()),
	     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     hbox_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     vbox_(&hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     descrbox_(&vbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     buttons_box_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     ok_(&buttons_box_,
	         "ok",
	         0,
	         0,
	         7 * kRowButtonSize,
	         kRowButtonSize,
	         UI::ButtonStyle::kFsMenuPrimary,
	         _("Upload")),
	     cancel_(&buttons_box_,
	             "cancel",
	             0,
	             0,
	             7 * kRowButtonSize,
	             kRowButtonSize,
	             UI::ButtonStyle::kFsMenuSecondary,
	             _("Cancel")),
	     images_(&hbox_, 0, 0, 150, 200, UI::PanelStyle::kFsMenu),
	     icon_(&vbox_, UI::PanelStyle::kFsMenu, 0, 0, 640, 360, nullptr),
	     description_(&descrbox_, 0, 0, 300, UI::PanelStyle::kFsMenu),
	     progress_(&buttons_box_,
	               UI::PanelStyle::kFsMenu,
	               UI::FontStyle::kFsMenuLabel,
	               "",
	               UI::Align::kCenter) {
		buttons_box_.add(&cancel_, UI::Box::Resizing::kFullSize);
		buttons_box_.add(&progress_, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
		buttons_box_.add(&ok_, UI::Box::Resizing::kFullSize);

		descrbox_.add(new UI::Textarea(&descrbox_, UI::PanelStyle::kFsMenu,
		                               UI::FontStyle::kFsMenuInfoPanelHeading, _("Description:"),
		                               UI::Align::kRight),
		              UI::Box::Resizing::kAlign, UI::Align::kCenter);
		descrbox_.add_space(kRowButtonSpacing);
		descrbox_.add(&description_, UI::Box::Resizing::kExpandBoth);

		vbox_.add(&icon_, UI::Box::Resizing::kExpandBoth);
		vbox_.add_space(kRowButtonSpacing);
		vbox_.add(&descrbox_, UI::Box::Resizing::kExpandBoth);

		hbox_.add(&images_, UI::Box::Resizing::kExpandBoth);
		hbox_.add_space(kRowButtonSpacing);
		hbox_.add(&vbox_, UI::Box::Resizing::kExpandBoth);

		box_.add(&hbox_, UI::Box::Resizing::kFullSize);
		box_.add_space(kRowButtonSpacing);
		box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

		description_.set_tooltip(_("Description"));
		for (const std::string& img : g_fs->list_directory(kScreenshotsDir)) {
			images_.add(FileSystem::fs_filename(img.c_str()), img);
		}

		images_.selected.connect(
		   [this](uint32_t) { icon_.set_icon(g_image_cache->get(images_.get_selected())); });
		cancel_.sigclicked.connect([this]() { die(); });
		ok_.sigclicked.connect([this, &ctrl, info, remote]() {
			if (!images_.has_selection() || description_.text().empty()) {
				return;
			}
			const std::string& sel = images_.get_selected();
			progress_.set_text(_("Uploading…"));
			ok_.set_enabled(false);
			cancel_.set_enabled(false);
			do_redraw_now();
			try {
				ctrl.net().upload_screenshot(info->internal_name, sel, description_.text());
				if (remote.get() != nullptr) {
					*remote = ctrl.net().fetch_one_remote(remote->internal_name);
					ctrl.rebuild();
				}
				die();
			} catch (const std::exception& e) {
				log_err("Upload screenshot %s for %s: %s", sel.c_str(), info->internal_name.c_str(),
				        e.what());
				progress_.set_text("");
				UI::WLMessageBox m(
				   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
				   (boost::format(_("The screenshot ‘%1$s’ for the add-on ‘%2$s’ could not be uploaded "
				                    "to the server.\n\nError Message:\n%3$s")) %
				    sel % info->internal_name % e.what())
				      .str(),
				   UI::WLMessageBox::MBoxType::kOk);
				m.run<UI::Panel::Returncodes>();
				ok_.set_enabled(true);
				cancel_.set_enabled(true);
			}
		});

		set_center_panel(&box_);
		center_to_parent();

		initialization_complete();
	}

	void think() override {
		ok_.set_enabled(images_.has_selection() && !description_.text().empty());
		UI::Window::think();
	}

private:
	UI::Box box_, hbox_, vbox_, descrbox_, buttons_box_;
	UI::Button ok_, cancel_;
	UI::Listselect<std::string> images_;
	UI::Icon icon_;
	UI::EditBox description_;
	UI::Textarea progress_;
};

class ContactForm : public UI::Window {
public:
	explicit ContactForm(AddOnsCtrl& ctrl)
	   : UI::Window(&ctrl.get_topmost_forefather(),
	                UI::WindowStyle::kFsMenu,
	                "contact",
	                0,
	                0,
	                100,
	                100,
	                _("Contact Form")),
	     ctrl_(ctrl),
	     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     buttons_box_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     message_(new UI::MultilineEditbox(&box_, 0, 0, 400, 200, UI::PanelStyle::kFsMenu)),
	     ok_(&buttons_box_,
	         "ok",
	         0,
	         0,
	         kRowButtonSize,
	         kRowButtonSize,
	         UI::ButtonStyle::kFsMenuPrimary,
	         _("Send message")),
	     cancel_(&buttons_box_,
	             "cancel",
	             0,
	             0,
	             kRowButtonSize,
	             kRowButtonSize,
	             UI::ButtonStyle::kFsMenuSecondary,
	             _("Cancel")) {
		buttons_box_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
		box_.add_space(kRowButtonSpacing);
		buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);

		box_.add(message_, UI::Box::Resizing::kExpandBoth);
		box_.add_space(kRowButtonSpacing);
		box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

		check_ok_button_enabled();

		cancel_.sigclicked.connect([this]() { die(); });
		ok_.sigclicked.connect([this]() {
			if (!check_ok_button_enabled()) {
				return;
			}
			UI::WLMessageBox w(&get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Send Message"),
			                   _("Send this message now?"), UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
			try {
				ctrl_.net().contact(message_->get_text());
			} catch (const std::exception& e) {
				log_err("contact error: %s", e.what());
				UI::WLMessageBox m(
				   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
				   (boost::format(_("Unable to submit your enquiry.\nError message:\n%s")) % e.what())
				      .str(),
				   UI::WLMessageBox::MBoxType::kOk);
				m.run<UI::Panel::Returncodes>();
				return;
			}

			die();
			UI::WLMessageBox m(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Sent"),
			   _("Your message has been sent to the Widelands Development Team, and we will respond "
			     "as soon as we can. Don’t forget to check your website inbox frequently."),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
		});

		set_center_panel(&box_);
		center_to_parent();

		initialization_complete();
	}

	void think() override {
		UI::Window::think();
		check_ok_button_enabled();
	}

private:
	AddOnsCtrl& ctrl_;

	bool check_ok_button_enabled() {
		ok_.set_enabled(!message_->get_text().empty());
		return ok_.enabled();
	}

	UI::Box box_, buttons_box_;
	UI::MultilineEditbox* message_;
	UI::Button ok_, cancel_;
};

ProgressIndicatorWindow::ProgressIndicatorWindow(UI::Panel* parent, const std::string& title)
   : UI::Window(parent,
                UI::WindowStyle::kFsMenu,
                "progress",
                0,
                0,
                parent->get_inner_w() - 2 * kRowButtonSize,
                2 * kRowButtonSize,
                title),
     modal_(*this),
     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, get_inner_w()),
     hbox_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     txt1_(&box_,
           UI::PanelStyle::kFsMenu,
           UI::FontStyle::kFsMenuInfoPanelHeading,
           "",
           UI::Align::kCenter),
     txt2_(&hbox_,
           UI::PanelStyle::kFsMenu,
           UI::FontStyle::kFsMenuInfoPanelParagraph,
           "",
           UI::Align::kLeft),
     txt3_(&hbox_,
           UI::PanelStyle::kFsMenu,
           UI::FontStyle::kFsMenuInfoPanelParagraph,
           "",
           UI::Align::kRight),
     progress_(&box_,
               UI::PanelStyle::kFsMenu,
               0,
               0,
               get_w(),
               kRowButtonSize,
               UI::ProgressBar::Horizontal) {

	hbox_.add(&txt2_, UI::Box::Resizing::kExpandBoth);
	hbox_.add(&txt3_, UI::Box::Resizing::kExpandBoth);
	box_.add(&txt1_, UI::Box::Resizing::kFullSize);
	box_.add_space(kRowButtonSpacing);
	box_.add(&hbox_, UI::Box::Resizing::kFullSize);
	box_.add_space(2 * kRowButtonSpacing);
	box_.add(&progress_, UI::Box::Resizing::kFullSize);

	set_center_panel(&box_);
	center_to_parent();

	initialization_complete();
}

AddOnsCtrl::AddOnsCtrl(MainMenu& fsmm, UI::UniqueWindow::Registry& reg)
   : UI::UniqueWindow(&fsmm,
                      UI::WindowStyle::kFsMenu,
                      "addons",
                      &reg,
                      fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuDefault),
                      fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuDefault),
                      _("Add-On Manager")),
     fsmm_(fsmm),
     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     buttons_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     warn_requirements_(
        &main_box_, 0, 0, get_w(), get_h() / 12, UI::PanelStyle::kFsMenu, "", UI::Align::kCenter),
     tabs_placeholder_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, 0, 0),
     tabs_(this, UI::TabPanelStyle::kFsMenu),
     installed_addons_outer_wrapper_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     installed_addons_inner_wrapper_(
        &installed_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     installed_addons_buttons_box_(
        &installed_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     installed_addons_box_(&installed_addons_inner_wrapper_,
                           UI::PanelStyle::kFsMenu,
                           0,
                           0,
                           UI::Box::Vertical,
                           kHugeSize,
                           kHugeSize),
     browse_addons_outer_wrapper_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     browse_addons_inner_wrapper_(
        &browse_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     browse_addons_buttons_box_(
        &browse_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     browse_addons_buttons_inner_box_1_(
        &browse_addons_buttons_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     browse_addons_buttons_inner_box_2_(
        &browse_addons_buttons_inner_box_1_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     browse_addons_box_(&browse_addons_inner_wrapper_,
                        UI::PanelStyle::kFsMenu,
                        0,
                        0,
                        UI::Box::Vertical,
                        kHugeSize,
                        kHugeSize),
     dev_box_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     filter_name_(&browse_addons_buttons_inner_box_1_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     filter_verified_(&browse_addons_buttons_inner_box_2_,
                      UI::PanelStyle::kFsMenu,
                      Vector2i(0, 0),
                      _("Verified only"),
                      _("Show only verified add-ons in the Browse tab")),
     sort_order_(&browse_addons_buttons_inner_box_1_,
                 "sort",
                 0,
                 0,
                 0,
                 10,
                 filter_name_.get_h(),
                 _("Sort by"),
                 UI::DropdownType::kTextual,
                 UI::PanelStyle::kFsMenu,
                 UI::ButtonStyle::kFsMenuSecondary),
     upload_addon_(&dev_box_,
                   "upload_addon",
                   0,
                   0,
                   get_inner_w() / 2,
                   8,
                   kRowButtonSize,
                   _("Choose add-on to upload…"),
                   UI::DropdownType::kTextualMenu,
                   UI::PanelStyle::kFsMenu,
                   UI::ButtonStyle::kFsMenuSecondary),
     upload_screenshot_(&dev_box_,
                        "upload_screenie",
                        0,
                        0,
                        get_inner_w() / 2,
                        8,
                        kRowButtonSize,
                        _("Upload a screenshot…"),
                        UI::DropdownType::kTextualMenu,
                        UI::PanelStyle::kFsMenu,
                        UI::ButtonStyle::kFsMenuSecondary),
     upload_addon_accept_(&dev_box_,
                          UI::PanelStyle::kFsMenu,
                          Vector2i(0, 0),
                          _("Understood and confirmed"),
                          _("By ticking this checkbox, you confirm that you have read and agree to "
                            "the above terms.")),
     filter_reset_(&browse_addons_buttons_inner_box_2_,
                   "f_reset",
                   0,
                   0,
                   24,
                   24,
                   UI::ButtonStyle::kFsMenuSecondary,
                   _("Reset"),
                   _("Reset the filters")),
     upgrade_all_(&buttons_box_,
                  "upgrade_all",
                  0,
                  0,
                  kRowButtonSize,
                  kRowButtonSize,
                  UI::ButtonStyle::kFsMenuSecondary,
                  ""),
     refresh_(&buttons_box_,
              "refresh",
              0,
              0,
              kRowButtonSize,
              kRowButtonSize,
              UI::ButtonStyle::kFsMenuSecondary,
              _("Refresh"),
              _("Refresh the list of add-ons available from the server")),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         kRowButtonSize,
         kRowButtonSize,
         UI::ButtonStyle::kFsMenuPrimary,
         _("OK")),
#if 0  // TODO(Nordfriese): Disabled autofix_dependencies for v1.0
     autofix_dependencies_(&buttons_box_,
                           "autofix",
                           0,
                           0,
                           kRowButtonSize,
                           kRowButtonSize,
                           UI::ButtonStyle::kFsMenuSecondary,
                           _("Fix dependencies…"),
                           _("Try to automatically fix the dependency errors")),
#endif
     move_top_(&installed_addons_buttons_box_,
               "move_top",
               0,
               0,
               kRowButtonSize,
               kRowButtonSize,
               UI::ButtonStyle::kFsMenuSecondary,
               g_image_cache->get("images/ui_basic/scrollbar_up_fast.png"),
               _("Move selected add-on to top")),
     move_up_(&installed_addons_buttons_box_,
              "move_up",
              0,
              0,
              kRowButtonSize,
              kRowButtonSize,
              UI::ButtonStyle::kFsMenuSecondary,
              g_image_cache->get("images/ui_basic/scrollbar_up.png"),
              _("Move selected add-on one step up")),
     move_down_(&installed_addons_buttons_box_,
                "move_down",
                0,
                0,
                kRowButtonSize,
                kRowButtonSize,
                UI::ButtonStyle::kFsMenuSecondary,
                g_image_cache->get("images/ui_basic/scrollbar_down.png"),
                _("Move selected add-on one step down")),
     move_bottom_(&installed_addons_buttons_box_,
                  "move_bottom",
                  0,
                  0,
                  kRowButtonSize,
                  kRowButtonSize,
                  UI::ButtonStyle::kFsMenuSecondary,
                  g_image_cache->get("images/ui_basic/scrollbar_down_fast.png"),
                  _("Move selected add-on to bottom")),
     launch_packager_(&dev_box_,
                      "packager",
                      0,
                      0,
                      0,
                      0,
                      UI::ButtonStyle::kFsMenuSecondary,
                      _("Launch the add-ons packager…")),
     login_button_(this, "login", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, ""),
     contact_(&dev_box_,
              "contact",
              0,
              0,
              0,
              0,
              UI::ButtonStyle::kFsMenuSecondary,
              /** TRANSLATORS: This button allows the user to send a message to the Widelands
                 Development Team */
              _("Contact us…")) {

	dev_box_.set_force_scrolling(true);
	dev_box_.add(
	   new UI::Textarea(&dev_box_, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuInfoPanelHeading,
	                    _("Tools for Add-Ons Developers"), UI::Align::kCenter),
	   UI::Box::Resizing::kFullSize);
	dev_box_.add_space(kRowButtonSize);
	{
		UI::MultilineTextarea* m = new UI::MultilineTextarea(
		   &dev_box_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu, "", UI::Align::kLeft,
		   UI::MultilineTextarea::ScrollMode::kNoScrolling);
		m->set_style(UI::FontStyle::kFsMenuInfoPanelParagraph);
		m->set_text(_("The interactive add-ons packager allows you to create, edit, and delete "
		              "add-ons. You can bundle maps designed with the Widelands Map Editor as an "
		              "add-on using the graphical interface and share them with other players, "
		              "without having to write a single line of code."));
		dev_box_.add(m, UI::Box::Resizing::kFullSize);
	}
	dev_box_.add_space(kRowButtonSpacing);
	dev_box_.add(&launch_packager_);

	dev_box_.add_space(kRowButtonSize);
	dev_box_.add(
	   new UI::MultilineTextarea(
	      &dev_box_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu,
	      (boost::format("<rt><p>%1$s</p></rt>") %
	       g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	          .as_font_tag(
	             (boost::format(_("For more information regarding how to develop and package your "
	                              "own add-ons, please visit %s.")) %
	              underline_tag(kDocumentationURL))
	                .str()))
	         .str(),
	      UI::Align::kLeft, UI::MultilineTextarea::ScrollMode::kNoScrolling),
	   UI::Box::Resizing::kFullSize);
	auto add_button = [this](const std::string& url) {
		UI::Button* b =
		   new UI::Button(&dev_box_, "url", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary,
#if SDL_VERSION_ATLEAST(2, 0, 14)
		                  _("Open Link")
#else
		                  _("Copy Link")
#endif
		   );
		b->sigclicked.connect([url]() {
#if SDL_VERSION_ATLEAST(2, 0, 14)
			SDL_OpenURL(url.c_str());
#else
			SDL_SetClipboardText(url.c_str());
#endif
		});
		dev_box_.add(b);
	};
	dev_box_.add_space(kRowButtonSpacing);
	add_button(kDocumentationURL);

	dev_box_.add_space(kRowButtonSize);
	dev_box_.add(
	   new UI::MultilineTextarea(
	      &dev_box_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu,
	      (boost::format("<rt><p>%s</p><p>%s</p><p>%s</p><p>%s</p></rt>") %
	       g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	          .as_font_tag(
	             _("Here, you can upload your add-ons to the server to make them available to other "
	               "players. By uploading, you agree to publish your creation under the terms of "
	               "the GNU General Public License (GPL) version 2 (the same license under which "
	               "Widelands itself is distributed). For more information on the GPL, please refer "
	               "to ‘About Widelands’ → ‘License’ in the main menu.")) %
	       g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	          .as_font_tag(_(
	             "It is forbidden to upload add-ons containing harmful or malicious content or "
	             "spam. By uploading an add-on, you assert that the add-on is of your own creation "
	             "or you have the add-on’s author(s) permission to submit it in their stead.")) %
	       g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	          .as_font_tag(
	             _("You are required to have read the add-ons documentation under the link given "
	               "further above before submitting content. Since the documentation is subject to "
	               "frequent changes, ensure that you have read it recently and that you followed "
	               "all guidelines stated there.")) %
	       g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	          .as_font_tag(_("The Widelands Development Team will review your add-on soon after "
	                         "uploading. In case they have further inquiries, they will contact you "
	                         "via a PM on the Widelands website; therefore please check the inbox "
	                         "of your online user profile page frequently.")))
	         .str(),
	      UI::Align::kLeft, UI::MultilineTextarea::ScrollMode::kNoScrolling),
	   UI::Box::Resizing::kFullSize);

	dev_box_.add_space(kRowButtonSpacing);
	dev_box_.add(&upload_addon_accept_);
	dev_box_.add_space(kRowButtonSpacing);
	dev_box_.add(&upload_addon_);
	dev_box_.add_space(kRowButtonSpacing);
	dev_box_.add(&upload_screenshot_);

	upload_addon_accept_.changed.connect([this]() { update_login_button(nullptr); });
	upload_addon_.selected.connect([this]() { upload_addon(upload_addon_.get_selected()); });
	upload_screenshot_.selected.connect([this]() {
		upload_screenshot_.set_list_visibility(false);
		std::shared_ptr<AddOns::AddOnInfo> info = upload_screenshot_.get_selected();
		ScreenshotUploadWindow s(*this, info, find_remote(info->internal_name));
		s.run<UI::Panel::Returncodes>();
	});

	dev_box_.add_space(kRowButtonSize);
	dev_box_.add(
	   new UI::MultilineTextarea(
	      &dev_box_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu,
	      (boost::format("<rt><p>%1$s</p></rt>") %
	       g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	          .as_font_tag(
	             (boost::format(_("Technical problems? Unclear documentation? Advanced needs such "
	                              "as deletion of an add-on or collaborating with another add-on "
	                              "designer? Please visit our forums at %s, explain your needs, and "
	                              "the Widelands Development Team will be happy to help.")) %
	              underline_tag(kForumURL))
	                .str()))
	         .str(),
	      UI::Align::kLeft, UI::MultilineTextarea::ScrollMode::kNoScrolling),
	   UI::Box::Resizing::kFullSize);
	dev_box_.add_space(kRowButtonSpacing);
	add_button(kForumURL);

	dev_box_.add_space(kRowButtonSize);
	dev_box_.add(new UI::MultilineTextarea(
	                &dev_box_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu,
	                (boost::format("<rt><p>%1$s</p></rt>") %
	                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	                    .as_font_tag(_(
	                       "Alternatively you can also send a message to the Widelands Development "
	                       "Team and we’ll try to help you with your enquiry as soon as we can. "
	                       "Only the server administrators can read messages sent in this way. "
	                       "Since other add-on designers can usually benefit from others’ questions "
	                       "and answers, please use this way of communication only if there is a "
	                       "reason why your concern is not for everyone’s ears. You’ll receive our "
	                       "reply via a PM on your Widelands website user profile page.")))
	                   .str(),
	                UI::Align::kLeft, UI::MultilineTextarea::ScrollMode::kNoScrolling),
	             UI::Box::Resizing::kFullSize);
	contact_.sigclicked.connect([this]() {
		ContactForm c(*this);
		c.run<UI::Panel::Returncodes>();
	});
	dev_box_.add_space(kRowButtonSpacing);
	dev_box_.add(&contact_);

	installed_addons_buttons_box_.add(&move_top_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_buttons_box_.add_space(kRowButtonSpacing);
	installed_addons_buttons_box_.add(&move_up_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_buttons_box_.add_space(kRowButtonSize + 2 * kRowButtonSpacing);
	installed_addons_buttons_box_.add(&move_down_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_buttons_box_.add_space(kRowButtonSpacing);
	installed_addons_buttons_box_.add(&move_bottom_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_outer_wrapper_.add(
	   &installed_addons_inner_wrapper_, UI::Box::Resizing::kExpandBoth);
	installed_addons_outer_wrapper_.add_space(kRowButtonSpacing);
	installed_addons_outer_wrapper_.add(
	   &installed_addons_buttons_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	browse_addons_outer_wrapper_.add(&browse_addons_buttons_box_, UI::Box::Resizing::kFullSize);
	browse_addons_outer_wrapper_.add_space(2 * kRowButtonSpacing);
	browse_addons_outer_wrapper_.add(&browse_addons_inner_wrapper_, UI::Box::Resizing::kExpandBoth);

	installed_addons_inner_wrapper_.add(&installed_addons_box_, UI::Box::Resizing::kExpandBoth);
	browse_addons_inner_wrapper_.add(&browse_addons_box_, UI::Box::Resizing::kExpandBoth);
	tabs_.add("my", "", &installed_addons_outer_wrapper_);
	tabs_.add("all", "", &browse_addons_outer_wrapper_);
	tabs_.add("all", _("Development"), &dev_box_);

	/** TRANSLATORS: Sort add-ons alphabetically by name */
	sort_order_.add(_("Name"), AddOnSortingCriteria::kNameABC);
	/** TRANSLATORS: Sort add-ons alphabetically by name (inverted) */
	sort_order_.add(_("Name (descending)"), AddOnSortingCriteria::kNameCBA);
	/** TRANSLATORS: Sort add-ons by average rating */
	sort_order_.add(_("Best average rating"), AddOnSortingCriteria::kHighestRating, nullptr, true);
	/** TRANSLATORS: Sort add-ons by average rating */
	sort_order_.add(_("Worst average rating"), AddOnSortingCriteria::kLowestRating);
	/** TRANSLATORS: Sort add-ons by how often they were downloaded */
	sort_order_.add(_("Most often downloaded"), AddOnSortingCriteria::kMostDownloads);
	/** TRANSLATORS: Sort add-ons by how often they were downloaded */
	sort_order_.add(_("Least often downloaded"), AddOnSortingCriteria::kFewestDownloads);
	/** TRANSLATORS: Sort add-ons by upload date/time */
	sort_order_.add(_("Oldest"), AddOnSortingCriteria::kOldest);
	/** TRANSLATORS: Sort add-ons by upload date/time */
	sort_order_.add(_("Newest"), AddOnSortingCriteria::kNewest);

	filter_verified_.set_state(true);
	filter_name_.set_tooltip(_("Filter add-ons by name"));
	{
		uint8_t index = 0;
		for (const auto& pair : AddOns::kAddOnCategories) {
			if (pair.first == AddOns::AddOnCategory::kNone) {
				continue;
			}
			UI::Checkbox* c = new UI::Checkbox(
			   &browse_addons_buttons_box_, UI::PanelStyle::kFsMenu, Vector2i(0, 0),
			   g_image_cache->get(pair.second.icon),
			   (boost::format(_("Toggle category ‘%s’")) % pair.second.descname()).str());
			filter_category_[pair.first] = c;
			c->set_state(true);
			c->changed.connect([this, &pair]() { category_filter_changed(pair.first); });
			c->set_desired_size(kRowButtonSize, kRowButtonSize);
			browse_addons_buttons_box_.add(c, UI::Box::Resizing::kAlign, UI::Align::kCenter);
			browse_addons_buttons_box_.add_space(kRowButtonSpacing);
			++index;
		}
	}
	browse_addons_buttons_inner_box_2_.add(&filter_verified_, UI::Box::Resizing::kFullSize);
	browse_addons_buttons_inner_box_2_.add(&filter_reset_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_inner_box_1_.add(
	   &browse_addons_buttons_inner_box_2_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_inner_box_1_.add_space(kRowButtonSpacing);
	browse_addons_buttons_inner_box_1_.add(&filter_name_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_inner_box_1_.add_space(kRowButtonSpacing);
	browse_addons_buttons_inner_box_1_.add(&sort_order_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_box_.add(
	   &browse_addons_buttons_inner_box_1_, UI::Box::Resizing::kExpandBoth);

	filter_reset_.set_enabled(false);
	filter_name_.changed.connect([this]() {
		filter_reset_.set_enabled(true);
		rebuild();
	});
	filter_verified_.changed.connect([this]() {
		filter_reset_.set_enabled(true);
		rebuild();
	});
	sort_order_.selected.connect([this]() { rebuild(); });

	ok_.sigclicked.connect([this]() { die(); });
	refresh_.sigclicked.connect([this]() {
		refresh_remotes();
		tabs_.activate(1);
	});
	tabs_.sigclicked.connect([this]() {
		if (tabs_.active() == 1 && remotes_.size() <= 1) {
			refresh_remotes();
		}
	});
#if 0  // TODO(Nordfriese): Disabled autofix_dependencies for v1.0
	autofix_dependencies_.sigclicked.connect([this]() { autofix_dependencies(); });
#endif

	filter_reset_.sigclicked.connect([this]() {
		filter_name_.set_text("");
		filter_verified_.set_state(true);
		for (auto& pair : filter_category_) {
			pair.second->set_state(true);
		}
		rebuild();
		filter_reset_.set_enabled(false);
	});
	upgrade_all_.sigclicked.connect([this]() {
		std::vector<std::pair<std::shared_ptr<AddOns::AddOnInfo>, bool /* full upgrade */>> upgrades;
		bool all_verified = true;
		size_t nr_full_updates = 0;
		for (const RemoteAddOnRow* r : browse_) {
			if (r->upgradeable()) {
				const bool full_upgrade = r->full_upgrade_possible();
				upgrades.push_back(std::make_pair(r->info(), full_upgrade));
				if (full_upgrade) {
					all_verified &= r->info()->verified;
					++nr_full_updates;
				}
			}
		}
		assert(!upgrades.empty());
		if (nr_full_updates > 0 && (!all_verified || !(SDL_GetModState() & KMOD_CTRL))) {
			// We ask for confirmation only for real upgrades. i18n-only upgrades are done silently.
			std::string text =
			   (boost::format(ngettext("Are you certain that you want to upgrade this %u add-on?",
			                           "Are you certain that you want to upgrade these %u add-ons?",
			                           nr_full_updates)) %
			    nr_full_updates)
			      .str();
			text += '\n';
			for (const auto& pair : upgrades) {
				if (pair.second) {
					text += (boost::format(_("\n· %1$s (%2$s) by %3$s")) % pair.first->descname() %
					         (pair.first->verified ? _("verified") : _("NOT VERIFIED")) %
					         pair.first->author())
					           .str();
				}
			}
			UI::WLMessageBox w(&get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Upgrade All"),
			                   text, UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		for (const auto& pair : upgrades) {
			install_or_upgrade(pair.first, !pair.second);
		}
		rebuild();
	});

	move_up_.sigclicked.connect([this]() {
		const auto& info = selected_installed_addon();
		auto it = AddOns::g_addons.begin();
		while (it->first->internal_name != info->internal_name) {
			++it;
		}
		const bool state = it->second;
		it = AddOns::g_addons.erase(it);
		--it;
		AddOns::g_addons.insert(it, std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});
	move_down_.sigclicked.connect([this]() {
		const auto& info = selected_installed_addon();
		auto it = AddOns::g_addons.begin();
		while (it->first->internal_name != info->internal_name) {
			++it;
		}
		const bool state = it->second;
		it = AddOns::g_addons.erase(it);
		++it;
		AddOns::g_addons.insert(it, std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});
	move_top_.sigclicked.connect([this]() {
		const auto& info = selected_installed_addon();
		auto it = AddOns::g_addons.begin();
		while (it->first->internal_name != info->internal_name) {
			++it;
		}
		const bool state = it->second;
		it = AddOns::g_addons.erase(it);
		AddOns::g_addons.insert(AddOns::g_addons.begin(), std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});
	move_bottom_.sigclicked.connect([this]() {
		const auto& info = selected_installed_addon();
		auto it = AddOns::g_addons.begin();
		while (it->first->internal_name != info->internal_name) {
			++it;
		}
		const bool state = it->second;
		it = AddOns::g_addons.erase(it);
		AddOns::g_addons.push_back(std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});

	launch_packager_.sigclicked.connect([this]() {
		AddOnsPackager a(fsmm_, *this);
		a.run<int>();

		// Perhaps add-ons were created or deleted
		rebuild();
	});

	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&upgrade_all_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&refresh_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
#if 0  // TODO(Nordfriese): Disabled autofix_dependencies for v1.0
	buttons_box_.add(&autofix_dependencies_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
#endif
	buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);

	main_box_.add(&tabs_placeholder_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(&warn_requirements_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kRowButtonSpacing);

	// prevent assert failures
	installed_addons_box_.set_size(100, 100);
	browse_addons_box_.set_size(100, 100);
	installed_addons_inner_wrapper_.set_size(100, 100);
	browse_addons_inner_wrapper_.set_size(100, 100);

	installed_addons_inner_wrapper_.set_force_scrolling(true);
	browse_addons_inner_wrapper_.set_force_scrolling(true);

	if (get_config_bool("registered", false)) {
		set_login(get_config_string("nickname", ""), get_config_string("password_sha1", ""), false);
	}
	login_button_.sigclicked.connect([this]() { login_button_clicked(); });
	update_login_button(&login_button_);

	do_not_layout_on_resolution_change();
	center_to_parent();
	rebuild();
}

AddOnsCtrl::~AddOnsCtrl() {
	std::string text;
	for (const auto& pair : AddOns::g_addons) {
		if (!text.empty()) {
			text += ',';
		}
		text += pair.first->internal_name + ':' + (pair.second ? "true" : "false");
	}
	set_config_string("addons", text);
	write_config();
	fsmm_.set_labels();
}

void AddOnsCtrl::login_button_clicked() {
	if (username_.empty()) {
		UI::UniqueWindow::Registry r;
		AddOnsLoginBox b(*this);
		if (b.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
		set_login(b.get_username(), b.get_password(), true);
	} else {
		set_login("", "", false);
	}
	update_login_button(&login_button_);
}

void AddOnsCtrl::update_login_button(UI::Button* b) {
	if (username_.empty()) {
		upload_addon_accept_.set_enabled(false);
		upload_addon_accept_.set_state(false);
		if (b) {
			b->set_title(_("Not logged in"));
			b->set_tooltip(_("Click to log in. You can then comment and vote on add-ons and upload "
			                 "your own add-ons."));
		}
		upload_addon_.set_enabled(false);
		upload_addon_.set_tooltip(_("Please log in to upload add-ons"));
		upload_screenshot_.set_enabled(upload_addon_accept_.get_state());
		upload_screenshot_.set_tooltip(_("Please log in to upload content"));
		contact_.set_enabled(false);
		contact_.set_tooltip(_("Please log in to send an enquiry"));
	} else {
		upload_addon_accept_.set_enabled(true);
		if (b) {
			b->set_title(
			   (boost::format(net().is_admin() ? _("Logged in as %s (admin)") : _("Logged in as %s")) %
			    username_)
			      .str());
			b->set_tooltip(_("Click to log out"));
		}
		const bool enable = upload_addon_accept_.get_state();
		upload_addon_.set_enabled(enable);
		upload_addon_.set_tooltip(
		   enable ? "" : _("Please tick the confirmation checkbox to upload add-ons"));
		upload_screenshot_.set_enabled(enable);
		upload_screenshot_.set_tooltip(
		   enable ? "" : _("Please tick the confirmation checkbox to upload content"));
		contact_.set_enabled(true);
		contact_.set_tooltip("");
	}
}

void AddOnsCtrl::set_login(const std::string& username,
                           const std::string& password,
                           const bool show_error) {
	if (password.empty() != username.empty()) {
		return;
	}

	username_ = username;
	try {
		net().set_login(username, password);

		if (!username.empty()) {
			set_config_string("nickname", username);
			set_config_bool("registered", true);
			set_config_string("password_sha1", password);
		}
	} catch (const std::exception& e) {
		if (username.empty()) {
			log_err("set_login (''): server error (%s)", e.what());
			if (show_error) {
				UI::WLMessageBox m(
				   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Server Error"),
				   (boost::format(_("Unable to connect to the server.\nError message:\n%s")) % e.what())
				      .str(),
				   UI::WLMessageBox::MBoxType::kOk);
				m.run<UI::Panel::Returncodes>();
			}
		} else {
			log_err("set_login as '%s': access denied (%s)", username.c_str(), e.what());
			if (show_error) {
				UI::WLMessageBox m(
				   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Wrong Password"),
				   (boost::format(_("The entered username or password is invalid:\n%s")) % e.what())
				      .str(),
				   UI::WLMessageBox::MBoxType::kOk);
				m.run<UI::Panel::Returncodes>();
			}
			set_login("", "", show_error);
		}
	}
}

inline std::shared_ptr<AddOns::AddOnInfo> AddOnsCtrl::selected_installed_addon() const {
	return dynamic_cast<InstalledAddOnRow&>(*installed_addons_box_.focused_child()).info();
}
void AddOnsCtrl::focus_installed_addon_row(std::shared_ptr<AddOns::AddOnInfo> info) {
	for (UI::Panel* p = installed_addons_box_.get_first_child(); p; p = p->get_next_sibling()) {
		if (dynamic_cast<InstalledAddOnRow&>(*p).info()->internal_name == info->internal_name) {
			p->focus();
			return;
		}
	}
	NEVER_HERE();
}

void AddOnsCtrl::think() {
	UI::Panel::think();
	check_enable_move_buttons();
}

static bool category_filter_changing = false;
void AddOnsCtrl::category_filter_changed(const AddOns::AddOnCategory which) {
	// protect against recursion
	if (category_filter_changing) {
		return;
	}
	category_filter_changing = true;

	// CTRL enables the selected category and disables all others
	if (SDL_GetModState() & KMOD_CTRL) {
		for (auto& pair : filter_category_) {
			pair.second->set_state(pair.first == which);
		}
	}

	filter_reset_.set_enabled(true);
	rebuild();
	category_filter_changing = false;
}

void AddOnsCtrl::check_enable_move_buttons() {
	const bool enable_move_buttons =
	   tabs_.active() == 0 && installed_addons_box_.focused_child() != nullptr;
	for (UI::Button* b : {&move_top_, &move_up_, &move_down_, &move_bottom_}) {
		b->set_enabled(enable_move_buttons);
	}
	if (enable_move_buttons) {
		const auto& sel = selected_installed_addon();
		if (sel->internal_name == AddOns::g_addons.begin()->first->internal_name) {
			move_top_.set_enabled(false);
			move_up_.set_enabled(false);
		}
		if (sel->internal_name == AddOns::g_addons.back().first->internal_name) {
			move_down_.set_enabled(false);
			move_bottom_.set_enabled(false);
		}
	}
}

bool AddOnsCtrl::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
		case SDLK_ESCAPE:
			die();
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void AddOnsCtrl::refresh_remotes() {
	try {
		remotes_ = net().refresh_remotes();
	} catch (const std::exception& e) {
		const std::string title = _("Server Connection Error");
		/** TRANSLATORS: This will be inserted into the string "Server Connection Error <br> by %s" */
		const std::string bug = _("a networking bug");
		const std::string err = (boost::format(_("Unable to fetch the list of available add-ons from "
		                                         "the server!<br>Error Message: %s")) %
		                         e.what())
		                           .str();
		AddOns::AddOnInfo* i = new AddOns::AddOnInfo();
		i->unlocalized_descname = title;
		i->unlocalized_description = err;
		i->unlocalized_author = bug;
		i->descname = [title]() { return title; };
		i->description = [err]() { return err; };
		i->author = [bug]() { return bug; };
		i->upload_username = bug;
		i->upload_timestamp = std::time(nullptr);
		i->icon = g_image_cache->get(AddOns::kAddOnCategories.at(AddOns::AddOnCategory::kNone).icon);
		i->sync_safe = true;  // suppress useless warning
		remotes_ = {std::shared_ptr<AddOns::AddOnInfo>(i)};
	}
	rebuild();
}

bool AddOnsCtrl::matches_filter(std::shared_ptr<AddOns::AddOnInfo> info) {
	if (info->internal_name.empty()) {
		// always show error messages
		return true;
	}

	if (!info->matches_widelands_version()) {
		// incompatible
		return false;
	}

	if (!filter_category_.at(info->category)->get_state()) {
		// wrong category
		return false;
	}

	if (filter_verified_.get_state() && !info->verified) {
		// not verified
		return false;
	}

	if (filter_name_.text().empty()) {
		// no text filter given, so we accept it
		return true;
	}
	for (const std::string& text : {info->descname(), info->author(), info->upload_username,
	                                info->internal_name, info->description()}) {
		if (text.find(filter_name_.text()) != std::string::npos) {
			// text filter found
			return true;
		}
	}
	// doesn't match the text filter
	return false;
}

void AddOnsCtrl::rebuild() {
	const uint32_t scrollpos_i =
	   installed_addons_inner_wrapper_.get_scrollbar() ?
         installed_addons_inner_wrapper_.get_scrollbar()->get_scrollpos() :
         0;
	const uint32_t scrollpos_b = browse_addons_inner_wrapper_.get_scrollbar() ?
                                   browse_addons_inner_wrapper_.get_scrollbar()->get_scrollpos() :
                                   0;
	installed_addons_box_.free_children();
	browse_addons_box_.free_children();
	installed_addons_box_.clear();
	browse_addons_box_.clear();
	browse_.clear();
	assert(installed_addons_box_.get_nritems() == 0);
	assert(browse_addons_box_.get_nritems() == 0);
	upload_addon_.clear();
	upload_screenshot_.clear();

	size_t index = 0;
	for (const auto& pair : AddOns::g_addons) {
		if (index > 0) {
			installed_addons_box_.add_space(kRowButtonSize);
		}
		InstalledAddOnRow* i =
		   new InstalledAddOnRow(&installed_addons_box_, this, pair.first, pair.second);
		installed_addons_box_.add(i, UI::Box::Resizing::kFullSize);
		++index;

		upload_addon_.add(
		   pair.first->internal_name, pair.first, pair.first->icon, false, pair.first->descname());
		upload_screenshot_.add(
		   pair.first->internal_name, pair.first, pair.first->icon, false, pair.first->descname());
	}
	tabs_.tabs()[0]->set_title((boost::format(_("Installed (%u)")) % index).str());

	index = 0;
	std::list<std::shared_ptr<AddOns::AddOnInfo>> remotes_to_show;
	for (auto& a : remotes_) {
		if (matches_filter(a)) {
			remotes_to_show.push_back(a);
		}
	}
	{
		const AddOnSortingCriteria sort_by = sort_order_.get_selected();
		remotes_to_show.sort([sort_by](const std::shared_ptr<AddOns::AddOnInfo> a,
		                               const std::shared_ptr<AddOns::AddOnInfo> b) {
			switch (sort_by) {
			case AddOnSortingCriteria::kNameABC:
				return a->descname().compare(b->descname()) < 0;
			case AddOnSortingCriteria::kNameCBA:
				return a->descname().compare(b->descname()) > 0;

			case AddOnSortingCriteria::kFewestDownloads:
				return a->download_count < b->download_count;
			case AddOnSortingCriteria::kMostDownloads:
				return a->download_count > b->download_count;

			case AddOnSortingCriteria::kOldest:
				return a->upload_timestamp < b->upload_timestamp;
			case AddOnSortingCriteria::kNewest:
				return a->upload_timestamp > b->upload_timestamp;

			case AddOnSortingCriteria::kLowestRating:
				if (a->number_of_votes() == 0) {
					// Add-ons without votes should always end up
					// below any others when sorting by rating
					return false;
				} else if (b->number_of_votes() == 0) {
					return true;
				} else if (std::abs(a->average_rating() - b->average_rating()) < 0.01) {
					// ambiguity – always choose the one with more votes
					return a->number_of_votes() > b->number_of_votes();
				} else {
					return a->average_rating() < b->average_rating();
				}
			case AddOnSortingCriteria::kHighestRating:
				if (a->number_of_votes() == 0) {
					return false;
				} else if (b->number_of_votes() == 0) {
					return true;
				} else if (std::abs(a->average_rating() - b->average_rating()) < 0.01) {
					return a->number_of_votes() > b->number_of_votes();
				} else {
					return a->average_rating() > b->average_rating();
				}
			}
			NEVER_HERE();
		});
	}
	std::vector<std::string> has_upgrades;
	for (const auto& a : remotes_to_show) {
		if (0 < index++) {
			browse_addons_box_.add_space(kRowButtonSize);
		}
		AddOns::AddOnVersion installed;
		uint32_t installed_i18n = 0;
		for (const auto& pair : AddOns::g_addons) {
			if (pair.first->internal_name == a->internal_name) {
				installed = pair.first->version;
				installed_i18n = pair.first->i18n_version;
				break;
			}
		}
		RemoteAddOnRow* r =
		   new RemoteAddOnRow(&browse_addons_box_, this, a, installed, installed_i18n);
		browse_addons_box_.add(r, UI::Box::Resizing::kFullSize);
		if (r->upgradeable()) {
			has_upgrades.push_back(a->descname());
		}
		browse_.push_back(r);
	}
	tabs_.tabs()[1]->set_title(index == 0 ? _("Browse") :
                                           (boost::format(_("Browse (%u)")) % index).str());

	if (installed_addons_inner_wrapper_.get_scrollbar() && scrollpos_i) {
		installed_addons_inner_wrapper_.get_scrollbar()->set_scrollpos(scrollpos_i);
	}
	if (browse_addons_inner_wrapper_.get_scrollbar() && scrollpos_b) {
		browse_addons_inner_wrapper_.get_scrollbar()->set_scrollpos(scrollpos_b);
	}

	check_enable_move_buttons();
	upgrade_all_.set_title((boost::format(_("Upgrade all (%u)")) % has_upgrades.size()).str());
	upgrade_all_.set_enabled(!has_upgrades.empty());
	if (has_upgrades.empty()) {
		upgrade_all_.set_tooltip(_("No upgrades are available for your installed add-ons"));
	} else {
		std::string text =
		   (boost::format(ngettext("Upgrade the following %u add-on:",
		                           "Upgrade the following %u add-ons:", has_upgrades.size())) %
		    has_upgrades.size())
		      .str();
		for (const std::string& name : has_upgrades) {
			text += "<br>";
			text += (boost::format(_("· %s")) % name).str();
		}
		upgrade_all_.set_tooltip(text);
	}

	update_dependency_errors();

	initialization_complete();
}

void AddOnsCtrl::update_dependency_errors() {
	std::vector<std::string> warn_requirements;
	for (auto addon = AddOns::g_addons.begin(); addon != AddOns::g_addons.end(); ++addon) {
		if (!addon->second) {
			// Disabled, so we don't care about dependencies
			continue;
		}
		for (const std::string& requirement : addon->first->requirements) {
			auto search_result = AddOns::g_addons.end();
			bool too_late = false;
			for (auto search = AddOns::g_addons.begin(); search != AddOns::g_addons.end(); ++search) {
				if (search->first->internal_name == requirement) {
					search_result = search;
					break;
				}
				if (search == addon) {
					assert(!too_late);
					too_late = true;
				}
			}
			if (search_result == AddOns::g_addons.end()) {
				warn_requirements.push_back(
				   (boost::format(_("· ‘%1$s’ requires ‘%2$s’ which could not be found")) %
				    addon->first->descname() % requirement)
				      .str());
			} else {
				if (!search_result->second) {
					warn_requirements.push_back(
					   (boost::format(_("· ‘%1$s’ requires ‘%2$s’ which is disabled")) %
					    addon->first->descname() % search_result->first->descname())
					      .str());
				}
				if (too_late) {
					warn_requirements.push_back(
					   (boost::format(
					       _("· ‘%1$s’ requires ‘%2$s’ which is listed below the requiring add-on")) %
					    addon->first->descname() % search_result->first->descname())
					      .str());
				}
			}
			// Also warn if the add-on's requirements are present in the wrong order
			// (e.g. when A requires B,C but they are ordered C,B,A)
			for (const std::string& previous_requirement : addon->first->requirements) {
				if (previous_requirement == requirement) {
					break;
				}
				// check if `previous_requirement` comes before `requirement`
				std::string prev_descname;
				for (const AddOns::AddOnState& a : AddOns::g_addons) {
					if (a.first->internal_name == previous_requirement) {
						prev_descname = a.first->descname();
						break;
					} else if (a.first->internal_name == requirement) {
						warn_requirements.push_back(
						   (boost::format(
						       _("· ‘%1$s’ requires first ‘%2$s’ and then ‘%3$s’, but they are "
						         "listed in the wrong order")) %
						    addon->first->descname() % prev_descname % search_result->first->descname())
						      .str());
						break;
					}
				}
			}
		}
	}
	if (warn_requirements.empty()) {
		warn_requirements_.set_text("");
		warn_requirements_.set_tooltip("");
		try {
			Widelands::Game game;
			game.descriptions();
		} catch (const std::exception& e) {
			warn_requirements_.set_text(
			   (boost::format(_("An enabled add-on is defective. No games can be started with the "
			                    "current configuration.\nError message:\n%s")) %
			    e.what())
			      .str());
		}
	} else {
		const unsigned nr_warnings = warn_requirements.size();
		std::string list;
		for (const std::string& msg : warn_requirements) {
			if (!list.empty()) {
				list += "<br>";
			}
			list += msg;
		}
		warn_requirements_.set_text(
		   (boost::format("<rt><p>%s</p><p>%s</p></rt>") %
		    g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		       .as_font_tag((boost::format(ngettext(
		                        "%u Dependency Error", "%u Dependency Errors", nr_warnings)) %
		                     nr_warnings)
		                       .str()) %
		    g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(list))
		      .str());
		warn_requirements_.set_tooltip(_("Add-Ons with dependency errors may work incorrectly or "
		                                 "prevent games and maps from loading."));
	}
	// TODO(Nordfriese): Disabled autofix_dependencies for v1.0
	// autofix_dependencies_.set_enabled(!warn_requirements.empty());
	layout();
}

void AddOnsCtrl::layout() {
	if (!is_minimal()) {
		main_box_.set_size(get_inner_w(), get_inner_h());

		warn_requirements_.set_visible(!warn_requirements_.get_text().empty());

		// Box layouting does not work well together with this scrolling tab panel, so we
		// use a plain Panel as a fixed-size placeholder which is layouted by the box and
		// we manually position and resize the real tab panel on top of the placeholder.
		tabs_.set_pos(Vector2i(tabs_placeholder_.get_x(), tabs_placeholder_.get_y()));
		tabs_.set_size(tabs_placeholder_.get_w(), tabs_placeholder_.get_h());

		installed_addons_outer_wrapper_.set_max_size(
		   tabs_placeholder_.get_w(), tabs_placeholder_.get_h() - 2 * kRowButtonSize);
		browse_addons_inner_wrapper_.set_max_size(
		   tabs_placeholder_.get_w(),
		   tabs_placeholder_.get_h() - 2 * kRowButtonSize - browse_addons_buttons_box_.get_h());

		login_button_.set_size(get_inner_w() / 3, login_button_.get_h());
		login_button_.set_pos(Vector2i(get_inner_w() - login_button_.get_w(), 0));
	}

	UI::Window::layout();
}

std::shared_ptr<AddOns::AddOnInfo> AddOnsCtrl::find_remote(const std::string& name) {
	for (auto& r : remotes_) {
		if (r->internal_name == name) {
			return r;
		}
	}
	return nullptr;
}

bool AddOnsCtrl::is_remote(const std::string& name) const {
	if (remotes_.size() <= 1) {
		// No data available
		return true;
	}
	for (const auto& r : remotes_) {
		if (r->internal_name == name) {
			return true;
		}
	}
	return false;
}

static void install_translation(const std::string& temp_locale_path,
                                const std::string& addon_name,
                                const int i18n_version) {
	assert(g_fs->file_exists(temp_locale_path));

	// NOTE:
	// gettext expects a directory structure such as
	// "~/.widelands/addons_i18n/nds/LC_MESSAGES/addon_name.wad.VERSION.mo"
	// where "nds" is the language abbreviation, VERSION the add-on's i18n version,
	// and "addon_name.wad" the add-on's name.
	// If we use a different structure, gettext will not find the translations!

	const std::string temp_filename =
	   FileSystem::fs_filename(temp_locale_path.c_str());                         // nds.mo.tmp
	const std::string locale = temp_filename.substr(0, temp_filename.find('.'));  // nds

	const std::string new_locale_dir = kAddOnLocaleDir + FileSystem::file_separator() + locale +
	                                   FileSystem::file_separator() +
	                                   "LC_MESSAGES";  // addons_i18n/nds/LC_MESSAGES
	g_fs->ensure_directory_exists(new_locale_dir);

	const std::string new_locale_path = new_locale_dir + FileSystem::file_separator() + addon_name +
	                                    '.' + std::to_string(i18n_version) + ".mo";

	assert(!g_fs->is_directory(new_locale_path));
	// Delete outdated translations if present.
	for (const std::string& mo : g_fs->list_directory(new_locale_dir)) {
		if (strncmp(FileSystem::fs_filename(mo.c_str()), addon_name.c_str(), addon_name.size()) ==
		    0) {
			g_fs->fs_unlink(mo);
		}
	}
	assert(!g_fs->file_exists(new_locale_path));

	// move translation file from temp location to the correct place
	g_fs->fs_rename(temp_locale_path, new_locale_path);

	assert(g_fs->file_exists(new_locale_path));
	assert(!g_fs->file_exists(temp_locale_path));
}

void AddOnsCtrl::upload_addon(std::shared_ptr<AddOns::AddOnInfo> addon) {
	upload_addon_.set_list_visibility(false);
	{
		UI::WLMessageBox w(
		   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Upload"),
		   (boost::format(_("Do you really want to upload the add-on ‘%s’ to the server?")) %
		    addon->internal_name)
		      .str(),
		   UI::WLMessageBox::MBoxType::kOkCancel);
		if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
	}
	ProgressIndicatorWindow w(&get_topmost_forefather(), addon->descname());
	w.set_message_1((boost::format(_("Uploading ‘%s’…")) % addon->descname()).str());
	try {
		int64_t nr_files = 0;
		net().upload_addon(
		   addon->internal_name,
		   [this, &w, &nr_files](const std::string& f, const int64_t l) {
			   w.set_message_2(f);
			   w.set_message_3((boost::format(_("%1% / %2%")) % l % nr_files).str());
			   w.progressbar().set_state(l);
			   do_redraw_now();
			   if (w.is_dying()) {
				   throw OperationCancelledByUserException();
			   }
		   },
		   [&w, &nr_files](const std::string&, const int64_t l) {
			   w.progressbar().set_total(l);
			   nr_files = l;
		   });
		std::shared_ptr<AddOns::AddOnInfo> r = find_remote(addon->internal_name);
		if (r.get() != nullptr) {
			*r = net().fetch_one_remote(r->internal_name);
		}
		rebuild();
	} catch (const OperationCancelledByUserException&) {
		log_info("upload addon %s cancelled by user", addon->internal_name.c_str());
	} catch (const std::exception& e) {
		log_err("upload addon %s: %s", addon->internal_name.c_str(), e.what());
		w.set_visible(false);
		UI::WLMessageBox m(
		   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
		   (boost::format(
		       _("The add-on ‘%1$s’ could not be uploaded to the server.\n\nError Message:\n%2$s")) %
		    addon->internal_name % e.what())
		      .str(),
		   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
	}
}

// TODO(Nordfriese): install_or_upgrade() should also (recursively) install the add-on's
// requirements
void AddOnsCtrl::install_or_upgrade(std::shared_ptr<AddOns::AddOnInfo> remote,
                                    const bool only_translations) {
	ProgressIndicatorWindow w(&get_topmost_forefather(), remote->descname());
	w.set_message_1((boost::format(_("Downloading ‘%s’…")) % remote->descname()).str());

	std::string temp_dir =
	   kTempFileDir + FileSystem::file_separator() + remote->internal_name + kTempFileExtension;
	if (g_fs->file_exists(temp_dir)) {
		g_fs->fs_unlink(temp_dir);
	}
	g_fs->ensure_directory_exists(kAddOnDir);

	bool need_to_rebuild_texture_atlas = false, enable_theme = false;
	if (!only_translations) {
		bool success = false;
		g_fs->ensure_directory_exists(temp_dir);
		try {
			const std::string size = filesize_string(remote->total_file_size);
			w.progressbar().set_total(remote->total_file_size);
			net().download_addon(
			   remote->internal_name, temp_dir,
			   [this, &w, size](const std::string& f, const int64_t l) {
				   w.set_message_2(f);
				   w.set_message_3((boost::format(_("%1% / %2%")) % filesize_string(l) % size).str());
				   w.progressbar().set_state(l);
				   do_redraw_now();
				   if (w.is_dying()) {
					   throw OperationCancelledByUserException();
				   }
			   });
			success = true;
		} catch (const OperationCancelledByUserException&) {
			log_info("install addon %s cancelled by user", remote->internal_name.c_str());
		} catch (const std::exception& e) {
			log_err("install addon %s: %s", remote->internal_name.c_str(), e.what());
			w.set_visible(false);
			UI::WLMessageBox m(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
			   (boost::format(
			       _("The add-on ‘%1$s’ could not be downloaded from the server. Installing/upgrading "
			         "this add-on will be skipped.\n\nError Message:\n%2$s")) %
			    remote->internal_name % e.what())
			      .str(),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
		}
		if (!success) {
			g_fs->fs_unlink(temp_dir);
			return;
		}

		const std::string new_path = kAddOnDir + FileSystem::file_separator() + remote->internal_name;
		if (g_fs->file_exists(new_path)) {
			g_fs->fs_unlink(new_path);
		}
		g_fs->fs_rename(temp_dir, new_path);

		need_to_rebuild_texture_atlas = remote->requires_texture_atlas_rebuild();
		bool found = false;
		for (auto& pair : AddOns::g_addons) {
			if (pair.first->internal_name == remote->internal_name) {
				pair.first = AddOns::preload_addon(remote->internal_name);
				enable_theme =
				   (remote->category == AddOns::AddOnCategory::kTheme &&
				    template_dir() == AddOns::theme_addon_template_dir(remote->internal_name));
				found = true;
				break;
			}
		}
		if (!found) {
			enable_theme = (remote->category == AddOns::AddOnCategory::kTheme);
			AddOns::g_addons.push_back(
			   std::make_pair(AddOns::preload_addon(remote->internal_name), true));
		}
	}

	assert(!g_fs->file_exists(temp_dir));
	g_fs->ensure_directory_exists(temp_dir);
	try {
		w.progressbar().set_state(0);
		w.progressbar().set_total(1);
		int64_t nr_translations = 0;
		w.set_message_3("");
		net().download_i18n(
		   remote->internal_name, temp_dir,
		   [this, &w, &nr_translations](const std::string& f, const int64_t l) {
			   w.set_message_2(f);
			   w.set_message_3((boost::format(_("%1% / %2%")) % l % nr_translations).str());
			   w.progressbar().set_state(l);
			   do_redraw_now();
			   if (w.is_dying()) {
				   throw OperationCancelledByUserException();
			   }
		   },
		   [&w, &nr_translations](const std::string&, const int64_t l) {
			   nr_translations = l;
			   w.progressbar().set_total(std::max<int64_t>(l, 1));
		   });

		for (const std::string& n : g_fs->list_directory(temp_dir)) {
			install_translation(n, remote->internal_name, remote->i18n_version);
		}
		for (auto& pair : AddOns::g_addons) {
			if (pair.first->internal_name == remote->internal_name) {
				pair.first->i18n_version = remote->i18n_version;
				break;
			}
		}
		Profile prof(kAddOnLocaleVersions.c_str());
		prof.pull_section("global").set_natural(remote->internal_name.c_str(), remote->i18n_version);
		prof.write(kAddOnLocaleVersions.c_str(), false);
	} catch (const OperationCancelledByUserException&) {
		log_info("install translations for %s cancelled by user", remote->internal_name.c_str());
	} catch (const std::exception& e) {
		log_err("install translations for %s: %s", remote->internal_name.c_str(), e.what());
		w.set_visible(false);
		UI::WLMessageBox m(
		   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
		   (boost::format(_("The translations for the add-on ‘%1$s’ could not be downloaded from the "
		                    "server. Installing/upgrading "
		                    "the translations will be skipped.\n\nError Message:\n%2$s")) %
		    remote->internal_name % e.what())
		      .str(),
		   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
	}
	g_fs->fs_unlink(temp_dir);

	if (need_to_rebuild_texture_atlas) {
		g_gr->rebuild_texture_atlas();
	}
	if (enable_theme) {
		AddOns::update_ui_theme(AddOns::UpdateThemeAction::kEnableArgument, remote->internal_name);
		get_topmost_forefather().template_directory_changed();
	}
	rebuild();
}

static inline std::string safe_richtext_message(std::string body) {
	newlines_to_richtext(body);
	return as_richtext_paragraph(body, UI::FontStyle::kFsMenuLabel, UI::Align::kCenter);
}

static void uninstall(AddOnsCtrl* ctrl, std::shared_ptr<AddOns::AddOnInfo> info, const bool local) {
	if (!(SDL_GetModState() & KMOD_CTRL)) {
		UI::WLMessageBox w(
		   &ctrl->get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Uninstall"),
		   safe_richtext_message(
		      (boost::format(
		          local ? _("Are you certain that you want to uninstall this add-on?\n\n"
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
		                    "%5$s")) %
		       info->descname() % info->author() % AddOns::version_to_string(info->version) %
		       AddOns::kAddOnCategories.at(info->category).descname() % info->description())
		         .str()),
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
			return ctrl->rebuild();
		}
	}
	NEVER_HERE();
}

#if 0  // TODO(Nordfriese): Disabled autofix_dependencies for v1.0
// UNTESTED
// Automatically fix all dependency errors by reordering add-ons and downloading missing ones.
// We make no guarantees inhowfar the existing order is preserved
// (e.g. if A currently comes before B, it may come after B after reordering even if
// there is no direct or indirect dependency relation between A and B).
void AddOnsCtrl::autofix_dependencies() {
	std::set<std::string> missing_requirements;

// Step 1: Enable all dependencies
step1:
	for (const AddOns::AddOnState& addon_to_fix : AddOns::g_addons) {
		if (addon_to_fix.second) {
			bool anything_changed = false;
			bool found = false;
			for (const std::string& requirement : addon_to_fix.first->requirements) {
				for (AddOns::AddOnState& a : AddOns::g_addons) {
					if (a.first->internal_name == requirement) {
						found = true;
						if (!a.second) {
							a.second = true;
							anything_changed = true;
						}
						break;
					}
				}
				if (!found) {
					missing_requirements.insert(requirement);
				}
			}
			if (anything_changed) {
				// concurrent modification – we need to start over
				goto step1;
			}
		}
	}

	// Step 2: Download missing add-ons
	for (const std::string& addon_to_install : missing_requirements) {
		bool found = false;
		for (const auto& info : remotes_) {
			if (info->internal_name == addon_to_install) {
				install_or_upgrade(info, false);
				found = true;
				break;
			}
		}
		if (!found) {
			UI::WLMessageBox w(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
			   (boost::format(_("The required add-on ‘%s’ could not be found on the server.")) %
			    addon_to_install)
			      .str(),
			   UI::WLMessageBox::MBoxType::kOk);
			w.run<UI::Panel::Returncodes>();
		}
	}

	// Step 3: Get all add-ons into the correct order
	std::map<std::string, AddOns::AddOnState> all_addons;

	for (const AddOns::AddOnState& aos : AddOns::g_addons) {
		all_addons[aos.first->internal_name] = aos;
	}

	std::multimap<unsigned /* number of dependencies */, AddOns::AddOnState> addons_tree;
	for (const auto& pair : all_addons) {
		addons_tree.emplace(
		   std::make_pair(count_all_dependencies(pair.first, all_addons), pair.second));
	}
	// The addons_tree now contains a list of all add-ons sorted by number
	// of (direct plus indirect) dependencies
	AddOns::g_addons.clear();
	for (const auto& pair : addons_tree) {
		AddOns::g_addons.push_back(AddOns::AddOnState(pair.second));
	}

	rebuild();
}
#endif

static std::string
required_wl_version_and_sync_safety_string(std::shared_ptr<AddOns::AddOnInfo> info) {
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
			str += (boost::format(_("Requires a Widelands version of at least %s.")) %
			        info->min_wl_version)
			          .str();
		} else if (info->min_wl_version.empty()) {
			str +=
			   (boost::format(_("Requires a Widelands version of at most %s.")) % info->max_wl_version)
			      .str();
		} else {
			str +=
			   (boost::format(_("Requires a Widelands version of at least %1$s and at most %2$s.")) %
			    info->min_wl_version % info->max_wl_version)
			      .str();
		}
		result += g_style_manager
		             ->font_style(info->matches_widelands_version() ? UI::FontStyle::kItalic :
                                                                    UI::FontStyle::kWarning)
		             .as_font_tag(str);
	}
	return result;
}

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
              (boost::format(_("%1$s+%2$u")) % AddOns::version_to_string(info->version) %
               info->i18n_version)
                 .str(),
              UI::Align::kCenter),
     txt_(this,
          0,
          0,
          24,
          24,
          UI::PanelStyle::kFsMenu,
          (boost::format("<rt><p>%s</p><p>%s%s</p><p>%s</p></rt>") %
           (boost::format(
               /** TRANSLATORS: Add-On localized name as header (Add-On internal name in italics) */
               _("%1$s %2$s")) %
            g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
               .as_font_tag(info->descname()) %
            g_style_manager->font_style(UI::FontStyle::kItalic)
               .as_font_tag((boost::format(_("(%s)")) % info->internal_name).str()))
              .str() %
           g_style_manager->font_style(UI::FontStyle::kItalic)
              .as_font_tag((boost::format(_("by %s")) % info->author()).str()) %
           required_wl_version_and_sync_safety_string(info) %
           g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
              .as_font_tag(info->description()))
             .str()) {

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
					ctrl->rebuild();
				}
				return ctrl->update_dependency_errors();
			}
		}
		NEVER_HERE();
	});
	category_.set_handle_mouse(true);
	category_.set_tooltip(
	   (boost::format(_("Category: %s")) % AddOns::kAddOnCategories.at(info->category).descname())
	      .str());
	version_.set_handle_mouse(true);
	version_.set_tooltip(
	   /** TRANSLATORS: (MajorVersion)+(MinorVersion) */
	   (boost::format(_("Version: %1$s+%2$u")) % AddOns::version_to_string(info->version) %
	    info->i18n_version)
	      .str());
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

class RemoteInteractionWindow : public UI::Window {
public:
	RemoteInteractionWindow(AddOnsCtrl& parent, std::shared_ptr<AddOns::AddOnInfo> info)
	   : UI::Window(parent.get_parent(),
	                UI::WindowStyle::kFsMenu,
	                info->internal_name,
	                parent.get_x() + kRowButtonSize,
	                parent.get_y() + kRowButtonSize,
	                parent.get_inner_w() - 2 * kRowButtonSize,
	                parent.get_inner_h() - 2 * kRowButtonSize,
	                info->descname()),
	     parent_(parent),
	     info_(info),
	     current_screenshot_(0),
	     nr_screenshots_(info->screenshots.size()),
	     current_vote_(-1),

	     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     tabs_(&main_box_, UI::TabPanelStyle::kFsMenu),
	     box_comments_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     box_comment_rows_(&box_comments_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     box_screenies_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     box_screenies_buttons_(&box_screenies_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     box_votes_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     voting_stats_(&box_votes_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
	     box_comment_rows_placeholder_(&box_comments_, UI::PanelStyle::kFsMenu, 0, 0, 0, 0),
	     comments_header_(&box_comments_,
	                      0,
	                      0,
	                      0,
	                      0,
	                      UI::PanelStyle::kFsMenu,
	                      "",
	                      UI::Align::kLeft,
	                      UI::MultilineTextarea::ScrollMode::kNoScrolling),
	     screenshot_(&box_screenies_, UI::PanelStyle::kFsMenu, 0, 0, 0, 0, nullptr),
	     own_voting_(&box_votes_,
	                 "voting",
	                 0,
	                 0,
	                 0,
	                 11,
	                 kRowButtonSize - kRowButtonSpacing,
	                 _("Your vote"),
	                 UI::DropdownType::kTextual,
	                 UI::PanelStyle::kFsMenu,
	                 UI::ButtonStyle::kFsMenuSecondary),
	     screenshot_stats_(&box_screenies_buttons_,
	                       UI::PanelStyle::kFsMenu,
	                       UI::FontStyle::kFsMenuLabel,
	                       "",
	                       UI::Align::kCenter),
	     screenshot_descr_(&box_screenies_,
	                       UI::PanelStyle::kFsMenu,
	                       UI::FontStyle::kFsMenuLabel,
	                       "",
	                       UI::Align::kCenter),
	     voting_stats_summary_(&box_votes_,
	                           UI::PanelStyle::kFsMenu,
	                           UI::FontStyle::kFsMenuLabel,
	                           "",
	                           UI::Align::kCenter),
	     screenshot_next_(&box_screenies_buttons_,
	                      "next_screenshot",
	                      0,
	                      0,
	                      48,
	                      24,
	                      UI::ButtonStyle::kFsMenuSecondary,
	                      g_image_cache->get("images/ui_basic/scrollbar_right.png"),
	                      _("Next screenshot")),
	     screenshot_prev_(&box_screenies_buttons_,
	                      "prev_screenshot",
	                      0,
	                      0,
	                      48,
	                      24,
	                      UI::ButtonStyle::kFsMenuSecondary,
	                      g_image_cache->get("images/ui_basic/scrollbar_left.png"),
	                      _("Previous screenshot")),
	     write_comment_(&box_comments_,
	                    "write_comment",
	                    0,
	                    0,
	                    0,
	                    0,
	                    UI::ButtonStyle::kFsMenuSecondary,
	                    _("Write a comment…")),
	     ok_(&main_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")),
	     login_button_(this, "login", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, "") {

		ok_.sigclicked.connect([this]() { end_modal(UI::Panel::Returncodes::kBack); });

		own_voting_.add(_("Not voted"), 0, nullptr, true);
		for (unsigned i = 1; i <= 10; ++i) {
			own_voting_.add(std::to_string(i), i);
		}
		own_voting_.selected.connect([this]() {
			current_vote_ = own_voting_.get_selected();
			try {
				parent_.net().vote(info_->internal_name, current_vote_);
				*info_ = parent_.net().fetch_one_remote(info_->internal_name);
			} catch (const std::exception& e) {
				UI::WLMessageBox w(
				   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
				   (boost::format(_("The vote could not be submitted.\nError code: %s")) % e.what())
				      .str(),
				   UI::WLMessageBox::MBoxType::kOk);
				w.run<UI::Panel::Returncodes>();
				return;
			}
			update_data();
			parent_.rebuild();
		});
		write_comment_.sigclicked.connect([this]() {
			CommentEditor m(parent_, info_, -1);
			if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
				update_data();
				parent_.rebuild();
			}
		});

		box_screenies_buttons_.add(&screenshot_prev_, UI::Box::Resizing::kFullSize);
		box_screenies_buttons_.add(&screenshot_stats_, UI::Box::Resizing::kExpandBoth);
		box_screenies_buttons_.add(&screenshot_next_, UI::Box::Resizing::kFullSize);

		box_screenies_.add_space(kRowButtonSpacing);
		box_screenies_.add(&box_screenies_buttons_, UI::Box::Resizing::kFullSize);
		box_screenies_.add_space(kRowButtonSpacing);
		box_screenies_.add(&screenshot_, UI::Box::Resizing::kExpandBoth);
		box_screenies_.add_space(kRowButtonSpacing);
		box_screenies_.add(&screenshot_descr_, UI::Box::Resizing::kFullSize);

		box_comment_rows_.set_force_scrolling(true);
		box_comments_.add(&comments_header_, UI::Box::Resizing::kFullSize);
		box_comments_.add_space(kRowButtonSpacing);
		box_comments_.add(&box_comment_rows_placeholder_, UI::Box::Resizing::kExpandBoth);
		box_comments_.add_space(kRowButtonSpacing);
		box_comments_.add(&write_comment_, UI::Box::Resizing::kFullSize);

		voting_stats_.add_inf_space();
		for (unsigned i = 0; i < AddOns::kMaxRating; ++i) {
			UI::Box* box =
			   new UI::Box(&voting_stats_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical);
			voting_bars_[i] =
			   new UI::ProgressBar(box, UI::PanelStyle::kFsMenu, 0, 0, kRowButtonSize * 3 / 2, 0,
			                       UI::ProgressBar::Vertical);
			voting_bars_[i]->set_show_percent(false);
			voting_txt_[i] = new UI::Textarea(
			   box, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, "", UI::Align::kCenter);

			box->add(voting_bars_[i], UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
			box->add_space(kRowButtonSpacing);
			box->add(voting_txt_[i], UI::Box::Resizing::kAlign, UI::Align::kCenter);
			voting_stats_.add(box, UI::Box::Resizing::kExpandBoth);
			voting_stats_.add_inf_space();
		}

		box_votes_.add(&voting_stats_summary_, UI::Box::Resizing::kFullSize);
		box_votes_.add_space(kRowButtonSpacing);
		box_votes_.add(&voting_stats_, UI::Box::Resizing::kExpandBoth);
		box_votes_.add_space(kRowButtonSpacing);
		box_votes_.add(&own_voting_, UI::Box::Resizing::kFullSize);
		box_votes_.add_space(kRowButtonSpacing);

		tabs_.add("comments", "", &box_comments_);
		if (nr_screenshots_) {
			tabs_.add("screenshots",
			          (boost::format(_("Screenshots (%u)")) % info_->screenshots.size()).str(),
			          &box_screenies_);
			tabs_.sigclicked.connect([this]() {
				if (tabs_.active() == 1) {
					next_screenshot(0);
				}
			});
		} else {
			box_screenies_.set_visible(false);
		}
		tabs_.add("votes", "", &box_votes_);

		main_box_.add(&tabs_, UI::Box::Resizing::kExpandBoth);
		main_box_.add_space(kRowButtonSpacing);
		main_box_.add(&ok_, UI::Box::Resizing::kFullSize);

		screenshot_next_.set_enabled(nr_screenshots_ > 1);
		screenshot_prev_.set_enabled(nr_screenshots_ > 1);
		screenshot_cache_.resize(nr_screenshots_, nullptr);
		screenshot_next_.sigclicked.connect([this]() { next_screenshot(1); });
		screenshot_prev_.sigclicked.connect([this]() { next_screenshot(-1); });

		login_button_.sigclicked.connect([this]() {
			parent_.login_button_clicked();
			parent_.update_login_button(&login_button_);
			login_changed();
		});
		parent_.update_login_button(&login_button_);
		login_changed();

		update_data();
		layout();

		initialization_complete();
	}

	void on_resolution_changed_note(const GraphicResolutionChanged& note) override {
		UI::Window::on_resolution_changed_note(note);

		set_size(
		   parent_.get_inner_w() - 2 * kRowButtonSize, parent_.get_inner_h() - 2 * kRowButtonSize);
		set_pos(Vector2i(parent_.get_x() + kRowButtonSize, parent_.get_y() + kRowButtonSize));
		main_box_.set_size(get_inner_w(), get_inner_h());
	}

	void layout() override {
		if (!is_minimal()) {
			main_box_.set_size(get_inner_w(), get_inner_h());
			login_button_.set_size(get_inner_w() / 3, login_button_.get_h());
			login_button_.set_pos(Vector2i(get_inner_w() - login_button_.get_w(), 0));
			box_comment_rows_.set_pos(box_comment_rows_placeholder_.get_pos());
			box_comment_rows_.set_size(
			   box_comment_rows_placeholder_.get_w(), box_comment_rows_placeholder_.get_h());
		}
		UI::Window::layout();
	}

	void update_data() {
		(*tabs_.tabs().begin())
		   ->set_title((boost::format(_("Comments (%u)")) % info_->user_comments.size()).str());
		(*tabs_.tabs().rbegin())
		   ->set_title((boost::format(_("Votes (%u)")) % info_->number_of_votes()).str());

		voting_stats_summary_.set_text(
		   info_->number_of_votes() ? (boost::format(ngettext("Average rating: %1$.3f (%2$u vote)",
		                                                      "Average rating: %1$.3f (%2$u votes)",
		                                                      info_->number_of_votes())) %
		                               info_->average_rating() % info_->number_of_votes())
		                                 .str() :
                                    _("No votes yet"));

		uint32_t most_votes = 1;
		for (uint32_t v : info_->votes) {
			most_votes = std::max(most_votes, v);
		}
		for (unsigned i = 0; i < AddOns::kMaxRating; ++i) {
			voting_bars_[i]->set_total(most_votes);
			voting_bars_[i]->set_state(info_->votes[i]);
			voting_txt_[i]->set_text(std::to_string(i + 1));
		}

		box_comment_rows_.clear();
		comment_rows_.clear();
		std::string text = "<rt><p>";
		text += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		           .as_font_tag(info_->user_comments.empty() ?
                                 _("No comments yet.") :
                                 (boost::format(ngettext(
		                               "%u comment:", "%u comments:", info_->user_comments.size())) %
		                            info_->user_comments.size())
		                              .str());
		text += "</p></rt>";
		comments_header_.set_text(text);
		int64_t index = 0;
		for (const auto& comment : info_->user_comments) {
			text = "<rt><p>";
			if (comment.editor.empty()) {
				text += g_style_manager->font_style(UI::FontStyle::kItalic)
				           .as_font_tag(time_string(comment.timestamp));
			} else if (comment.editor == comment.username) {
				text += g_style_manager->font_style(UI::FontStyle::kItalic)
				           .as_font_tag((boost::format(_("%1$s (edited on %2$s)")) %
				                         time_string(comment.timestamp) %
				                         time_string(comment.edit_timestamp))
				                           .str());
			} else {
				text += g_style_manager->font_style(UI::FontStyle::kItalic)
				           .as_font_tag((boost::format(_("%1$s (edited by ‘%2$s’ on %3$s)")) %
				                         time_string(comment.timestamp) % comment.editor %
				                         time_string(comment.edit_timestamp))
				                           .str());
			}
			text += "<br>";
			text += g_style_manager->font_style(UI::FontStyle::kItalic)
			           .as_font_tag((boost::format(_("‘%1$s’ commented on version %2$s:")) %
			                         comment.username % AddOns::version_to_string(comment.version))
			                           .str());
			text += "<br>";
			text += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			           .as_font_tag(comment.message);
			text += "</p></rt>";

			CommentRow* cr = new CommentRow(parent_, info_, *this, box_comment_rows_, text, index);
			comment_rows_.push_back(std::unique_ptr<CommentRow>(cr));
			box_comment_rows_.add_space(kRowButtonSize);
			box_comment_rows_.add(cr, UI::Box::Resizing::kFullSize);
			++index;
		}
		// layout();
	}

private:
	static std::map<std::pair<std::string /* add-on */, std::string /* screenshot */>,
	                std::string /* image path */>
	   downloaded_screenshots_cache_;

	void next_screenshot(int8_t delta) {
		assert(nr_screenshots_ > 0);
		while (delta < 0) {
			delta += nr_screenshots_;
		}
		current_screenshot_ = (current_screenshot_ + delta) % nr_screenshots_;
		assert(current_screenshot_ < static_cast<int32_t>(screenshot_cache_.size()));

		auto it = info_->screenshots.begin();
		std::advance(it, current_screenshot_);

		screenshot_stats_.set_text(
		   (boost::format(_("%1$u / %2$u")) % (current_screenshot_ + 1) % nr_screenshots_).str());
		screenshot_descr_.set_text(it->second);
		screenshot_.set_tooltip("");

		if (screenshot_cache_[current_screenshot_]) {
			screenshot_.set_icon(screenshot_cache_[current_screenshot_]);
			return;
		}

		const Image* image = nullptr;
		const std::pair<std::string, std::string> cache_key(info_->internal_name, it->first);
		auto cached = downloaded_screenshots_cache_.find(cache_key);
		if (cached == downloaded_screenshots_cache_.end()) {
			const std::string screenie =
			   parent_.net().download_screenshot(cache_key.first, cache_key.second);
			try {
				if (!screenie.empty()) {
					image = g_image_cache->get(screenie);
				}
				downloaded_screenshots_cache_[cache_key] = screenie;
			} catch (const std::exception& e) {
				log_err("Error downloading screenshot %s for %s: %s", it->first.c_str(),
				        info_->internal_name.c_str(), e.what());
				image = nullptr;
			}
		} else if (!cached->second.empty()) {
			image = g_image_cache->get(cached->second);
		}

		if (image) {
			screenshot_.set_icon(image);
			screenshot_cache_[current_screenshot_] = image;
		} else {
			screenshot_.set_icon(g_image_cache->get("images/ui_basic/stop.png"));
			screenshot_.set_handle_mouse(true);
			screenshot_.set_tooltip(
			   _("This screenshot could not be fetched from the server due to an error."));
		}
	}

	void login_changed() {
		current_vote_ = parent_.net().get_vote(info_->internal_name);
		if (current_vote_ < 0) {
			write_comment_.set_enabled(false);
			write_comment_.set_tooltip(_("Please log in to comment"));
			own_voting_.set_enabled(false);
			own_voting_.set_tooltip(_("Please log in to vote"));
			own_voting_.select(0);
		} else {
			write_comment_.set_enabled(true);
			write_comment_.set_tooltip("");
			own_voting_.set_enabled(true);
			own_voting_.set_tooltip("");
			own_voting_.select(current_vote_);
		}
		for (auto& cr : comment_rows_) {
			cr->update_edit_enabled();
		}
	}

	AddOnsCtrl& parent_;
	std::shared_ptr<AddOns::AddOnInfo> info_;
	int32_t current_screenshot_, nr_screenshots_;
	std::vector<const Image*> screenshot_cache_;

	int current_vote_;

	UI::Box main_box_;
	UI::TabPanel tabs_;
	UI::Box box_comments_, box_comment_rows_, box_screenies_, box_screenies_buttons_, box_votes_,
	   voting_stats_;
	UI::Panel box_comment_rows_placeholder_;

	class CommentEditor : public UI::Window {
	public:
		CommentEditor(AddOnsCtrl& ctrl, std::shared_ptr<AddOns::AddOnInfo> info, const int64_t index)
		   : UI::Window(&ctrl.get_topmost_forefather(),
		                UI::WindowStyle::kFsMenu,
		                "write_comment",
		                0,
		                0,
		                100,
		                100,
		                index < 0 ? _("Write Comment") : _("Edit Comment")),
		     info_(info),
		     index_(index),
		     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
		     markup_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
		     buttons_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
		     preview_(&main_box_, 0, 0, 300, 150, UI::PanelStyle::kFsMenu, "", UI::Align::kLeft),
		     text_(new UI::MultilineEditbox(&main_box_, 0, 0, 450, 200, UI::PanelStyle::kFsMenu)),
		     ok_(&buttons_box_,
		         "ok",
		         0,
		         0,
		         kRowButtonSize,
		         kRowButtonSize,
		         UI::ButtonStyle::kFsMenuPrimary,
		         _("OK")),
		     reset_(&buttons_box_,
		            "reset",
		            0,
		            0,
		            kRowButtonSize,
		            kRowButtonSize,
		            UI::ButtonStyle::kFsMenuSecondary,
		            _("Reset")),
		     cancel_(&buttons_box_,
		             "cancel",
		             0,
		             0,
		             kRowButtonSize,
		             kRowButtonSize,
		             UI::ButtonStyle::kFsMenuSecondary,
		             _("Cancel")) {
			if (ctrl.username().empty()) {
				die();
				return;
			}

			buttons_box_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
			buttons_box_.add_space(kRowButtonSpacing);
			buttons_box_.add(&reset_, UI::Box::Resizing::kExpandBoth);
			buttons_box_.add_space(kRowButtonSpacing);
			buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);

			auto markup_button = [this](const std::string& name, const std::string& open,
			                            const std::string& close, const std::string& title,
			                            const std::string& tt) {
				UI::Button* b = new UI::Button(&markup_box_, name, 0, 0, kRowButtonSize, kRowButtonSize,
				                               UI::ButtonStyle::kFsMenuMenu, title, tt);
				b->sigclicked.connect([this, open, close]() { apply_format(open, close); });
				return b;
			};
			markup_box_.add(markup_button("markup_bold", "<font bold=true>", "</font>",
			                              /** TRANSLATORS: Short for Bold text markup */
			                              _("B"), _("Bold")));
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_italic", "<font italic=true>", "</font>",
			                              /** TRANSLATORS: Short for Italic text markup */
			                              _("I"), _("Italic")));
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_line", "<font underline=true>", "</font>",
			                              /** TRANSLATORS: Short for Underline text markup */
			                              _("_"), _("Underline")));
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_shadow", "<font shadow=true>", "</font>",
			                              /** TRANSLATORS: Short for Shadow text markup */
			                              _("S"), _("Shadow")));
			markup_box_.add_space(kRowButtonSpacing);
			{
				UI::Button* b = new UI::Button(&markup_box_, "markup_color", 0, 0, kRowButtonSize,
				                               kRowButtonSize, UI::ButtonStyle::kFsMenuMenu,
				                               /** TRANSLATORS: Short for Color text markup */
				                               _("C"), _("Color…"));
				b->sigclicked.connect([this]() {
					UI::ColorChooser c(
					   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, RGBColor(0xffffff), nullptr);
					if (c.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
						std::string str = "<font color=";
						str += c.get_color().hex_value();
						str += ">";
						apply_format(str, "</font>");
					}
				});
				markup_box_.add(b);
			}
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_tiny", "<font size=8>", "</font>",
			                              /** TRANSLATORS: Short for Tiny text markup */
			                              _("1"), _("Tiny")));
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_small", "<font size=11>", "</font>",
			                              /** TRANSLATORS: Short for Small text markup */
			                              _("2"), _("Small")));
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_normal", "<font size=14>", "</font>",
			                              /** TRANSLATORS: Short for Medium text markup */
			                              _("3"), _("Medium")));
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_large", "<font size=18>", "</font>",
			                              /** TRANSLATORS: Short for Large text markup */
			                              _("4"), _("Large")));
			markup_box_.add_space(kRowButtonSpacing);
			markup_box_.add(markup_button("markup_huge", "<font size=24>", "</font>",
			                              /** TRANSLATORS: Short for Huge text markup */
			                              _("5"), _("Huge")));
			markup_box_.add_space(kRowButtonSpacing);

			main_box_.add(&markup_box_, UI::Box::Resizing::kAlign, UI::Align::kRight);
			main_box_.add_space(kRowButtonSpacing);
			main_box_.add(text_, UI::Box::Resizing::kFullSize);
			main_box_.add_space(kRowButtonSpacing);
			main_box_.add(&preview_, UI::Box::Resizing::kFullSize);
			main_box_.add_space(kRowButtonSpacing);
			main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

			cancel_.sigclicked.connect([this]() { die(); });
			reset_.sigclicked.connect([this]() { reset_text(); });
			ok_.sigclicked.connect([this, &ctrl]() {
				try {
					std::string message = text_->get_text();
					if (message.empty()) {
						return;
					}
					ctrl.net().comment(*info_, message, index_);
					*info_ = ctrl.net().fetch_one_remote(info_->internal_name);
					end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
				} catch (const std::exception& e) {
					log_err("Edit comment #%" PRId64 " for %s: %s", index_, info_->internal_name.c_str(),
					        e.what());
					UI::WLMessageBox m(
					   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
					   (boost::format(_("The comment could not be submitted.\n\nError Message:\n%s")) %
					    e.what())
					      .str(),
					   UI::WLMessageBox::MBoxType::kOk);
					m.run<UI::Panel::Returncodes>();
				}
			});

			reset_text();
			set_center_panel(&main_box_);
			center_to_parent();
			text_->focus();

			initialization_complete();
		}

		void think() override {
			UI::Window::think();

			std::string p = "<rt><p>";
			p += g_style_manager->font_style(UI::FontStyle::kItalic).as_font_tag(_("Preview:"));
			p += "</p><p>";

			std::string message = text_->get_text();
			for (;;) {
				size_t pos = message.find('\n');
				if (pos == std::string::npos) {
					break;
				}
				message = message.replace(pos, 1, "<br>");
			}

			p += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(message);
			p += "</p></rt>";

			ok_.set_enabled(!message.empty());
			try {
				// TODO(Nordfriese): The font renderer produces memory leaks if the text
				// is invalid (which is exactly what we're trying to check here). There
				// must be a safer (and faster) way to check whether the text is valid.
				UI::g_fh->render(p, preview_.get_inner_w());
			} catch (const std::exception& e) {
				ok_.set_enabled(false);
				p = "<rt><p>";
				p += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
				        .as_font_tag(_("The comment contains invalid richtext markup:"));
				p += "</p><p>";
				p += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
				        .as_font_tag(richtext_escape(e.what()));
				p += "</p></rt>";
			}

			if (preview_.get_text() != p) {
				preview_.set_text(p);
			}
		}

	private:
		std::shared_ptr<AddOns::AddOnInfo> info_;
		const int64_t index_;

		UI::Box main_box_, markup_box_, buttons_box_;
		UI::MultilineTextarea preview_;
		UI::MultilineEditbox* text_;
		UI::Button ok_, reset_, cancel_;

		void apply_format(const std::string& open_tag, const std::string& close_tag) {
			const size_t caret = text_->get_caret_pos();
			if (text_->has_selection()) {
				std::string str = open_tag;
				str += text_->get_selected_text();
				str += close_tag;
				text_->replace_selected_text(str);
			} else {
				std::string str = text_->get_text().substr(0, caret);
				str += open_tag;
				str += close_tag;
				str += text_->get_text().substr(caret);
				text_->set_text(str);
				text_->set_caret_pos(caret + open_tag.size());
			}
			text_->focus();
		}

		void reset_text() {
			text_->set_text(index_ < 0 ? "" : info_->user_comments[index_].message);
			think();
		}
	};

	class CommentRow : public UI::MultilineTextarea {
	public:
		CommentRow(AddOnsCtrl& ctrl,
		           std::shared_ptr<AddOns::AddOnInfo> info,
		           RemoteInteractionWindow& r,
		           UI::Panel& parent,
		           const std::string& text,
		           const int64_t index)
		   : UI::MultilineTextarea(&parent,
		                           0,
		                           0,
		                           0,
		                           0,
		                           UI::PanelStyle::kFsMenu,
		                           text,
		                           UI::Align::kLeft,
		                           UI::MultilineTextarea::ScrollMode::kNoScrolling),
		     ctrl_(ctrl),
		     info_(info),
		     index_(index),
		     edit_(this, "edit", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("Edit…")) {
			edit_.sigclicked.connect([this, &r]() {
				if (ctrl_.username().empty()) {
					return;
				}
				CommentEditor m(ctrl_, info_, index_);
				if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
					r.update_data();
				}
			});
			layout();
			update_edit_enabled();
			initialization_complete();
		}
		void update_edit_enabled() {
			/* Admins can edit all posts; normal users only their own posts and only if the post was
			 * never edited by an admin yet. */
			edit_.set_visible(!ctrl_.username().empty() &&
			                  (ctrl_.net().is_admin() ||
			                   (info_->user_comments[index_].username == ctrl_.username() &&
			                    (info_->user_comments[index_].editor.empty() ||
			                     info_->user_comments[index_].editor == ctrl_.username()))));
		}
		void layout() override {
			UI::MultilineTextarea::layout();
			edit_.set_pos(Vector2i(get_w() - edit_.get_w(), 0));
		}

	private:
		AddOnsCtrl& ctrl_;
		std::shared_ptr<AddOns::AddOnInfo> info_;
		const int64_t index_;
		UI::Button edit_;
	};
	UI::MultilineTextarea comments_header_;
	std::list<std::unique_ptr<CommentRow>> comment_rows_;
	UI::Icon screenshot_;

	UI::Dropdown<uint8_t> own_voting_;
	UI::ProgressBar* voting_bars_[AddOns::kMaxRating];
	UI::Textarea* voting_txt_[AddOns::kMaxRating];
	UI::Textarea screenshot_stats_, screenshot_descr_, voting_stats_summary_;
	UI::Button screenshot_next_, screenshot_prev_, write_comment_, ok_, login_button_;
};
std::map<std::pair<std::string, std::string>, std::string>
   RemoteInteractionWindow::downloaded_screenshots_cache_;

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
     version_(this,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsMenuInfoPanelHeading,
              0,
              0,
              0,
              0,
              /** TRANSLATORS: (MajorVersion)+(MinorVersion) */
              (boost::format(_("%1$s+%2$u")) % AddOns::version_to_string(info->version) %
               info->i18n_version)
                 .str(),
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
           (boost::format(
               /** TRANSLATORS: Filesize · Download count · Average rating · Number of comments ·
                  Number of screenshots */
               _("%1$s   ⬇ %2$u   ★ %3$s   “” %4$u   ▣ %5$u")) %
            filesize_string(info->total_file_size) % info->download_count %
            (info->number_of_votes() ? (boost::format("%.2f") % info->average_rating()).str() :
                                       "–") %
            info->user_comments.size() % info->screenshots.size())
              .str(),
        UI::Align::kRight),
     txt_(this,
          0,
          0,
          24,
          24,
          UI::PanelStyle::kFsMenu,
          (boost::format("<rt><p>%s</p><p>%s%s</p><p>%s</p></rt>")
           /** TRANSLATORS: Add-On localized name as header (Add-On internal name in italics) */
           % (boost::format(_("%1$s %2$s")) %
              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
                 .as_font_tag(info->descname()) %
              g_style_manager->font_style(UI::FontStyle::kItalic)
                 .as_font_tag((boost::format(_("(%s)")) % info->internal_name).str()))
                .str() %
           g_style_manager->font_style(UI::FontStyle::kItalic)
              .as_font_tag(info->author() == info->upload_username ?
                              (boost::format(_("by %s")) % info->author()).str() :
                              (boost::format(_("by %1$s (uploaded by %2$s)")) % info->author() %
                               info->upload_username)
                                 .str()) %
           required_wl_version_and_sync_safety_string(info) %
           g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
              .as_font_tag(info->description()))
             .str()),
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
			   safe_richtext_message(
			      (boost::format(_("Are you certain that you want to install this add-on?\n\n"
			                       "%1$s\n"
			                       "by %2$s\n"
			                       "%3$s\n"
			                       "Version %4$s\n"
			                       "Category: %5$s\n"
			                       "%6$s\n")) %
			       info_->descname() % info_->author() %
			       (info_->verified ? _("Verified") : _("NOT VERIFIED")) %
			       AddOns::version_to_string(info_->version) %
			       AddOns::kAddOnCategories.at(info_->category).descname() % info_->description())
			         .str()),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		ctrl->install_or_upgrade(info_, false);
		ctrl->rebuild();
	});
	upgrade_.sigclicked.connect([this, ctrl, info, installed_version]() {
		if (!info->verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(
			   &ctrl->get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Upgrade"),
			   safe_richtext_message(
			      (boost::format(_("Are you certain that you want to upgrade this add-on?\n\n"
			                       "%1$s\n"
			                       "by %2$s\n"
			                       "%3$s\n"
			                       "Installed version: %4$s\n"
			                       "Available version: %5$s\n"
			                       "Category: %6$s\n"
			                       "%7$s")) %
			       info->descname() % info->author() %
			       (info->verified ? _("Verified") : _("NOT VERIFIED")) %
			       AddOns::version_to_string(installed_version) %
			       AddOns::version_to_string(info->version) %
			       AddOns::kAddOnCategories.at(info->category).descname() % info->description())
			         .str()),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		ctrl->install_or_upgrade(info, !full_upgrade_possible_);
		ctrl->rebuild();
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
	     std::vector<UI::Panel*>{&category_, &version_, &verified_, &bottom_row_right_}) {
		p->set_handle_mouse(true);
	}
	category_.set_tooltip(
	   (boost::format(_("Category: %s")) % AddOns::kAddOnCategories.at(info->category).descname())
	      .str());
	version_.set_tooltip(
	   /** TRANSLATORS: (MajorVersion)+(MinorVersion) */
	   (boost::format(_("Version: %1$s+%2$u")) % AddOns::version_to_string(info->version) %
	    info->i18n_version)
	      .str());
	verified_.set_tooltip(
	   info->internal_name.empty() ?
         _("Error") :
	   info->verified ?
         _("Verified by the Widelands Development Team") :
         _("This add-on was not checked by the Widelands Development Team yet. We cannot guarantee "
	        "that it does not contain harmful or offensive content."));
	bottom_row_right_.set_tooltip(
	   info->internal_name.empty() ?
         "" :
         (boost::format("%s<br>%s<br>%s<br>%s<br>%s") %
	       (boost::format(
	           ngettext("Total size: %u byte", "Total size: %u bytes", info->total_file_size)) %
	        info->total_file_size)
	          .str() %
	       (boost::format(ngettext("%u download", "%u downloads", info->download_count)) %
	        info->download_count)
	          .str() %
	       (info->number_of_votes() ? (boost::format(ngettext("Average rating: %1$.3f (%2$u vote)",
	                                                          "Average rating: %1$.3f (%2$u votes)",
	                                                          info->number_of_votes())) %
	                                   info->average_rating() % info->number_of_votes())
	                                     .str() :
                                     _("No votes yet")) %
	       (boost::format(ngettext("%u comment", "%u comments", info->user_comments.size())) %
	        info->user_comments.size()) %
	       (boost::format(ngettext("%u screenshot", "%u screenshots", info->screenshots.size())) %
	        info->screenshots.size())
	          .str())
	         .str());

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
	        &install_, &uninstall_, &interact_, &upgrade_, &category_, &version_, &verified_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	const int icon_size = 2 * kRowButtonSize + kRowButtonSpacing;
	icon_.set_size(icon_size, icon_size);
	icon_.set_pos(Vector2i(0, kRowButtonSpacing));
	version_.set_size(
	   3 * kRowButtonSize + 2 * kRowButtonSpacing, kRowButtonSize - kRowButtonSpacing);
	version_.set_pos(Vector2i(
	   get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing, kRowButtonSize + kRowButtonSpacing));
	uninstall_.set_pos(Vector2i(get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing, 0));
	upgrade_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 0));
	install_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	interact_.set_pos(Vector2i(get_w() - kRowButtonSize, 2 * kRowButtonSize));
	category_.set_pos(
	   Vector2i(get_w() - 3 * kRowButtonSize - 2 * kRowButtonSpacing, 2 * kRowButtonSize));
	verified_.set_pos(
	   Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 2 * kRowButtonSize));
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

}  // namespace FsMenu
