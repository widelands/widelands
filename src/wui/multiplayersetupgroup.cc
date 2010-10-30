/*
 * Copyright (C) 2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "multiplayersetupgroup.h"

#include "constants.h"
#include "gamesettings.h"
#include "i18n.h"
#include "log.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "wexception.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/icon.h"
#include "ui_basic/scrollbar.h"
#include "ui_basic/textarea.h"

struct MultiPlayerClientGroup : public UI::Box {
	MultiPlayerClientGroup
		(UI::Panel            * const parent, uint8_t id,
		 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
		 GameSettingsProvider * const settings,
		 std::string const & fname, uint32_t const fsize)
		 :
		 UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
		 type_icon(0),
		 type(0),
		 s(settings),
		 m_id(id),
		 m_save(-2)
	{
		set_size(w, h);
		name = new UI::Textarea
			(this, 0, 0, w - h - UI::Scrollbar::Size * 11 / 5, h);
		name->set_font(fname, fsize, UI_FONT_CLR_FG);
		add(name, UI::Box::AlignCenter);
		// Either Button if changeable OR text if not
		if (id == settings->settings().usernum) { // Our Client
			type = new UI::Callback_Button
				(this, "client_type",
				 0, 0, h, h,
				 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
				 boost::bind
					 (&MultiPlayerClientGroup::toggle_type, boost::ref(*this)),
				 std::string(), std::string(), true, false, fname, fsize);
			add(type, UI::Box::AlignCenter);
		} else { // just a shown client
			type_icon = new UI::Icon
				(this, 0, 0, h, h,
				 g_gr->get_picture(PicMod_UI, "pics/menu_tab_watch.png"));
			add(type_icon, UI::Box::AlignCenter);
		}

	}

	/// Switch human players and spectator
	void toggle_type() {
		UserSettings us = s->settings().users.at(m_id);
		int16_t p = us.position;
		if (p == UserSettings::none())
			p = -1;

		for (++p; p < static_cast<int16_t>(s->settings().players.size()); ++p) {
			if
				(s->settings().players.at(p).state == PlayerSettings::stateHuman
				 ||
				 s->settings().players.at(p).state == PlayerSettings::stateOpen)
			{
				s->setPlayerNumber(p);
				return;
			}
		}
		s->setPlayerNumber(UserSettings::none());
	}

	/// Care about visibility and current values
	void refresh() {
		UserSettings us = s->settings().users[m_id];
		if (us.position == UserSettings::notConnected()) {
			name->set_text(_("<free>"));
			if (type)
				type->set_visible(false);
			else
				type_icon->set_visible(false);
		} else {
			name->set_text(us.name);
			if (m_save != us.position) {
				char buf[42] = "pics/menu_tab_watch.png";
				char buf2[128];
				if (us.position < UserSettings::highestPlayernum()) {
					snprintf
						(buf, sizeof(buf),
						 "pics/genstats_enable_plr_0%i.png", us.position + 1);
					snprintf(buf2, sizeof(buf2), _("Player %i"), us.position + 1);
				} else
					snprintf(buf2, sizeof(buf2), _("Spectator"));

				// Either Button if changeable OR text if not
				if (m_id == s->settings().usernum) {
					type->set_pic(g_gr->get_picture(PicMod_Game, buf));
					type->set_tooltip(buf2);
					type->set_visible(true);
				} else {
					type_icon->setIcon(g_gr->get_picture(PicMod_UI, buf));
				}
				m_save = us.position;
			}
		}
	}

	UI::Textarea           * name;
	UI::Icon               * type_icon;
	UI::Button             * type;
	GameSettingsProvider   * const s;
	uint8_t                  const m_id;
	int16_t                  m_save; // saved position to check rewrite need.
};

MultiPlayerSetupGroup::MultiPlayerSetupGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t butw, uint32_t buth,
	 std::string const & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
s(settings),
clientbox(this, 0, buth, UI::Box::Vertical, w / 3, h - buth),
m_buth(buth),
m_fsize(fsize),
m_fname(fname)
{
	c.resize(MAXCLIENTS);
	for (uint32_t i = 0; i < c.size(); ++i) {
		c.at(i) = 0;
	}
	clientbox.set_size(w / 3, h - buth);
	clientbox.set_scrolling(true);
	refresh();
}


MultiPlayerSetupGroup::~MultiPlayerSetupGroup()
{
	for (uint32_t i = 0; i < c.size(); ++i) {
		delete c.at(i);
	}
}


/**
 * Update display and enabled buttons based on current settings.
 */
void MultiPlayerSetupGroup::refresh()
{
	GameSettings const & settings = s->settings();

	for (uint32_t i = 0; (i < settings.users.size()) && (i < MAXCLIENTS); ++i) {
		if (!c.at(i)) {
			c.at(i) = new MultiPlayerClientGroup
				(&clientbox, i,
				 0, 0, clientbox.get_w(), m_buth,
				 s, m_fname, m_fsize);
			clientbox.add(&*c[i], 1);
		}
		c[i]->refresh();
	}
}
