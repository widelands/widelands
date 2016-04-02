/*
 * Copyright (C) 2016 by Widelands Development Team
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

#include "ui_fsmenu/about.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"


FullscreenMenuAbout::FullscreenMenuAbout()
	:
	FullscreenMenuBase(),

// Values for alignment and size
	butw_    (get_w() / 5),
	buth_    (get_h() * 9 / 200),
	hmargin_ (get_w() * 19 / 200),
	padding_ (10),
	tab_panel_width_(get_inner_w() - 2 * hmargin_),
	tab_panel_y_(get_h() * 14 / 100),

	title_(this, get_w() / 2, buth_, _("About Widelands"), UI::Align::kCenter),

	close_
		(this, "close",
		 get_w() * 2 / 4 - butw_ / 2,
		 get_inner_h() - hmargin_,
		 butw_, buth_,
		 g_gr->images().get("images/ui_basic/but2.png"),
		 _("Close"), std::string(), true, false),

	tabs_(this, hmargin_, 0,
			tab_panel_width_, get_inner_h() - tab_panel_y_ - buth_ - hmargin_,
			g_gr->images().get("images/ui_basic/but1.png"),
			UI::TabPanel::Type::kBorder)
{
	title_.set_fontsize(UI_FONT_SIZE_BIG);
	tabs_.set_pos(Point(hmargin_, tab_panel_y_));

	tabs_.add_tab("txts/README.lua");
	tabs_.add_tab("txts/LICENSE.lua");
	tabs_.add_tab("txts/AUTHORS.lua");
	tabs_.add_tab("txts/TRANSLATORS.lua");

	close_.sigclicked.connect(boost::bind(&FullscreenMenuAbout::clicked_back, this));
}
