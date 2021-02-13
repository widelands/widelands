/*
 * Copyright (C) 2007-2021 by the Widelands Development Team
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
 */

#include "wui/game_player_list_menu.h"

#include <memory>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "network/participantlist.h"
#include "ui_basic/icon.h"
#include "ui_basic/textarea.h"

namespace {
constexpr int kMargin = 12;
constexpr int kSpacing = 8;

constexpr int kWidthIcon = 34;
constexpr int kWidthName = 256;
constexpr int kWidthStatus = 128;
constexpr int kWidthPing = 64;
}  // namespace

struct ColumnHeaders : public UI::Box {
	ColumnHeaders(UI::Panel& parent, bool player, bool can_kick)
	   : UI::Box(&parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
	     name_(this, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading, _("Name")),
	     status_(this, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading, _("Status")),
	     ping_(this, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading, _("Ping")),
	     kick_(this, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading, _("Kick")) {

		set_inner_spacing(kSpacing);

		// Space for the playercolor / flag
		add_space(kWidthIcon);
		add(&name_);
		name_.set_desired_size(kWidthName, name_.get_h());
		if (player) {
			add(&status_);
			status_.set_desired_size(kWidthStatus, status_.get_h());
		} else {
			add_space(kWidthStatus);
		}
		add(&ping_);
		ping_.set_desired_size(kWidthPing, ping_.get_h());
		if (can_kick) {
			add(&kick_);
			kick_.set_desired_size(kWidthIcon, kick_.get_h());
		} else {
			kick_.set_visible(false);
		}
	}

	UI::Textarea name_;
	UI::Textarea status_;
	UI::Textarea ping_;
	UI::Textarea kick_;
};

struct ParticipantEntry : public UI::Box {
	ParticipantEntry(UI::Panel& parent, ParticipantList* participants, int16_t index, bool can_kick)
	   : UI::Box(&parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
	     participant_index_(index),
	     flag_(this,
	           UI::PanelStyle::kWui,
	           g_image_cache->get("images/wui/fieldaction/menu_tab_watch.png")),
	     name_(this,
	           "whisper",
	           0,
	           0,
	           0,
	           0,
	           UI::ButtonStyle::kWuiMenu,
	           "",
	           _("Send whisper message to user"),
	           UI::Button::VisualState::kFlat),
	     status_(this, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading),
	     ping_(this, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading),
	     kick_(this,
	           "kick",
	           0,
	           0,
	           kWidthIcon,
	           kWidthIcon,
	           UI::ButtonStyle::kWuiPrimary,
	           g_image_cache->get("images/ui_basic/stop.png"),
	           _("Kick player"),
	           UI::Button::VisualState::kRaised) {

		set_inner_spacing(kSpacing);

		const ParticipantList::ParticipantType type = participants->get_participant_type(index);

		// Add player-colored flag / observer icon
		if (type != ParticipantList::ParticipantType::kSpectator) {
			flag_.set_icon(playercolor_image(
			   participants->get_participant_color(index), "images/players/genstats_player.png"));
		}
		add(&flag_);
		flag_.set_desired_size(kWidthIcon, flag_.get_h());
		// Add name as button
		name_.set_title(participants->get_participant_name(index));
		if (type != ParticipantList::ParticipantType::kAI &&
		    participants->get_participant_name(index) != participants->get_local_playername()) {
			// Some other user, allow chatting on click
			name_.sigclicked.connect(
			   [participants, index]() { participants->participants_whisper(index); });
		} else {
			// Either ourselves or an AI. No chatting possible
			name_.set_tooltip(_("You cannot send a message to this user"));
			name_.set_disable_style(UI::ButtonDisableStyle::kFlat);
			name_.set_enabled(false);
		}
		add(&name_);
		name_.set_desired_size(kWidthName, name_.get_h());
		// Add status as text
		if (type != ParticipantList::ParticipantType::kSpectator) {
			if (participants->get_participant_defeated(index)) {
				status_.set_text(_("Defeated"));
			} else {
				status_.set_text(_("Playing"));
			}
			add(&status_);
			status_.set_desired_size(kWidthStatus, status_.get_h());
		} else {
			add_space(kWidthStatus);
		}
		// Add RTT
		update_ping(participants);
		add(&ping_);
		ping_.set_desired_size(kWidthPing, ping_.get_h());

		if (can_kick) {
			add(&kick_);
			kick_.set_desired_size(kWidthIcon, kick_.get_h());
			kick_.sigclicked.connect(
			   [participants, index]() { participants->participants_kick(index); });

		} else {
			kick_.set_visible(false);
		}
	}

	void update_ping(const ParticipantList* participants) {
		if (participants->get_participant_type(participant_index_) !=
		    ParticipantList::ParticipantType::kAI) {
			ping_.set_text(std::to_string(
			   static_cast<unsigned>(participants->get_participant_rtt(participant_index_))));
		} else {
			ping_.set_text("-");
		}
	}

	int16_t participant_index_;
	UI::Icon flag_;
	UI::Button name_;
	UI::Textarea status_;
	UI::Textarea ping_;
	UI::Button kick_;
};

GamePlayerListMenu::GamePlayerListMenu(Panel& parent,
                                       UI::UniqueWindow::Registry& registry,
                                       ParticipantList* participants)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "player_list_menu", &registry, 100, 100, _("Player List")),
     participants_(participants),
     vbox_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, kSpacing) {

	vbox_.set_border(kMargin, kMargin, kMargin, kMargin);
	set_center_panel(&vbox_);

	rebuild();
	update_signal_connection_ = participants_->participants_updated.connect([this]() { rebuild(); });
	rtt_signal_connection_ = participants_->participants_updated_rtt.connect([this]() {
		for (auto& team : teams_) {
			for (std::unique_ptr<UI::Panel>& panel : team.second) {
				ParticipantEntry* pe = dynamic_cast<ParticipantEntry*>(panel.get());
				if (pe != nullptr) {
					pe->update_ping(participants_);
				}
			}
		}
		for (std::unique_ptr<UI::Panel>& panel : spectators_) {
			ParticipantEntry* pe = dynamic_cast<ParticipantEntry*>(panel.get());
			if (pe != nullptr) {
				pe->update_ping(participants_);
			}
		}
	});

	participants_->enable_participant_rtt_updates(true);

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

GamePlayerListMenu::~GamePlayerListMenu() {
	participants_->enable_participant_rtt_updates(false);
	// Unregister
	update_signal_connection_.disconnect();
	rtt_signal_connection_.disconnect();
}

void GamePlayerListMenu::rebuild() {

	vbox_.clear();
	teams_.clear();
	spectators_.clear();

	assert(participants_->get_participant_counts().humans > 0);
	assert(participants_->get_participant_type(0) != ParticipantList::ParticipantType::kAI);
	// User is the host (that is: same name as host) and there is at least one other human to kick
	const bool can_kick =
	   (participants_->get_participant_name(0) == participants_->get_local_playername()) &&
	   (participants_->get_participant_counts().humans > 1);

	// Create the lines for all participants and add them to the map
	for (int16_t i = 0; i < participants_->get_participant_counts().total; ++i) {
		std::unique_ptr<ParticipantEntry> pe(new ParticipantEntry(
		   vbox_, participants_, i,
		   // is host player but not host entry and no AI entry
		   can_kick && i != 0 &&
		      participants_->get_participant_type(i) != ParticipantList::ParticipantType::kAI));

		// Get the team
		if (participants_->get_participant_type(i) == ParticipantList::ParticipantType::kSpectator) {
			if (spectators_.empty()) {
				// First time something is added: Add a header
				spectators_.emplace_back(new UI::Textarea(
				   &vbox_, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading, _("Spectators")));
				spectators_.emplace_back(new ColumnHeaders(vbox_, false, can_kick));
			}
			spectators_.push_back(std::move(pe));
		} else {
			Widelands::TeamNumber team = participants_->get_participant_team(i);
			if (teams_.count(team) == 0) {
				std::string str;
				if (team == 0) {
					str = _("No team");
				} else {
					// The static_cast is needed so the uint8_t (=team) isn't interpreted as char \0
					str = (boost::format(_("Team %u")) % static_cast<unsigned int>(team)).str();
				}
				teams_[team].emplace_back(new UI::Textarea(
				   &vbox_, UI::PanelStyle::kWui, UI::FontStyle::kWuiInfoPanelHeading, str));
				teams_[team].emplace_back(new ColumnHeaders(vbox_, true, can_kick));
			}
			teams_[team].push_back(std::move(pe));
		}
		assert(!pe);
	}

	static const auto add_from_list = [](const std::vector<std::unique_ptr<UI::Panel>>& v,
	                                     UI::Box& vbox) {
		for (const std::unique_ptr<UI::Panel>& pe : v) {
			vbox.add(pe.get());
		}
	};

	// Iterate over the map and add its entries to the UI
	for (const auto& iter : teams_) {
		// The team with number 0 is the "no team" entry. Add it last
		if (iter.first == 0) {
			continue;
		}
		add_from_list(iter.second, vbox_);
	}
	if (teams_.count(0)) {
		add_from_list(teams_[0], vbox_);
	}
	add_from_list(spectators_, vbox_);
}
