/*
 * Copyright (C) 2022-2023 by the Widelands Development Team
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

#include "wui/pinned_note.h"

#include "ui_basic/color_chooser.h"
#include "wui/interactive_player.h"

constexpr int16_t kButtonSize = 32;
constexpr int16_t kSpacing = 4;

PinnedNoteEditor::PinnedNoteEditor(InteractivePlayer& parent,
                                   UI::UniqueWindow::Registry& r,
                                   Widelands::FCoords pos,
                                   const std::string& text,
                                   const RGBColor& rgb,
                                   bool is_new)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      format("pinned_note_%d_%d", pos.x, pos.y),
                      &r,
                      0,
                      0,
                      100,
                      100,
                      _("Pinned Note")),
     iplayer_(parent),
     pos_(pos),
     initial_color_(rgb),
     current_color_(rgb),
     delete_on_cancel_(is_new),

     box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     buttons_box_(&box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     ok_(
        &buttons_box_, "ok", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiPrimary, _("OK")),
     delete_(&buttons_box_,
             "delete",
             0,
             0,
             kButtonSize,
             kButtonSize,
             UI::ButtonStyle::kWuiSecondary,
             _("Delete")),
     cancel_(&buttons_box_,
             "cancel",
             0,
             0,
             kButtonSize,
             kButtonSize,
             UI::ButtonStyle::kWuiSecondary,
             _("Cancel")),
     color_(&buttons_box_,
            "color",
            0,
            0,
            kButtonSize,
            kButtonSize,
            UI::ButtonStyle::kWuiSecondary,
            "",
            _("Change color…")),
     text_(new UI::EditBox(&box_, 0, 0, 400, UI::PanelStyle::kWui)) {
	buttons_box_.add(&delete_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kSpacing);
	buttons_box_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kSpacing);
	buttons_box_.add(&color_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kSpacing);
	buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);

	box_.add(text_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(kSpacing);
	box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

	cancel_.sigclicked.connect([this]() { die(); });
	color_.sigclicked.connect([this]() {
		UI::ColorChooser c(
		   &iplayer_, UI::WindowStyle::kWui, current_color_, &iplayer_.player().get_playercolor());
		if (c.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
			current_color_ = c.get_color();
			update_color_preview();
		}
	});
	delete_.sigclicked.connect([this]() {
		send_delete();
		die();
	});
	ok_.sigclicked.connect([this]() { ok(); });
	text_->ok.connect([this]() { ok(); });
	text_->cancel.connect([this]() { die(); });

	subscriber_ = Notifications::subscribe<Widelands::NotePinnedNoteMoved>(
	   [this](const Widelands::NotePinnedNoteMoved& note) {
		   if (iplayer_.player_number() == note.player && pos_ == note.old_pos) {
			   pos_ = note.new_pos;
		   }
	   });

	text_->set_text(text);
	update_color_preview();

	set_center_panel(&box_);
	center_to_parent();
	text_->focus();

	initialization_complete();
}

void PinnedNoteEditor::update_color_preview() {
	color_.set_pic(playercolor_image(current_color_, "images/players/team.png"));
}

void PinnedNoteEditor::send_delete() {
	iplayer_.game().send_player_pinned_note(
	   iplayer_.player_number(), pos_, "", current_color_, true);
}

void PinnedNoteEditor::ok() {
	delete_on_cancel_ = false;
	iplayer_.game().send_player_pinned_note(
	   iplayer_.player_number(), pos_, text_->get_text(), current_color_, false);
	die();
}

void PinnedNoteEditor::die() {
	if (delete_on_cancel_) {
		send_delete();
	}
	UI::UniqueWindow::die();
}

bool PinnedNoteEditor::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
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
	return UI::UniqueWindow::handle_key(down, code);
}
