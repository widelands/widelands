/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "ui_fsmenu/editor.h"

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "wui/text_constants.h"

FullscreenMenuEditor::FullscreenMenuEditor() :
	FullscreenMenuMainMenu(),

// Title
	title
		(this, get_w() / 2, m_title_y, _("Editor"), UI::Align_HCenter),

// Buttons
	new_map
		(this, "new_map",
		 m_butx, m_buty, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("New Map"), "", true, false),
	load_map
		(this, "load_map",
		 m_butx, get_y_from_preceding(new_map) + m_padding, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Load Map"), "", true, false),
	back
		(this, "back",
		 m_butx, m_back_button_y, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Back"), "", true, false)
{
	new_map.sigclicked.connect
		(boost::bind(&FullscreenMenuEditor::end_modal, boost::ref(*this), static_cast<int32_t>(MenuTarget::kNewMap)));
	load_map.sigclicked.connect
		(boost::bind(&FullscreenMenuEditor::end_modal, boost::ref(*this), static_cast<int32_t>(MenuTarget::kLoadMap)));
	back.sigclicked.connect
		(boost::bind(&FullscreenMenuEditor::end_modal, boost::ref(*this), static_cast<int32_t>(MenuTarget::kBack)));

	title.set_font(ui_fn(), fs_big(), UI_FONT_CLR_FG);
}
