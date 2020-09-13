#ifndef WL_UI_FSMENU_MAPDETAILSBOX_H
#define WL_UI_FSMENU_MAPDETAILSBOX_H

#include "logic/game_settings.h"
#include "logic/map.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "wui/suggested_teams_box.h"

class MapDetailsBox : public UI::Box {
public:
	MapDetailsBox(Panel* parent,
	              uint32_t standard_element_width,
	              uint32_t standard_element_height,
	              uint32_t padding,
	              int32_t max_x = 0,
	              int32_t max_y = 0);
	~MapDetailsBox();

	void update(GameSettingsProvider* settings, Widelands::Map& map);
	void update_from_savegame(GameSettingsProvider* settings);

	/// passed callback is called when the select map button is clicked
	void set_select_map_action(const std::function<void()>& action);

	void force_new_dimensions(float scale, uint32_t standard_element_height, uint32_t i);

	// TODO(jmoerschbach): only used by multiplayer screen...
	void set_map_description_text(const std::string& text);

private:
	UI::Textarea title_;
	UI::Box title_box_;
	UI::Box content_box_;
	UI::Textarea map_name_;
	UI::Button select_map_;
	UI::MultilineTextarea map_description_;
	UI::SuggestedTeamsBox suggested_teams_box_;

	void show_map_description(Widelands::Map& map, GameSettingsProvider* settings);
	void show_map_description_savegame(const GameSettings& game_settings);
	void show_map_name(const GameSettings& game_settings);
};

#endif  // WL_UI_FSMENU_MAPDETAILSBOX_H
