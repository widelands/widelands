/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_map_options.h"

#include <cstdio>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"


inline EditorInteractive & MainMenuMapOptions::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}


/**
 * Create all the buttons etc...
*/
MainMenuMapOptions::MainMenuMapOptions(EditorInteractive & parent, bool modal)
	:
	UI::Window
		(&parent, "map_options",
		 0, 0, 350, 520,
		 _("Map Options")),
	padding_(4),
	indent_(10),
	labelh_(20),
	checkbox_space_(25),
	butw_((get_inner_w() - 3 * padding_) / 2),
	buth_(20),
	max_w_(get_inner_w() - 2 * padding_),
	ok_(
		this, "ok",
		padding_, get_inner_h() - padding_ - buth_,
		butw_, buth_,
		g_gr->images().get("images/ui_basic/but5.png"),
		_("OK")),
	cancel_(
		this, "cancel",
		butw_ + 2 * padding_, get_inner_h() - padding_ - buth_,
		butw_, buth_,
		g_gr->images().get("images/ui_basic/but1.png"),
		_("Cancel")),
	tab_box_(this, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
	tabs_(&tab_box_, 0, 0, nullptr),

	main_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
	tags_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),
	teams_box_(&tabs_, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),

	name_(&main_box_, 0, 0, max_w_, g_gr->images().get("images/ui_basic/but1.png")),
	author_(&main_box_, 0, 0, max_w_, g_gr->images().get("images/ui_basic/but1.png")),
	size_(&main_box_, 0, 0, max_w_ - indent_, labelh_, ""),

	teams_list_(&teams_box_, 0, 0, max_w_, 60, UI::Align::kLeft, true),

	modal_(modal) {

	descr_ = new UI::MultilineEditbox(
					&main_box_, 0, 0, max_w_, 9 * labelh_, "", g_gr->images().get("images/ui_basic/but1.png"));
	hint_ = new UI::MultilineEditbox(
				  &main_box_, 0, 0, max_w_, 4 * labelh_, "", g_gr->images().get("images/ui_basic/but1.png"));

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Map Name:")), UI::Align::kLeft);
	main_box_.add(&name_, UI::Align::kLeft);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Authors:")), UI::Align::kLeft);
	main_box_.add(&author_, UI::Align::kLeft);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Description:")), UI::Align::kLeft);
	main_box_.add(descr_, UI::Align::kLeft);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Hint (optional):")),
					  UI::Align::kLeft);
	main_box_.add(hint_, UI::Align::kLeft);
	main_box_.add_space(indent_);

	main_box_.add(&size_, UI::Align::kLeft);
	main_box_.add_space(indent_);

	main_box_.set_size(max_w_, get_inner_h() - buth_ - 2 * padding_);

	tags_box_.add(new UI::Textarea(&tags_box_, 0, 0, max_w_, labelh_, _("Tags:")), UI::Align::kLeft);
	add_tag_checkbox(&tags_box_, "unbalanced", _("Unbalanced"));
	add_tag_checkbox(&tags_box_, "ffa", _("Free for all"));
	add_tag_checkbox(&tags_box_, "1v1", _("1v1"));
	add_tag_checkbox(&tags_box_, "2teams", _("Teams of 2"));
	add_tag_checkbox(&tags_box_, "3teams", _("Teams of 3"));
	add_tag_checkbox(&tags_box_, "4teams", _("Teams of 4"));
	tags_box_.set_size(max_w_, get_inner_h() - buth_ - 2 * padding_);

	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, _("Suggested Teams:")),
						UI::Align::kLeft);
	teams_box_.add(&teams_list_, UI::Align::kLeft);
	// TODO(GunChleoc): We need team images in the listselect here,
	// so map editors will be able to delete entries.
	// This is waiting for the new RT renderer.
	teams_list_.add("Not implemented yet.", "", nullptr, false);

	unsigned int nr_players = static_cast<unsigned int>(eia().egbase().map().get_nrplayers());
	std::string players = (boost::format(ngettext("%u Player", "%u Players", nr_players)) % nr_players).str();
	teams_box_.add(new UI::Textarea(&teams_box_, 0, 0, max_w_, labelh_, players), UI::Align::kLeft);
	teams_box_.set_size(max_w_, get_inner_h() - buth_ - 2 * padding_);

	tab_box_.add(&tabs_, UI::Align::kLeft, true);
	tabs_.add("main_map_options",
				 g_gr->images().get("images/wui/menus/menu_toggle_minimap.png"), &main_box_, _("Main Options"));
	tabs_.add("map_tags", g_gr->images().get("images/ui_basic/checkbox_checked.png"), &tags_box_, _("Tags"));
	tabs_.add("map_teams", g_gr->images().get("images/wui/editor/editor_menu_player_menu.png"),
				 &teams_box_, _("Teams"));
	tabs_.set_size(max_w_, get_inner_h() - buth_ - 2 * padding_);
	tab_box_.set_size(max_w_, get_inner_h() - buth_ - 2 * padding_);

	name_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	author_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	descr_->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));

	ok_.sigclicked.connect
		(boost::bind(&MainMenuMapOptions::clicked_ok, boost::ref(*this)));
	ok_.set_enabled(false);
	cancel_.sigclicked.connect
		(boost::bind(&MainMenuMapOptions::clicked_cancel, boost::ref(*this)));

	update();
	center_to_parent();
	move_to_top();
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
*/
void MainMenuMapOptions::update() {
	const Widelands::Map & map = eia().egbase().map();
	author_.set_text(map.get_author());
	name_.set_text(map.get_name());
	size_.set_text((boost::format(_("Size: %1% x %2%"))
						 % map.get_width()
						 % map.get_height()).str());
	descr_->set_text(map.get_description());
	hint_->set_text(map.get_hint());

	std::set<std::string> tags = map.get_tags();
	for (std::pair<std::string, UI::Checkbox*> tag : tags_checkboxes_) {
		tag.second->set_state(tags.count(tag.first) > 0);
	}
}


/**
 * Called when one of the editboxes are changed
*/
void MainMenuMapOptions::changed() {
	ok_.set_enabled(true);
}

void MainMenuMapOptions::clicked_ok() {
	eia().egbase().map().set_name(name_.text());
	eia().egbase().map().set_author(author_.text());
	g_options.pull_section("global").set_string("realname", author_.text());
	eia().egbase().map().set_description(descr_->get_text());
	eia().egbase().map().set_hint(hint_->get_text());

	eia().egbase().map().clear_tags();
	for (std::pair<std::string, UI::Checkbox*> tag : tags_checkboxes_) {
		if (tag.second->get_state()) {
			eia().egbase().map().add_tag(tag.first);
		}
	}

	if (modal_) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
	} else {
		die();
	}
}

void MainMenuMapOptions::clicked_cancel() {
	if (modal_) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	} else {
		die();
	}
}

/*
 * Add a tag to the checkboxes
 */
void MainMenuMapOptions::add_tag_checkbox(UI::Box* parent, std::string tag, std::string displ_name) {
	UI::Box* box = new UI::Box(parent, 0, 0, UI::Box::Horizontal, max_w_, checkbox_space_, 0);
	UI::Checkbox* cb = new UI::Checkbox(box, Point(0, 0), displ_name);
	box->add(cb, UI::Align::kLeft, true);
	box->add_space(checkbox_space_);
	parent->add(box, UI::Align::kLeft);
	parent->add_space(padding_);
	tags_checkboxes_[tag] = cb;
}
