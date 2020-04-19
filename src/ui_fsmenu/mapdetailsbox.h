#ifndef WL_UI_FSMENU_MAPDETAILSBOX_H
#define WL_UI_FSMENU_MAPDETAILSBOX_H
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
class MapDetailsBox : public UI::Box {
public:
	MapDetailsBox(Panel* parent,
	              int32_t x,
	              int32_t y,
	              int32_t padding,
	              int32_t indent,
	              int32_t max_x = 0,
	              int32_t max_y = 0);
	~MapDetailsBox();

private:
	UI::Box map_title_box_;

	UI::Textarea map_name_;
	UI::Button select_map_;
	UI::MultilineTextarea map_description_;
};

#endif  // WL_UI_FSMENU_MAPDETAILSBOX_H
