#ifndef WL_UI_FSMENU_MAPDETAILSBOX_H
#define WL_UI_FSMENU_MAPDETAILSBOX_H
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

struct GameSettingsProvider;
class MapDetailsBox : public UI::Box {
public:
	MapDetailsBox(Panel* parent,
	              uint32_t standard_element_width,
	              uint32_t standard_element_height,
	              int32_t max_x = 0,
	              int32_t max_y = 0);
	~MapDetailsBox();

	void update(GameSettingsProvider* settings);

	/// passed callback is called when the select map button is clicked
	void set_select_map_action(std::function<void()> action);

	void set_font_scale(float scale, uint32_t standard_element_height);

private:
	UI::Textarea title_;
	UI::Box title_box_;
	UI::Textarea map_name_;
	UI::Button select_map_;
	UI::MultilineTextarea map_description_;

	void load_map_info(GameSettingsProvider* settings);
};

#endif  // WL_UI_FSMENU_MAPDETAILSBOX_H
