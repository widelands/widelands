/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "editor/ui_menus/main_menu_map_options.h"

#include "base/i18n.h"
#include "base/string.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler.h"
#include "graphic/text_layout.h"
#include "logic/map.h"
#include "logic/note_map_options.h"
#include "ui_basic/textarea.h"
#include "ui_basic/textinput.h"
#include "wlapplication_options.h"
#include "wui/map_tags.h"

inline EditorInteractive& MainMenuMapOptions::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

constexpr unsigned kSuggestedTeamsUnitSize = 24;

SuggestedTeamsEntry::SuggestedTeamsEntry(MainMenuMapOptions* mmmo,
                                         UI::Panel* parent,
                                         const Widelands::Map& map,
                                         unsigned w,
                                         Widelands::SuggestedTeamLineup t)
   : UI::Panel(parent,
               UI::PanelStyle::kWui,
               0,
               0,
               w,
               kSuggestedTeamsUnitSize,
               _("Click player to remove")),
     map_(map),
     team_(std::move(t)),
     delete_(this,
             "delete",
             0,
             0,
             kSuggestedTeamsUnitSize,
             kSuggestedTeamsUnitSize,
             UI::ButtonStyle::kWuiSecondary,
             _("Delete"),
             _("Delete this suggested team lineup")) {
	const size_t nr_teams = team_.size();
	for (size_t index = 0; index < nr_teams; ++index) {
		dropdowns_.push_back(create_dropdown(index));
	}
	buttons_.resize(nr_teams);
	for (size_t i = 0; i < nr_teams; ++i) {
		for (const Widelands::PlayerNumber& p : team_[i]) {
			buttons_[i].push_back(create_button(p));
		}
	}

	delete_.sigclicked.connect([this, mmmo]() { mmmo->delete_suggested_team(this); });

	update();
}

UI::Button* SuggestedTeamsEntry::create_button(Widelands::PlayerNumber p) {
	UI::Button* b =
	   new UI::Button(this, std::to_string(static_cast<unsigned>(p)), 0, 0, kSuggestedTeamsUnitSize,
	                  kSuggestedTeamsUnitSize, UI::ButtonStyle::kWuiSecondary,
	                  playercolor_image(p, "images/players/player_position_menu.png"),
	                  map_.get_scenario_player_name(p + 1), UI::Button::VisualState::kFlat);
	b->sigclicked.connect([this, b]() {
		auto teams_it = team_.begin();
		for (std::vector<UI::Button*>& vector : buttons_) {
			auto t = teams_it->begin();
			for (auto it = vector.begin(); it != vector.end(); ++it, ++t) {
				if (*it == b) {
					vector.erase(it);
					teams_it->erase(t);
					b->die();
					update();
					return;
				}
			}
			++teams_it;
		}
		NEVER_HERE();
	});
	return b;
}

UI::Dropdown<Widelands::PlayerNumber>* SuggestedTeamsEntry::create_dropdown(size_t index) {
	UI::Dropdown<Widelands::PlayerNumber>* dd = new UI::Dropdown<Widelands::PlayerNumber>(
	   this, std::to_string(index), 0, index * kSuggestedTeamsUnitSize, kSuggestedTeamsUnitSize, 8,
	   0, _("+"), UI::DropdownType::kPictorialMenu, UI::PanelStyle::kWui,
	   UI::ButtonStyle::kWuiSecondary);
	for (size_t i = 0; i < map_.get_nrplayers(); ++i) {
		dd->add(map_.get_scenario_player_name(i + 1), i,
		        playercolor_image(i, "images/players/player_position_menu.png"));
	}
	dd->set_tooltip(_("Add a player to this team"));
	dd->selected.connect([this, dd]() {
		const Widelands::PlayerNumber player = dd->get_selected();
		// add this player to this team and remove him from all other teams in this lineup
		{
			size_t row = 0;
			for (Widelands::SuggestedTeam& t : team_) {
				auto button = buttons_[row].begin();
				for (auto it = t.begin(); it != t.end(); ++it, ++button) {
					if (*it == player) {
						t.erase(it);
						(*button)->die();
						buttons_[row].erase(button);
						break;
					}
				}
				++row;
			}
		}
		// determine our index (it may have changed since the creation!)
		unsigned dd_index = dropdowns_.size();
		for (size_t i = 0; i < dropdowns_.size(); ++i) {
			if (dropdowns_[i] == dd) {
				dd_index = i;
				break;
			}
		}
		if (dd_index >= team_.size()) {
			assert(dd_index == team_.size());
			team_.push_back(Widelands::SuggestedTeam());
			buttons_.emplace_back();
		}
		team_[dd_index].push_back(player);
		buttons_[dd_index].push_back(create_button(player));
		update();
	});
	return dd;
}

void SuggestedTeamsEntry::layout() {
	const uint16_t h = kSuggestedTeamsUnitSize * (team_.size() + 1);
	set_desired_size(get_w(), h);
	delete_.set_pos(Vector2i(get_w() / 2, h - kSuggestedTeamsUnitSize));
	delete_.set_size(get_w() / 2, kSuggestedTeamsUnitSize);

	size_t index = 0;
	for (auto& dd : dropdowns_) {
		dd->set_size(kSuggestedTeamsUnitSize, kSuggestedTeamsUnitSize);
		dd->set_pos(Vector2i(0, kSuggestedTeamsUnitSize * (index++)));
	}
	index = 0;
	for (auto& bb : buttons_) {
		size_t index2 = 0;
		for (auto& b : bb) {
			b->set_pos(
			   Vector2i(kSuggestedTeamsUnitSize * (++index2), kSuggestedTeamsUnitSize * index));
		}
		++index;
	}
}

// Delete empty teams, and append an empty team to the end if not present
void SuggestedTeamsEntry::update() {
	int nr_teams = team_.size();
	int nr_dd = dropdowns_.size();
	assert(static_cast<int>(buttons_.size()) == nr_teams);
	for (int i = 0; i < nr_teams;) {
		if (team_[i].empty()) {
			dropdowns_[i]->die();
			assert(buttons_[i].empty());

			for (int j = i + 1; j < nr_dd; ++j) {
				dropdowns_[j - 1] = dropdowns_[j];
			}
			dropdowns_.resize(nr_dd - 1);
			for (int j = i + 1; j < nr_teams; ++j) {
				team_[j - 1] = team_[j];
				buttons_[j - 1] = buttons_[j];
			}
			team_.resize(nr_teams - 1);
			buttons_.resize(nr_teams - 1);

			--nr_teams;
			--nr_dd;
		} else {
			++i;
		}
	}
	assert(nr_teams >= 0);
	assert(nr_dd >= 0);

	assert((nr_dd == nr_teams) ^ (nr_dd == nr_teams + 1));
	if (nr_dd <= nr_teams) {
		dropdowns_.push_back(create_dropdown(nr_teams));
	}

	layout();

	initialization_complete();
}

constexpr uint16_t kMaxRecommendedWaterwayLengthLimit = 20;

/**
 * Create all the buttons etc...
 */
MainMenuMapOptions::MainMenuMapOptions(EditorInteractive& parent, Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "map_options",
                      &registry,
                      350,
                      parent.get_inner_h() - 80,
                      _("Map Options")),

     labelh_(text_height(UI::FontStyle::kWuiLabel) + 4),

     butw_((get_inner_w() - 3 * padding_) / 2),
     max_w_(get_inner_w() - 2 * padding_),
     tab_box_(this,
              UI::PanelStyle::kWui,
              padding_,
              padding_,
              UI::Box::Vertical,
              max_w_,
              get_inner_h(),
              0),
     buttons_box_(&tab_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     ok_(&buttons_box_,
         "ok",
         UI::g_fh->fontset()->is_rtl() ? padding_ : butw_ + 2 * padding_,
         get_inner_h() - padding_ - labelh_,
         butw_,
         labelh_,
         UI::ButtonStyle::kWuiPrimary,
         _("OK")),
     cancel_(&buttons_box_,
             "cancel",
             UI::g_fh->fontset()->is_rtl() ? butw_ + 2 * padding_ : padding_,
             get_inner_h() - padding_ - labelh_,
             butw_,
             labelh_,
             UI::ButtonStyle::kWuiSecondary,
             _("Cancel")),
     tabs_(&tab_box_, UI::TabPanelStyle::kWuiLight),

     main_box_(&tabs_,
               UI::PanelStyle::kWui,
               padding_,
               padding_,
               UI::Box::Vertical,
               max_w_,
               get_inner_h(),
               0),
     tags_box_(&tabs_,
               UI::PanelStyle::kWui,
               padding_,
               padding_,
               UI::Box::Vertical,
               max_w_,
               get_inner_h(),
               0),
     teams_box_(&tabs_,
                UI::PanelStyle::kWui,
                padding_,
                padding_,
                UI::Box::Vertical,
                max_w_,
                get_inner_h(),
                0),
     inner_teams_box_(&teams_box_,
                      UI::PanelStyle::kWui,
                      padding_,
                      padding_,
                      UI::Box::Vertical,
                      max_w_,
                      get_inner_h() / 2,
                      kSuggestedTeamsUnitSize),

     name_(&main_box_, 0, 0, max_w_, UI::PanelStyle::kWui),
     author_(&main_box_, 0, 0, max_w_, UI::PanelStyle::kWui),
     size_(&main_box_,
           UI::PanelStyle::kWui,
           UI::FontStyle::kWuiLabel,
           0,
           0,
           max_w_ - indent_,
           labelh_,
           ""),
     balancing_dropdown_(&tags_box_,
                         "dropdown_balancing",
                         0,
                         0,
                         200,
                         50,
                         24,
                         "",
                         UI::DropdownType::kTextual,
                         UI::PanelStyle::kWui,
                         UI::ButtonStyle::kWuiSecondary),
     theme_dropdown_(&tags_box_,
                     "dropdown_theme",
                     0,
                     0,
                     200,
                     50,
                     24,
                     _("Theme"),
                     UI::DropdownType::kTextual,
                     UI::PanelStyle::kWui,
                     UI::ButtonStyle::kWuiSecondary),
     new_suggested_team_(&teams_box_,
                         "new_suggested_team",
                         0,
                         0,
                         max_w_,
                         kSuggestedTeamsUnitSize,
                         UI::ButtonStyle::kWuiSecondary,
                         _("Add lineup"),
                         _("Add another suggested team lineup")),
     registry_(registry) {

	tab_box_.set_size(max_w_, get_inner_h() - labelh_ - 2 * padding_);
	tabs_.set_size(max_w_, tab_box_.get_inner_h());
	main_box_.set_size(max_w_, tabs_.get_inner_h() - 35);
	tags_box_.set_size(max_w_, tabs_.get_inner_h() - 35);
	teams_box_.set_size(max_w_, tabs_.get_inner_h() - 35);

	// ### Main tab ###

	// Calculate the overall remaining space for MultilineEditboxes.
	uint32_t remaining_space = main_box_.get_inner_h() - 7 * labelh_ - 5 * indent_;

	// We need less space for the hint and the description, but it should at least have 1 line
	// height.
	const int16_t hinth = std::max(labelh_, remaining_space / 3);
	descr_ = new UI::MultilineEditbox(
	   &main_box_, 0, 0, max_w_, remaining_space - hinth, UI::PanelStyle::kWui);
	hint_ = new UI::MultilineEditbox(&main_box_, 0, 0, max_w_, hinth, UI::PanelStyle::kWui);

	main_box_.add(new UI::Textarea(&main_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0,
	                               max_w_, labelh_, _("Map name:")));
	main_box_.add(&name_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0,
	                               max_w_, labelh_, _("Authors:")));
	main_box_.add(&author_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0,
	                               max_w_, labelh_, _("Description:")));
	main_box_.add(descr_);
	main_box_.add_space(indent_);

	main_box_.add(new UI::Textarea(&main_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0,
	                               max_w_, labelh_, _("Hint (optional):")));
	main_box_.add(hint_);
	main_box_.add_space(indent_);

	main_box_.add(&size_);
	main_box_.add_space(indent_);

	// ### Tags tab ###

	tags_box_.add(new UI::Textarea(&tags_box_, UI::PanelStyle::kWui,
	                               UI::FontStyle::kWuiInfoPanelHeading, 0, 0, max_w_, labelh_,
	                               _("Tags:")));

	tags_box_.add_space(padding_);

	UI::Textarea* team_tags_label =
	   new UI::Textarea(&tags_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0, max_w_,
	                    /** TRANSLATORS: Header for suitable team line-up tags, like "Free for all",
	                     * "Teams of 2", etc. */
	                    labelh_, _("This map is suitable for:"));
	team_tags_label->set_tooltip(
	   _("Please add suggested team line-ups in the next tab for all selected options"));
	team_tags_label->set_handle_mouse(true);
	tags_box_.add(team_tags_label);
	tags_box_.add_space(padding_);

	add_tag_checkbox(&tags_box_, "ffa");
	add_tag_checkbox(&tags_box_, "1v1");
	add_tag_checkbox(&tags_box_, "2teams");
	add_tag_checkbox(&tags_box_, "3teams");
	add_tag_checkbox(&tags_box_, "4teams");

	tags_box_.add_space(separator_);

	add_tag_to_dropdown(&balancing_dropdown_, "balanced");
	add_tag_to_dropdown(&balancing_dropdown_, "unbalanced");
	balancing_dropdown_.set_tooltip(
	   _("Mark whether the starting positions provide equal conditions for each player"));
	tags_box_.add(&balancing_dropdown_, UI::Box::Resizing::kFullSize);

	tags_box_.add_space(separator_);

	const std::string theme_tooltip = _("Set the theme for the game loadscreens");
	theme_dropdown_.add(pgettext("map_theme", "(none)"), "", nullptr, false, theme_tooltip);
	for (const Widelands::Map::OldWorldInfo& owi : Widelands::Map::kOldWorldNames) {
		theme_dropdown_.add(owi.descname(), owi.name, nullptr, false, theme_tooltip);
	}
	theme_dropdown_.set_tooltip(theme_tooltip);
	tags_box_.add(&theme_dropdown_, UI::Box::Resizing::kFullSize);

	tags_box_.add_space(separator_);

	UI::Textarea* ww_text =
	   new UI::Textarea(&tags_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0, max_w_,
	                    labelh_, _("Ferry range:"));
	std::string ww_tooltip = _("Enable ferries, waterways, and ferry yards on this map by setting"
	                           " the maximum length of waterways for ferries");
	ww_text->set_tooltip(ww_tooltip);
	ww_text->set_handle_mouse(true);
	tags_box_.add(ww_text);
	UI::Box* ww_box =
	   new UI::Box(&tags_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, max_w_);
	ww_box->set_tooltip(ww_tooltip);
	waterway_length_warning_ =
	   new UI::Icon(ww_box, UI::PanelStyle::kWui, g_image_cache->get("images/ui_basic/stop.png"));
	waterway_length_warning_->set_handle_mouse(true);
	uint32_t ww_spinbox_w = max_w_ - waterway_length_warning_->get_w();
	waterway_length_box_ = new UI::SpinBox(
	   ww_box, 0, 0, ww_spinbox_w, ww_spinbox_w - padding_, 1, 1, 50, UI::PanelStyle::kWui,
	   std::string(), UI::SpinBox::Units::kFields, UI::SpinBox::Type::kBig, 1, 5);
	/** TRANSLATORS: Map Options: Waterways are disabled */
	waterway_length_box_->add_replacement(1, _("Disabled"));
	waterway_length_box_->changed.connect([this]() { update_waterway_length_warning(); });
	ww_box->add(waterway_length_warning_, UI::Box::Resizing::kFullSize);
	ww_box->add_inf_space();
	ww_box->add(waterway_length_box_, UI::Box::Resizing::kFullSize);
	tags_box_.add(ww_box, UI::Box::Resizing::kFullSize);
	tags_box_.add_space(padding_);

	// ### Teams tab ###

	inner_teams_box_.set_force_scrolling(true);
	for (const Widelands::SuggestedTeamLineup& team : parent.egbase().map().get_suggested_teams()) {
		SuggestedTeamsEntry* ste = new SuggestedTeamsEntry(
		   this, &inner_teams_box_, parent.egbase().map(), max_w_ - UI::Scrollbar::kSize, team);
		inner_teams_box_.add(ste);
		inner_teams_box_.add_space(kSuggestedTeamsUnitSize);
		suggested_teams_entries_.push_back(ste);
	}

	const unsigned nr_players = eia().egbase().map().get_nrplayers();
	teams_box_.add(new UI::Textarea(
	   &teams_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0, max_w_, labelh_,
	   format(ngettext("%u Player", "%u Players", nr_players), nr_players)));
	teams_box_.add_space(padding_);
	teams_box_.add(new UI::Textarea(&teams_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0,
	                                0, max_w_, labelh_, _("Suggested Teams:")));
	teams_box_.add_space(padding_);
	teams_box_.add(&inner_teams_box_, UI::Box::Resizing::kFullSize);
	teams_box_.add_space(padding_);
	teams_box_.add(&new_suggested_team_, UI::Box::Resizing::kFullSize);
	new_suggested_team_.sigclicked.connect([this]() {
		SuggestedTeamsEntry* ste =
		   new SuggestedTeamsEntry(this, &inner_teams_box_, eia().egbase().map(),
		                           max_w_ - UI::Scrollbar::kSize, Widelands::SuggestedTeamLineup());
		inner_teams_box_.add(ste);
		suggested_teams_entries_.push_back(ste);
	});

	// ### End of tab content definitions ###

	buttons_box_.add(UI::g_fh->fontset()->is_rtl() ? &ok_ : &cancel_, UI::Box::Resizing::kFullSize);
	buttons_box_.add_space(4);
	buttons_box_.add(UI::g_fh->fontset()->is_rtl() ? &cancel_ : &ok_, UI::Box::Resizing::kFullSize);

	tab_box_.add(&tabs_, UI::Box::Resizing::kFullSize);
	tab_box_.add_space(4);
	tab_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);
	tabs_.add("main_map_options", g_image_cache->get("images/wui/menus/toggle_minimap.png"),
	          &main_box_, _("Main Options"));
	tabs_.add("map_tags", g_image_cache->get("images/ui_basic/checkbox_checked.png"), &tags_box_,
	          _("Tags"));
	tabs_.add("map_teams", g_image_cache->get("images/wui/editor/tools/players.png"), &teams_box_,
	          _("Teams"));

	set_center_panel(&tab_box_);

	name_.changed.connect([this]() { changed(); });
	author_.changed.connect([this]() { changed(); });
	descr_->changed.connect([this]() { changed(); });
	hint_->changed.connect([this]() { changed(); });
	waterway_length_box_->changed.connect([this]() { changed(); });
	for (const auto& tag : tags_checkboxes_) {
		tag.second->changed.connect([this]() { changed(); });
	}

	balancing_dropdown_.selected.connect([this] { changed(); });
	theme_dropdown_.selected.connect([this] { changed(); });

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	cancel_.sigclicked.connect([this]() { clicked_cancel(); });

	update();
	ok_.set_enabled(true);

	name_.focus();
	center_to_parent();
	move_to_top();

	initialization_complete();
}

void MainMenuMapOptions::update_waterway_length_warning() {
	const uint32_t len = waterway_length_box_->get_value();
	if (len > kMaxRecommendedWaterwayLengthLimit) {
		waterway_length_warning_->set_icon(g_image_cache->get("images/ui_basic/stop.png"));
		waterway_length_warning_->set_tooltip(
		   format(_("It is not recommended to permit waterway lengths greater than %u"),
		          kMaxRecommendedWaterwayLengthLimit));
	} else {
		waterway_length_warning_->set_icon(nullptr);
		waterway_length_warning_->set_tooltip("");
	}
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
 */
void MainMenuMapOptions::update() {
	const Widelands::Map& map = eia().egbase().map();
	author_.set_text(map.get_author());
	name_.set_text(map.get_name());
	size_.set_text(format(_("Size: %1% x %2%"), map.get_width(), map.get_height()));
	descr_->set_text(map.get_description());
	hint_->set_text(map.get_hint());
	waterway_length_box_->set_value(map.get_waterway_max_length());
	update_waterway_length_warning();

	const std::set<std::string>& tags = map.get_tags();
	for (auto tag : tags_checkboxes_) {
		tag.second->set_state(tags.count(tag.first) > 0);
	}

	balancing_dropdown_.select(tags.count("balanced") != 0u ? "balanced" : "unbalanced");

	theme_dropdown_.select(map.get_background_theme());
	if (!theme_dropdown_.has_selection()) {
		theme_dropdown_.select("");
	}
}

/**
 * Called when one of the editboxes are changed
 */
void MainMenuMapOptions::changed() {
	ok_.set_enabled(true);
}

void MainMenuMapOptions::clicked_ok() {
	Widelands::Map& map = *eia().egbase().mutable_map();
	map.set_name(name_.get_text());
	map.set_author(author_.get_text());
	set_config_string("realname", author_.get_text());
	map.set_description(descr_->get_text());
	map.set_hint(hint_->get_text());
	map.set_waterway_max_length(waterway_length_box_->get_value());

	map.get_suggested_teams().clear();
	for (SuggestedTeamsEntry* ste : suggested_teams_entries_) {
		if (!ste->team().empty()) {
			map.get_suggested_teams().push_back(ste->team());
		}
	}

	map.clear_tags();
	for (const auto& tag : tags_checkboxes_) {
		if (tag.second->get_state()) {
			map.add_tag(tag.first);
		}
	}
	map.add_tag(balancing_dropdown_.get_selected());
	map.set_background_theme(theme_dropdown_.get_selected());
	Notifications::publish(NoteMapOptions());
	registry_.destroy();
}

void MainMenuMapOptions::clicked_cancel() {
	registry_.destroy();
}

bool MainMenuMapOptions::handle_key(bool down, SDL_Keysym code) {
	if (down && code.sym == SDLK_RETURN) {
		clicked_ok();
		return true;
	}
	return UI::UniqueWindow::handle_key(down, code);
}

/*
 * Add a tag to the checkboxes
 */
void MainMenuMapOptions::add_tag_checkbox(UI::Box* parent, const std::string& tag) {
	UI::Box* box = new UI::Box(
	   parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, max_w_, checkbox_space_, 0);
	TagTexts l = localize_tag(tag);
	UI::Checkbox* cb = new UI::Checkbox(box, UI::PanelStyle::kWui, Vector2i::zero(), l.displayname);
	cb->set_tooltip(l.tooltip);
	box->add(cb, UI::Box::Resizing::kFullSize);
	box->add_space(checkbox_space_);
	parent->add(box);
	parent->add_space(padding_);
	tags_checkboxes_[tag] = cb;
}

void MainMenuMapOptions::delete_suggested_team(SuggestedTeamsEntry* ste) {
	inner_teams_box_.set_force_scrolling(false);

	auto is_deleted_panel = [ste](SuggestedTeamsEntry* i) { return ste == i; };
	suggested_teams_entries_.erase(std::remove_if(suggested_teams_entries_.begin(),
	                                              suggested_teams_entries_.end(), is_deleted_panel),
	                               suggested_teams_entries_.end());
	ste->die();
	inner_teams_box_.set_force_scrolling(true);
}
