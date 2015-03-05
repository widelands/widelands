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
#include "profile/profile.h"
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
		 20, 20, 350, parent.get_inner_h() - 80,
		 _("Map Options")),
	padding_(4),
	indent_(10),
	labelh_(20),
	butw_((get_inner_w() - 3 * padding_) / 2),
	buth_(20),
	max_w_(get_inner_w() - 2 * padding_),
	ok_(
		this, "ok",
		padding_, get_inner_h() - padding_ - buth_,
		butw_, buth_,
		g_gr->images().get("pics/but0.png"),
		_("OK")),
	cancel_(
		this, "cancel",
		butw_ + 2 * padding_, get_inner_h() - padding_ - buth_,
		butw_, buth_,
		g_gr->images().get("pics/but1.png"),
		_("Cancel")),

	main_box_(this, padding_, padding_, UI::Box::Vertical, max_w_, get_inner_h(), 0),

	name_(&main_box_, 0, 0, max_w_, labelh_, g_gr->images().get("pics/but1.png")),
	author_(&main_box_, 0, 0, max_w_, labelh_, g_gr->images().get("pics/but1.png")),
	nrplayers_(&main_box_, 0, 0, max_w_ - indent_, labelh_, ""),
	size_(&main_box_, 0, 0, max_w_ - indent_, labelh_, ""),

	modal_(modal) {

	descr_ = new UI::MultilineEditbox(&main_box_, 0, 0, max_w_, 5 * labelh_, "");
	hint_ = new UI::MultilineEditbox(&main_box_, 0, 0, max_w_, 5 * labelh_, "");

	UI::Button * btn =
		new UI::Button
			(&main_box_, "set_origin",
			 0, 0, max_w_, buth_,
			 g_gr->images().get("pics/but0.png"),
			 _("Set origin"),
			 (boost::format("%s %s")
			 % _("Set the position that will have the coordinates (0, 0). This will "
				"be the top-left corner of a generated minimap.")
			  % _("This setting will take effect immediately.")).str());

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Minimap:")), UI::Box::AlignLeft);
	main_box_.add(btn, UI::Box::AlignLeft);
	main_box_.add_space(2 * indent_);

	main_box_.add(&nrplayers_, UI::Box::AlignLeft);
	main_box_.add_space(padding_);
	main_box_.add(&size_, UI::Box::AlignLeft);
	main_box_.add_space(2 * indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Map Name:")), UI::Box::AlignLeft);
	main_box_.add(&name_, UI::Box::AlignLeft);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Authors:")), UI::Box::AlignLeft);
	main_box_.add(&author_, UI::Box::AlignLeft);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Description:")), UI::Box::AlignLeft);
	main_box_.add(descr_, UI::Box::AlignLeft);
	main_box_.add_space(indent_);


	main_box_.add(new UI::Textarea(&main_box_, 0, 0, max_w_, labelh_, _("Hint (optional):")), UI::Box::AlignLeft);
	main_box_.add(hint_, UI::Box::AlignLeft);
	main_box_.add_space(indent_);
	/* NOCOM
	if (!map.get_background().empty())
		s.set_string("background",  map.get_background ());
	s.set_string("tags", boost::algorithm::join(map.get_tags(), ","));
	 */

	main_box_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 2 * buth_ - 3 * padding_);

	btn->sigclicked.connect
		(boost::bind
		 (&EditorInteractive::select_tool, &parent,
		  boost::ref(parent.tools.set_origin), EditorTool::First));

	name_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	author_.changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	descr_->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));

	ok_.sigclicked.connect
		(boost::bind(&MainMenuMapOptions::clicked_ok, boost::ref(*this)));
	ok_.set_enabled(false);
	cancel_.sigclicked.connect
		(boost::bind(&MainMenuMapOptions::clicked_cancel, boost::ref(*this)));

	update();
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
*/
void MainMenuMapOptions::update() {
	const Widelands::Map & map = eia().egbase().map();
	size_.set_text((boost::format(_("Size: %1% x %2%"))
						 % map.get_width()
						 % map.get_height()).str());
	author_.set_text(map.get_author());
	name_.set_text(map.get_name());
	nrplayers_.set_text((boost::format(_("Number of Players: %u"))
								% static_cast<unsigned int>(map.get_nrplayers())).str());
	descr_->set_text(map.get_description());
	hint_->set_text(map.get_hint());
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
	if (modal_) {
		end_modal(1);
	} else {
		die();
	}
}

void MainMenuMapOptions::clicked_cancel() {
	if (modal_) {
		end_modal(0);
	} else {
		die();
	}
}

