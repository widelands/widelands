/*
 * Copyright (C) 2015-2016 by the Widelands Development Team
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

#include "graphic/graphic.h"

namespace {
static char const * const player_pictures_small[] = {
	"images/players/fsel_editor_set_player_01_pos.png",
	"images/players/fsel_editor_set_player_02_pos.png",
	"images/players/fsel_editor_set_player_03_pos.png",
	"images/players/fsel_editor_set_player_04_pos.png",
	"images/players/fsel_editor_set_player_05_pos.png",
	"images/players/fsel_editor_set_player_06_pos.png",
	"images/players/fsel_editor_set_player_07_pos.png",
	"images/players/fsel_editor_set_player_08_pos.png"
};
} // namespace

namespace UI {

SuggestedTeamsBox::SuggestedTeamsBox(Panel * parent,
							int32_t x, int32_t y,
							uint32_t orientation,
							int32_t padding, int32_t indent,
							int32_t max_x, int32_t max_y) :
	UI::Box(parent, x, y, orientation, max_x, max_y, g_gr->images().get(player_pictures_small[0])->height()),
	padding_(padding),
	indent_(indent),
	label_height_(g_gr->images().get(player_pictures_small[0])->height() + padding)
{
	player_icons_.clear();
	suggested_teams_.clear();
	set_size(max_x, max_y);

	suggested_teams_box_label_ = new UI::Textarea(this, "", UI::Align::kCenterLeft);
	add(suggested_teams_box_label_, UI::Align::kLeft);
}
SuggestedTeamsBox::~SuggestedTeamsBox() {
	SuggestedTeamsBox::hide();
}

void SuggestedTeamsBox::hide() {
	// Delete former images
	for (UI::Icon* player_icon : player_icons_) {
		player_icon->set_icon(nullptr);
		player_icon->set_visible(false);
		player_icon->set_no_frame();
	}
	player_icons_.clear();

	// Delete vs. labels
	for (UI::Textarea* vs_label : vs_labels_) {
		vs_label->set_visible(false);
	}
	vs_labels_.clear();

	set_visible(false);
	suggested_teams_box_label_->set_visible(false);
	suggested_teams_box_label_->set_text("");
}


void SuggestedTeamsBox::show(const std::vector<Widelands::Map::SuggestedTeamLineup>& suggested_teams)
{
	hide();
	suggested_teams_ = suggested_teams;

	if (!suggested_teams_.empty()) {

		// Initialize
		uint8_t lineup_counter = 0;
		set_visible(true);
		suggested_teams_box_label_->set_visible(true);
		/** TRANSLATORS: Label for the list of suggested teams when choosing a map */
		suggested_teams_box_label_->set_text(_("Suggested Teams:"));
		int32_t teamlist_offset = suggested_teams_box_label_->get_y() +
										  suggested_teams_box_label_->get_h() +
										  padding_;

		// Parse suggested teams
		UI::Icon* player_icon;
		UI::Textarea * vs_label;
		for (const Widelands::Map::SuggestedTeamLineup& lineup : suggested_teams_) {

			lineup_box_ =
					new UI::Box(this, indent_, teamlist_offset + lineup_counter * (label_height_),
									UI::Box::Horizontal, get_w() - indent_);

			lineup_box_->set_size(get_w(), label_height_);

			bool is_first = true;
			for (const Widelands::Map::SuggestedTeam& team : lineup) {

				if (!is_first) {
					lineup_box_->add_space(padding_);
					vs_label = new UI::Textarea(lineup_box_, "x", UI::Align::kBottomCenter);
					lineup_box_->add(vs_label, UI::Align::kLeft);
					vs_label->set_visible(true);
					vs_labels_.push_back(vs_label);
					lineup_box_->add_space(padding_);
				}
				is_first = false;

				for (Widelands::PlayerNumber player : team) {
					assert(player < MAX_PLAYERS);
					const Image* player_image = g_gr->images().get(player_pictures_small[player]);
					assert(player_image);
					player_icon = new UI::Icon(lineup_box_, 0, 0,
														player_image->width(), player_image->height(), player_image);
					player_icon->set_visible(true);
					player_icon->set_no_frame();
					lineup_box_->add(player_icon, UI::Align::kLeft);
					player_icons_.push_back(player_icon);
				} // Players in team
			} // Teams in lineup
			++lineup_counter;
		} // All lineups

		// Adjust size to content
		set_size(get_w(), teamlist_offset + lineup_counter * (label_height_));
	}
}

}
