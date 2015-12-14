/*
 * Copyright (C) 2015 by the Widelands Development Team
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

#include "wui/suggested_teams_box.h"

#include <set>
#include <string>

#include <boost/format.hpp>

#include "graphic/graphic.h"

namespace UI {

SuggestedTeamsBox::SuggestedTeamsBox(Panel * parent,
							int32_t x, int32_t y,
							uint32_t orientation,
							int32_t padding, int32_t indent, int32_t label_height,
							int32_t max_x, int32_t max_y,
							uint32_t inner_spacing) :
	UI::Box(parent, x, y, orientation, max_x, max_y, inner_spacing),
	m_padding(padding),
	m_indent(indent),
	m_label_height(label_height)
{
	m_player_icons.clear();
	m_suggested_teams.clear();
	set_size(max_x, max_y);

	m_suggested_teams_box_label =
			new UI::Textarea(this, "", UI::Align_CenterLeft);
	add(m_suggested_teams_box_label, UI::Align::Align_Left);
}
SuggestedTeamsBox::~SuggestedTeamsBox() {
	SuggestedTeamsBox::hide();
}

void SuggestedTeamsBox::hide() {
	// Delete former images
	for (UI::Icon* player_icon : m_player_icons) {
		player_icon->set_icon(nullptr);
		player_icon->set_visible(false);
		player_icon->set_no_frame();
	}
	m_player_icons.clear();

	// Delete vs. labels
	for (UI::Textarea* vs_label : m_vs_labels) {
		vs_label->set_visible(false);
	}
	m_vs_labels.clear();

	set_visible(false);
	m_suggested_teams_box_label->set_visible(false);
	m_suggested_teams_box_label->set_text("");
}


void SuggestedTeamsBox::show(const std::vector<Widelands::Map::SuggestedTeamLineup>& suggested_teams)
{
	hide();
	m_suggested_teams = suggested_teams;

	if (!m_suggested_teams.empty()) {

		// Initialize
		uint8_t lineup_counter = 0;
		set_visible(true);
		m_suggested_teams_box_label->set_visible(true);
		/** TRANSLATORS: Label for the list of suggested teams when choosing a map */
		m_suggested_teams_box_label->set_text(_("Suggested Teams:"));
		int32_t teamlist_offset = m_suggested_teams_box_label->get_y() +
										  m_suggested_teams_box_label->get_h() +
										  m_padding;

		// Parse suggested teams
		UI::Icon* player_icon;
		UI::Textarea * vs_label;
		for (const Widelands::Map::SuggestedTeamLineup& lineup : m_suggested_teams) {

			m_lineup_box =
					new UI::Box(this, m_indent, teamlist_offset + lineup_counter * (m_label_height + m_padding),
									UI::Box::Horizontal, get_w() - m_indent);

			m_lineup_box->set_size(get_w(), m_label_height + m_padding);

			bool is_first = true;
			for (const Widelands::Map::SuggestedTeam& team : lineup) {

				if (!is_first) {
					m_lineup_box->add_space(m_padding);
					vs_label = new UI::Textarea(m_lineup_box, "x", UI::Align_BottomCenter);
					m_lineup_box->add(vs_label, UI::Align::Align_Left);
					vs_label->set_visible(true);
					m_vs_labels.push_back(vs_label);
					m_lineup_box->add_space(m_padding);
				}
				is_first = false;

				for (uint16_t player : team) {
					assert(player < MAX_PLAYERS);
					const std::string player_filename = (boost::format("pics/fsel_editor_set_player_0%i_pos.png")
																	 % (++player)).str().c_str();
					player_icon = new UI::Icon(m_lineup_box, 0, 0, 20, 20,
																	 g_gr->images().get(player_filename));
					player_icon->set_visible(true);
					player_icon->set_no_frame();
					m_lineup_box->add(player_icon, UI::Align::Align_Left);
					m_player_icons.push_back(player_icon);
				} // Players in team
			} // Teams in lineup
			++lineup_counter;
		} // All lineups

		// Adjust size to content
		set_size(get_w(), teamlist_offset + lineup_counter * (m_label_height + m_padding));
	}
}

}
