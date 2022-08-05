/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#include "ui_fsmenu/keyboard_options.h"

#include "base/i18n.h"
#include "base/string.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/icon.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_fsmenu/mousewheel_options.h"
#include "wlapplication.h"

namespace FsMenu {

constexpr int16_t kPadding = 4;
constexpr int16_t kButtonHeight = 28;

struct ShortcutChooser : public UI::Window {
	ShortcutChooser(KeyboardOptions& parent,
	                const KeyboardShortcut c,
	                Widelands::Game* game_for_fastplace)
	   : UI::Window(&parent.get_topmost_forefather(),
	                UI::WindowStyle::kFsMenu,
	                "choose_shortcut",
	                0,
	                0,
	                300,
	                200,
	                to_string(c)),
	     code_(c),
	     key(get_shortcut(code_)),
	     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, kPadding) {
		UI::Button* const reset = new UI::Button(
		   &box_, "reset", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Reset to default"));
		reset->sigclicked.connect([this, &parent]() {
			key = get_default_shortcut(code_);
			if (is_fastplace(code_)) {
				fastplace = parent.get_default_fastplace_shortcuts(code_);
			}
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
		});

		UI::Button* const cancel = new UI::Button(
		   &box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Cancel"));
		cancel->sigclicked.connect(
		   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack); });

		UI::Button* const ok =
		   new UI::Button(&box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK"));
		ok->sigclicked.connect(
		   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk); });

		UI::MultilineTextarea* const txt = new UI::MultilineTextarea(
		   &box_, 0, 0, 200, 100, UI::PanelStyle::kFsMenu,
		   _("Press the new shortcut or close this window to cancel."), UI::Align::kCenter);

		box_.add(txt, UI::Box::Resizing::kExpandBoth);

		if (game_for_fastplace != nullptr) {
			fastplace = get_fastplace_shortcuts(code_);
			create_fastplace_dropdowns(*game_for_fastplace, ok->get_h());
		}

		box_.add(reset, UI::Box::Resizing::kFullSize);
		box_.add(cancel, UI::Box::Resizing::kFullSize);
		box_.add(ok, UI::Box::Resizing::kFullSize);
		set_center_panel(&box_);
		center_to_parent();
		initialization_complete();
	}

	const KeyboardShortcut code_;
	SDL_Keysym key;
	std::map<std::string, std::string> fastplace;

protected:
	UI::Box box_;

	void create_fastplace_dropdowns(const Widelands::Game& game, const int height) {
		for (Widelands::DescriptionIndex t = 0; t < game.descriptions().nr_tribes(); ++t) {
			const Widelands::TribeDescr* tribe = game.descriptions().get_tribe_descr(t);
			const auto iterator = fastplace.find(tribe->name());
			const std::string selection = (iterator == fastplace.end() ? "" : iterator->second);
			create_fastplace_dropdown(height, tribe->name(), selection, tribe);
		}
		for (const auto& pair : fastplace) {
			if (!game.descriptions().tribe_exists(pair.first)) {
				create_fastplace_dropdown(height, pair.first, pair.second, nullptr);
			}
		}
		box_.add_space(height);
	}

	void create_fastplace_dropdown(int height,
	                               const std::string& tribename,
	                               const std::string& selection,
	                               const Widelands::TribeDescr* tribe) {
		UI::Box* hbox = new UI::Box(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal);
		box_.add(hbox, UI::Box::Resizing::kFullSize);

		UI::Icon* icon =
		   new UI::Icon(hbox, UI::PanelStyle::kFsMenu, 0, 0, height, height,
		                tribe == nullptr ? nullptr : g_image_cache->get(tribe->basic_info().icon));
		icon->set_handle_mouse(true);
		icon->set_tooltip(tribe == nullptr ? tribename : tribe->descname());
		hbox->add(icon, UI::Box::Resizing::kAlign, UI::Align::kCenter);
		hbox->add_space(kPadding);

		UI::Dropdown<std::string>* dd = new UI::Dropdown<std::string>(
		   hbox, "choose_fastplace", 0, 0, 400, 8, height, "", UI::DropdownType::kTextual,
		   UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuMenu);

		dd->add(_("(unused)"), "", nullptr, selection.empty());

		if (tribe != nullptr) {
			for (Widelands::DescriptionIndex di : tribe->buildings()) {
				const Widelands::BuildingDescr* bld = tribe->get_building_descr(di);
				if (bld->is_buildable()) {
					dd->add(bld->descname(), bld->name(), bld->icon(), selection == bld->name());
				}
			}
		}

		if (!dd->has_selection()) {
			// The assigned building is defined by a currently disabled add-on.
			dd->add(selection, selection, nullptr, true);
		}

		dd->selected.connect([this, tribename, dd]() {
			const std::string& sel = dd->get_selected();
			if (sel.empty()) {
				fastplace.erase(tribename);
			} else {
				fastplace[tribename] = sel;
			}
		});
		hbox->add(dd, UI::Box::Resizing::kExpandBoth);
	}

	bool handle_key(const bool down, const SDL_Keysym code) override {
		if (!down) {
			return false;
		}

		switch (code.sym) {

		// Ignore modifiers
		case SDLK_RCTRL:
		case SDLK_RSHIFT:
		case SDLK_RALT:
		case SDLK_RGUI:
		case SDLK_LCTRL:
		case SDLK_LSHIFT:
		case SDLK_LALT:
		case SDLK_LGUI:
		// Also ignore reserved system keys
		case SDLK_RETURN:
		case SDLK_SPACE:
		case SDLK_ESCAPE:
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_BACKSPACE:
		case SDLK_TAB:
			return false;

		default:
			key = code;
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
			return true;
		}
	}
};

KeyboardOptions::KeyboardOptions(Panel& parent)
   : UI::Window(&parent.get_topmost_forefather(),
                UI::WindowStyle::kFsMenu,
                "keyboard_options",
                /* Size and position will be set by the main game window so that
                 * it can follow resolution change.
                 * This window's height is set smaller than the Options dialog to
                 * avoid looking too big and cluttered. The width is set quite big
                 * relative to the game window though, because we need lots of
                 * horizontal space for some button labels.
                 */
                0,
                0,
                0,
                0,
                _("Edit Keyboard And Mouse Actions")),
     buttons_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, 0, kPadding),
     tabs_(this, UI::TabPanelStyle::kFsMenu),
     mousewheel_options_(&tabs_),
     reset_(&buttons_box_,
            "reset",
            0,
            0,
            0,
            0,
            UI::ButtonStyle::kFsMenuSecondary,
            _("Reset all"),
            _("Reset all to defaults")),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         reset_.get_w(),
         reset_.get_h(),
         UI::ButtonStyle::kFsMenuPrimary,
         _("OK")) {
	std::map<KeyboardShortcut, UI::Button*> all_keyboard_buttons;

	auto generate_title = [](const KeyboardShortcut key) {
		const std::string shortcut = shortcut_string_for(key, false);
		return format(
		   /** TRANSLATORS: This is a button label for a keyboard shortcut in the form
		      "Action: Key" */
		   _("%1$s: %2$s"), to_string(key), shortcut);
	};

	auto add_key = [this, generate_title, &all_keyboard_buttons](
	                  UI::Box& box, const KeyboardShortcut key) {
		UI::Button* b = new UI::Button(&box, std::to_string(static_cast<int>(key)), 0, 0, 0, 0,
		                               UI::ButtonStyle::kFsMenuMenu, generate_title(key));
		all_keyboard_buttons.emplace(std::make_pair(key, b));
		box.add(b, UI::Box::Resizing::kFullSize);
		box.add_space(kPadding);
		b->sigclicked.connect([this, b, key, generate_title]() {
			const bool fastplace = is_fastplace(key);
			WLApplication* const app = WLApplication::get();
			app->enable_handle_key(false);
			ShortcutChooser c(*this, key, fastplace ? game_.get() : nullptr);
			while (c.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
				KeyboardShortcut conflict;
				if (set_shortcut(key, c.key, &conflict)) {
					if (is_fastplace(key)) {
						set_fastplace_shortcuts(key, c.fastplace);
					}
					b->set_title(generate_title(key));
					break;
				}
				UI::WLMessageBox warning(
				   get_parent(), UI::WindowStyle::kFsMenu, _("Keyboard Shortcut Conflict"),
				   as_richtext_paragraph(
				      format(_("The shortcut you selected (‘%1$s’) is already in use for the "
				               "following action: ‘%2$s’. Please select a different shortcut "
				               "or change the conflicting shortcut first."),
				             shortcut_string_for(c.key, true), to_string(conflict)),
				      UI::FontStyle::kFsMenuLabel, UI::Align::kCenter),
				   UI::WLMessageBox::MBoxType::kOk);
				warning.run<UI::Panel::Returncodes>();
			}
			app->enable_handle_key(true);
		});
	};

	auto create_tab = [this, add_key](const std::string& title,
	                                  const KeyboardShortcut shortcut_start,
	                                  const KeyboardShortcut shortcut_end) {
		const uint16_t s1 = static_cast<uint16_t>(shortcut_start);
		const uint16_t s2 = static_cast<uint16_t>(shortcut_end);
		assert(s1 < s2);
		UI::Box* b =
		   new UI::Box(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, kPadding);
		b->set_force_scrolling(true);
		for (uint16_t k = s1; k <= s2; ++k) {
			add_key(*b, static_cast<KeyboardShortcut>(k));
		}
		tabs_.add(title, title, b, "");
		boxes_.push_back(b);
	};
	create_tab(_("General"), KeyboardShortcut::kCommon_Begin, KeyboardShortcut::kCommon_End);
	create_tab(_("Main Menu"), KeyboardShortcut::kMainMenu_Begin, KeyboardShortcut::kMainMenu_End);
	create_tab(_("Editor"), KeyboardShortcut::kEditor_Begin, KeyboardShortcut::kEditor_End);
	create_tab(_("Game"), KeyboardShortcut::kInGame_Begin, KeyboardShortcut::kInGame_End);

	const size_t fastplace_tab_index = tabs_.tabs().size();
	create_tab(_("Fastplace"), KeyboardShortcut::kFastplace_Begin, KeyboardShortcut::kFastplace_End);

	tabs_.add("options_scroll", _("Mouse Scrolling"), &mousewheel_options_, "");

	buttons_box_.add_inf_space();
	buttons_box_.add(&reset_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	buttons_box_.add_inf_space();
	buttons_box_.add(&ok_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	buttons_box_.add_inf_space();

	tabs_.sigclicked.connect([this, all_keyboard_buttons, generate_title, fastplace_tab_index]() {
		if (tabs_.active() == fastplace_tab_index && game_ == nullptr) {
			game_.reset(new Widelands::Game());
			game_->create_loader_ui({}, false, "", "", this);
			game_->load_all_tribes();
			game_->postload_addons();

			init_fastplace_default_shortcuts();

			for (const auto& pair : all_keyboard_buttons) {
				pair.second->set_title(generate_title(pair.first));
			}

			game_->remove_loader_ui();
		}
	});
	reset_.sigclicked.connect([this, all_keyboard_buttons, generate_title]() {
		init_shortcuts(true);
		if (game_) {
			init_fastplace_default_shortcuts();
		}

		for (const auto& pair : all_keyboard_buttons) {
			pair.second->set_title(generate_title(pair.first));
		}

		mousewheel_options_.reset();
	});
	ok_.sigclicked.connect([this]() {
		mousewheel_options_.apply_settings();
		die();
	});

	get_parent()->layout();
	initialization_complete();
}

std::map<std::string, std::string>
KeyboardOptions::get_default_fastplace_shortcuts(const KeyboardShortcut id) const {
	assert(is_fastplace(id));
	assert(game_);

	std::string key = get_fastplace_group_name(id);
	assert(key.compare(0, kFastplaceGroupPrefix.size(), kFastplaceGroupPrefix) == 0);
	key.erase(0, kFastplaceGroupPrefix.size());
	std::map<std::string, std::string> values;
	for (Widelands::DescriptionIndex t = 0; t < game_->descriptions().nr_tribes(); ++t) {
		const Widelands::TribeDescr* tribe = game_->descriptions().get_tribe_descr(t);
		const auto it = tribe->fastplace_defaults().find(key);
		if (it != tribe->fastplace_defaults().end()) {
			values.emplace(tribe->name(), it->second);
		}
	}

	return values;
}

void KeyboardOptions::init_fastplace_default_shortcuts() {
	assert(game_);
	std::map<std::string /* key */, std::map<std::string /* tribe */, std::string /* building */>>
	   fp;
	for (Widelands::DescriptionIndex t = 0; t < game_->descriptions().nr_tribes(); ++t) {
		const Widelands::TribeDescr* tribe = game_->descriptions().get_tribe_descr(t);
		for (const auto& pair : tribe->fastplace_defaults()) {
			fp[kFastplaceGroupPrefix + pair.first][tribe->name()] = pair.second;
		}
	}
	::init_fastplace_default_shortcuts(fp);
}

bool KeyboardOptions::handle_key(bool down, SDL_Keysym code) {
	if (down && code.sym == SDLK_RETURN) {
		mousewheel_options_.apply_settings();
		die();
		return true;
	}
	return UI::Window::handle_key(down, code);
}

void KeyboardOptions::layout() {
	if (!is_minimal()) {
		reset_.set_desired_size(get_w() / 3, kButtonHeight);
		ok_.set_desired_size(reset_.get_w(), reset_.get_h());
		int w;
		int h;
		buttons_box_.get_desired_size(&w, &h);
		buttons_box_.set_size(get_inner_w(), h);
		buttons_box_.set_pos(Vector2i(0, get_inner_h() - h - kPadding));
		tabs_.set_size(get_inner_w(), get_inner_h() - h - 2 * kPadding);
		for (UI::Box* b : boxes_) {
			b->set_max_size(tabs_.get_inner_w(), tabs_.get_inner_h());
		}
		mousewheel_options_.set_size(tabs_.get_inner_w(), tabs_.get_inner_h());
	}
	UI::Window::layout();
}

}  // namespace FsMenu
