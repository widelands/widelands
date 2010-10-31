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
					type->set_pic(g_gr->get_picture(PicMod_UI, buf));
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

struct MultiPlayerPlayerGroup : public UI::Box {
	MultiPlayerPlayerGroup
		(UI::Panel            * const parent, uint8_t id,
		 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
		 GameSettingsProvider * const settings,
		 std::string const & fname, uint32_t const fsize,
		 std::map<std::string,PictureID> & tp,
		 std::map<std::string,std::string> & tn)
		 :
		 UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
		 player(0),
		 type(0),
		 tribe(0),
		 init(0),
		 s(settings),
		 m_id(id),
		 m_tribepics(tp),
		 m_tribenames(tn)
	{
		set_size(w, h);
		char buf[42];
		snprintf
			(buf, sizeof(buf), "pics/fsel_editor_set_player_0%i_pos.png", id + 1);
		player =
			new UI::Icon(this, 0, 0, h, h, g_gr->get_picture(PicMod_UI, buf));
		add(player, UI::Box::AlignCenter);
		type = new UI::Callback_Button
			(this, "player_type",
			 0, 0, h, h,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 boost::bind
				 (&MultiPlayerPlayerGroup::toggle_type, boost::ref(*this)),
			 std::string(), std::string(), true, false, fname, fsize);
		add(type, UI::Box::AlignCenter);
		tribe = new UI::Callback_Button
			(this, "player_tribe",
			 0, 0, h, h,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 boost::bind
				 (&MultiPlayerPlayerGroup::toggle_tribe, boost::ref(*this)),
			 std::string(), std::string(), true, false, fname, fsize);
		add(tribe, UI::Box::AlignCenter);
		init = new UI::Callback_Button
			(this, "player_init",
			 0, 0, w - 4 * h, h,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 boost::bind
				 (&MultiPlayerPlayerGroup::toggle_init, boost::ref(*this)),
			 std::string(), std::string(), true, false, fname, fsize);
		add(init, UI::Box::AlignCenter);
		team = new UI::Callback_Button
			(this, "player_init",
			 0, 0, h, h,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 boost::bind
				 (&MultiPlayerPlayerGroup::toggle_team, boost::ref(*this)),
			 std::string(), std::string(), true, false, fname, fsize);
		add(team, UI::Box::AlignCenter);
	}

	/// Toggle through the types
	void toggle_type() {
		if (m_id >= s->settings().players.size())
			return;

		assert(s->settings().usernum == 0);

		s->nextPlayerState(m_id);
	}

	/// Toggle through the tribes
	void toggle_tribe() {
		GameSettings const & settings = s->settings();

		if (m_id >= settings.players.size())
			return;

		std::vector<PlayerSettings> pl = settings.players;
		std::string const & currenttribe = pl.at(m_id).tribe;
		std::string nexttribe = settings.tribes.at(0).name;

		for (uint32_t i = 0; i < settings.tribes.size() - 1; ++i)
			if (settings.tribes[i].name == currenttribe) {
				nexttribe = settings.tribes.at(i + 1).name;
				break;
			}
		s->setPlayerTribe(m_id, nexttribe);
	}

	/// Toggle through the initializations
	void toggle_init() {
		GameSettings const & settings = s->settings();

		if (m_id >= settings.players.size())
			return;

		PlayerSettings const & player = settings.players[m_id];
		container_iterate_const(std::vector<TribeBasicInfo>, settings.tribes, j)
			if (j.current->name == player.tribe)
				return
					s->setPlayerInit
						(m_id,
						 (player.initialization_index + 1)
						 %
						 j.current->initializations.size());
		assert(false);
	}

	/// Toggle through the teams
	void toggle_team() {
		const GameSettings & settings = s->settings();
		
		if (m_id >= settings.players.size())
			return;
		
		Widelands::TeamNumber currentteam = settings.players.at(m_id).team;
		Widelands::TeamNumber maxteam = settings.players.size() / 2;
		Widelands::TeamNumber newteam;
		
		if (currentteam >= maxteam)
			newteam = 0;
		else
			newteam = currentteam + 1;
		
		s->setPlayerTeam(m_id, newteam);
	}

	/// Refresh all user interfaces
	void refresh() {
		GameSettings const & settings = s->settings();

		if (m_id >= settings.players.size()) {
			set_visible(false);
			return;
		}

		set_visible(true);

		PlayerSettings const & player = settings.players[m_id];
		bool typeaccess       = s->canChangePlayerState(m_id);
		bool tribeaccess      = s->canChangePlayerTribe(m_id);
		bool const initaccess = s->canChangePlayerInit(m_id);
		bool teamaccess       = s->canChangePlayerTeam(m_id);

		if (player.state == PlayerSettings::stateClosed) {
			type ->set_tooltip(_("Closed"));
			type ->set_pic(g_gr->get_picture(PicMod_UI, "pics/stop.png"));
			team ->set_visible(false);
			team ->set_enabled(false);
			tribe->set_visible(false);
			tribe->set_enabled(false);
			init ->set_visible(false);
			init ->set_enabled(false);
		} else {
			type->set_visible(true);
			type->set_enabled(typeaccess);

			if (player.state == PlayerSettings::stateOpen) {
				type ->set_tooltip(_("Open"));
				type ->set_pic(g_gr->get_picture(PicMod_UI, "pics/continue.png"));
				team ->set_visible(false);
				team ->set_enabled(false);
				tribe->set_visible(false);
				tribe->set_enabled(false);
				init ->set_visible(false);
				init ->set_enabled(false);
			} else {
				std::string title;
				std::string pic = "pics/";
				if (player.state == PlayerSettings::stateComputer) {
					if (player.ai.empty()) {
						title = _("Computer");
						pic += "novalue.png";
					} else {
						title = _("AI: ");
						title += _(player.ai);
						pic += "ai_" + player.ai + ".png";
					}
				} else { // PlayerSettings::stateHuman
					title = _("Human");
					pic += "genstats_nrworkers.png";
				}
				type->set_tooltip(title.c_str());
				type->set_pic(g_gr->get_picture(PicMod_UI, pic));
				std::string tribepath("tribes/" + player.tribe);
				if (!m_tribenames[player.tribe].size()) {
					// get tribes name and picture
					Profile prof
						((tribepath + "/conf").c_str(), 0, "tribe_" + player.tribe);
					Section & global = prof.get_safe_section("tribe");
					m_tribenames[player.tribe] = global.get_safe_string("name");
					m_tribepics[player.tribe] =
						g_gr->get_picture
							(PicMod_UI,
							 (tribepath + "/") + global.get_safe_string("icon"));
				}
				tribe->set_tooltip(m_tribenames[player.tribe].c_str());
				tribe->set_pic(m_tribepics[player.tribe]);
				{
					i18n::Textdomain td(tribepath); // for translated initialisation
					container_iterate_const
						 (std::vector<TribeBasicInfo>, settings.tribes, i)
					{
						if (i.current->name == player.tribe) {
							init->set_title
								(_
									(i.current->initializations.at
										(player.initialization_index)
									 .second));
							break;
						}
					}
				}

				if (player.team) {
					char buf[64];
					snprintf(buf, sizeof(buf), "%i", player.team);
					team->set_title(buf);
				} else {
					team->set_title("--");
				}

				team ->set_visible(true);
				tribe->set_visible(true);
				init ->set_visible(true);
				team ->set_enabled(teamaccess);
				tribe->set_enabled(tribeaccess);
				init ->set_enabled(initaccess);
			}
		}
	}

	UI::Icon               * player;
	UI::Button             * type;
	UI::Button             * tribe;
	UI::Button             * init;
	UI::Button             * team;
	GameSettingsProvider   * const s;
	uint8_t                  const m_id;
	std::map<std::string,PictureID> & m_tribepics;
	std::map<std::string,std::string> & m_tribenames;
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
playerbox(this, w * 6 / 15, buth, UI::Box::Vertical, w * 9 / 15, h - buth),
m_buth(buth),
m_fsize(fsize),
m_fname(fname)
{
	// Clientbox and labels
	labels.push_back
		(new UI::Textarea
			(this,
			 UI::Scrollbar::Size * 6 / 5, buth / 3,
			 w / 3 - buth - UI::Scrollbar::Size * 2, buth));
	labels.back()->set_text(_("Client name"));
	labels.back()->set_font(fname, fsize * 3 / 4, UI_FONT_CLR_FG);

	labels.push_back
		(new UI::Textarea
			(this,
			 w / 3 - buth - UI::Scrollbar::Size * 6 / 5, buth / 3,
			 buth * 2, buth));
	labels.back()->set_text(_("Mode"));
	labels.back()->set_font(fname, fsize * 3 / 4, UI_FONT_CLR_FG);

	clientbox.set_size(w / 3, h - buth);
	clientbox.set_scrolling(true);
	c.resize(MAXCLIENTS);
	for (uint32_t i = 0; i < c.size(); ++i) {
		c.at(i) = 0;
	}

	// Playerbox and labels
	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15, buth / 3,
			 buth, buth));
	labels.back()->set_text(_("Start"));
	labels.back()->set_font(fname, fsize * 3 / 4, UI_FONT_CLR_FG);

	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15 + buth, buth / 3,
			 buth, buth));
	labels.back()->set_text(_("Type"));
	labels.back()->set_font(fname, fsize * 3 / 4, UI_FONT_CLR_FG);

	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15 + buth * 2, buth / 3,
			 buth, buth));
	labels.back()->set_text(_("Tribe"));
	labels.back()->set_font(fname, fsize * 3 / 4, UI_FONT_CLR_FG);

	labels.push_back
		(new UI::Textarea
			(this,
			 w * 6 / 15 + buth * 3, buth / 3,
			 w * 9 / 15 - 4 * buth, buth));
	labels.back()->set_text(_("Initialization"));
	labels.back()->set_font(fname, fsize * 3 / 4, UI_FONT_CLR_FG);

	labels.push_back(new UI::Textarea(this, w - buth, buth / 3, buth, buth));
	labels.back()->set_text(_("Team"));
	labels.back()->set_font(fname, fsize * 3 / 4, UI_FONT_CLR_FG);

	playerbox.set_size(w * 9 / 15, h - buth);
	p.resize(MAX_PLAYERS);
	for (uint8_t i = 0; i < p.size(); ++i) {
		p.at(i) = new MultiPlayerPlayerGroup
			(&playerbox, i,
			 0, 0, playerbox.get_w(), buth,
			 s, fname, fsize,
			 m_tribepics, m_tribenames);
		playerbox.add(&*p.at(i), 1);
	}
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

	// Update / initialize client groups
	for (uint32_t i = 0; (i < settings.users.size()) && (i < MAXCLIENTS); ++i) {
		if (!c.at(i)) {
			c.at(i) = new MultiPlayerClientGroup
				(&clientbox, i,
				 0, 0, clientbox.get_w(), m_buth,
				 s, m_fname, m_fsize);
			clientbox.add(&*c.at(i), 1);
		}
		c.at(i)->refresh();
	}

	// Update player groups
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		p.at(i)->refresh();
}
